///////////////////////////////////////////////////////////////////////
//
//  ofxGVF class
//
//  The library (ofxGVF.cpp, ofxGVF.h) has been created in 2010-2011 at Ircam Centre Pompidou by
//  - Baptiste Caramiaux
//  previously with Ircam Centre Pompidou and University Paris VI, since 2012 with Goldsmiths College, University of London
//  - Nicola Montecchio
//  previously with University of Padova, since 2012 with The Echo Nest
//
//  The library is maintained by Baptiste Caramiaux at Goldsmiths College, University of London
//
//  Copyright (C) 2013 Baptiste Caramiaux, Nicola Montecchio - STMS lab Ircam-CRNS-UPMC, University of Padova
//
//  The library is under the GNU Lesser General Public License (LGPL v3)
//
//
///////////////////////////////////////////////////////////////////////

#include "ofxGVF.h"

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tr1/memory>
#include <unistd.h>


using namespace std;



vector<vector<float> > return_RotationMatrix_3d(float phi, float theta, float psi);



////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
//              CONSTRUCTORS
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


//--------------------------------------------------------------
ofxGVF::ofxGVF(){
    setup();
}

//--------------------------------------------------------------
ofxGVF::ofxGVF(ofxGVFConfig _config){
    setup(_config);
}

//--------------------------------------------------------------
ofxGVF::ofxGVF(ofxGVFConfig _config, ofxGVFParameters _parameters){
    setup(_config, _parameters);
}




////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
//              SET-UPS
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

//--------------------------------------------------------------
void ofxGVF::setup(){
    
    // use defualt parameters
    ofxGVFConfig defaultConfig;
    
    defaultConfig.inputDimensions   = 2;
    defaultConfig.translate         = true;
    defaultConfig.segmentation      = false;
    defaultConfig.normalization     = false;
    
    setup(defaultConfig);
}

//--------------------------------------------------------------
void ofxGVF::setup(ofxGVFConfig _config){
 
    clear(); // just in case
    
    // Set configuration:
    config      = _config;
    
    // default parameters
    ofxGVFParameters defaultParameters;
    defaultParameters.numberParticles = 2000;
    defaultParameters.tolerance = 0.1f;
    defaultParameters.resamplingThreshold = 500;
    defaultParameters.distribution = 0.0f;
    defaultParameters.phaseVariance = 0.000001;
    defaultParameters.speedVariance = 0.001;
    defaultParameters.scaleVariance = vector<float>(1, 0.00001); // TODO: Check that default works like this.
    defaultParameters.rotationVariance = vector<float>(1, 0.00000);
    
    // MARK: DEPRECATED parametersSetAsDefault
    // parametersSetAsDefault = true;
    
    setup(_config,  defaultParameters);

}

//--------------------------------------------------------------
void ofxGVF::setup(ofxGVFConfig _config, ofxGVFParameters _parameters){
    
    
    clear(); // just in case
    
    // Set configuration and parameters
    config      = _config;
    parameters  = _parameters;
    
    // setStateDimensions(config.inputDimensions);
    
//    // Adjust to dimensions necessary (added by AVZ)
//    parameters.scaleVariance = vector<float>(scale_dim, parameters.scaleVariance[0]);
//    parameters.rotationVariance = vector<float>(rotation_dim, parameters.rotationVariance[0]);
    

#if !BOOSTLIB
    normdist = new std::tr1::normal_distribution<float>();
    unifdist = new std::tr1::uniform_real<float>();
    rndnorm  = new std::tr1::variate_generator<std::tr1::mt19937, std::tr1::normal_distribution<float> >(rng, *normdist);
#endif
    
    // absolute weights
    abs_weights = vector<float>();
    
    // MARK: DEPRECATED parametersSetAsDefault
    //parametersSetAsDefault = false;
    
    has_learned = false;
}



////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
//              DESTRUCTOR
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////



//--------------------------------------------------------------
ofxGVF::~ofxGVF(){

#if !BOOSTLIB
    if(normdist != NULL)
        delete (normdist);
    if(unifdist != NULL)
        delete (unifdist);
#endif
    
    if (rndnorm != NULL)
        delete (rndnorm);
    
    clear(); // not really necessary but it's polite ;)
    
}


//--------------------------------------------------------------
// Clear the internal data (templates)
void ofxGVF::clear(){
    
    state = STATE_CLEAR;
    
    // clear templates
    gestureTemplates.clear();
    
    mostProbableIndex = -1;
    mostProbableStatus.clear();

}




////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
//              ADD & FILL TEMPLATES FOR GESTURES
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


//--------------------------------------------------------------
void ofxGVF::addGestureTemplate(ofxGVFGesture & gestureTemplate){
    
    int inputDimension = gestureTemplate.getNumberDimensions();
    config.inputDimensions = inputDimension;
    
    if(minRange.size() == 0){
        minRange.resize(inputDimension);
        maxRange.resize(inputDimension);
    }
    
    for(int j = 0; j < inputDimension; j++){
        minRange[j] = INFINITY;
        maxRange[j] = -INFINITY;
    }
    
    // compute min/max from the data
    for(int i = 0; i < gestureTemplates.size(); i++){
        ofxGVFGesture& tGestureTemplate = gestureTemplates[i];
        vector<float>& tMinRange = tGestureTemplate.getMinRange();
        vector<float>& tMaxRange = tGestureTemplate.getMaxRange();
        for(int j = 0; j < inputDimension; j++){
            if(tMinRange[j] < minRange[j]) minRange[j] = tMinRange[j];
            if(tMaxRange[j] > maxRange[j]) maxRange[j] = tMaxRange[j];
        }
    }
    
    for(int i = 0; i < gestureTemplates.size(); i++){
        ofxGVFGesture& tGestureTemplate = gestureTemplates[i];
        tGestureTemplate.setMinRange(minRange);
        tGestureTemplate.setMaxRange(maxRange);
    }
    
    
    gestureTemplates.push_back(gestureTemplate);
    abs_weights.resize(gestureTemplates.size());
    
    
    // (re-)learn for each new template added
    //learn();
    
}




////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
//              LEARN
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


//----------------------------------------------
//
// Learn Internal Configuration
//
void ofxGVF::learn(){
    
    if (gestureTemplates.size() > 0){
        
        // get the number of dimension in templates
        config.inputDimensions = gestureTemplates[0].getTemplateDimension();
        
        // Set Scale and Rotation dimensions, according to input dimensions and init variances
        initStateSpace();
        initParticleFilter();
        

        // Offset for segmentation
        offS=vector<vector<float> >(parameters.numberParticles);
        for (int k = 0; k < parameters.numberParticles; k++)
        {
            offS[k] = vector<float>(config.inputDimensions);
            for (int j = 0; j < config.inputDimensions; j++)
                offS[k][j] = 0.0;
        }
        
        // NORMALIZATION
        if (config.normalization) {     // update the global normaliation factor
            globalNormalizationFactor = -1.0;
            // loop on previous gestures already learned
            // take the max of all the gesture learned ...
            for (int k=0; k<getNumberOfGestureTemplates() ; k++){
                for(int j = 0; j < config.inputDimensions; j++){
                    float rangetmp = fabs(getGestureTemplate(k).getMaxRange()[j]-getGestureTemplate(k).getMinRange()[j]);
                    if (rangetmp > globalNormalizationFactor)
                        globalNormalizationFactor=rangetmp;
                }
            }
        }
        
        has_learned = true; // ???: Should there be a verification that learning was successful?
    }
}


//--------------------------------------------------------------
void ofxGVF::initStateSpace() {
    
    int input_dim = config.inputDimensions;
    // input dimension here defines the number of dimensions of the state space
    if (input_dim == 2){
        // scale uniformly on the 2 dimensions
        // rotate by a 2-d rotation matrix depending on 1 angle
        scale_dim = 1;
        rotation_dim = 1;
    }
    else if (input_dim == 3){
        // scale non-uniformaly on the 3 dimensions (3 scaling coef)
        // 3-d rotation matrix with 3 angles
        scale_dim = 3;
        rotation_dim = 3;
    }
    else {
        scale_dim = 1;
        rotation_dim = 1;
    }
    
    // pdim = state space dimension
    pdim = 2 + scale_dim + rotation_dim;
    
    
    // init the dynamics of the states
    // dynamics are goven by the gaussian variances as:
    //      x_t = A x_{t-1} + v_t
    // where v_t is the gaussian noise, x_t the state space at t
    parameters.scaleVariance.resize(scale_dim);
    parameters.rotationVariance.resize(rotation_dim);

    featVariances.clear();
    featVariances = vector<float> (pdim);
    
    featVariances[0] = sqrt(parameters.phaseVariance);
    featVariances[1] = sqrt(parameters.speedVariance);
    for (int k = 0; k < scale_dim; k++)
        featVariances[2+k] = sqrt(parameters.scaleVariance[k]);
    for (int k = 0; k < rotation_dim; k++)
        featVariances[2+scale_dim+k] = sqrt(parameters.rotationVariance[k]);
    
    // Spreading parameters: initial value for each variation (e.g. speed start at 1.0 [i.e. original speed])
    parameters.phaseInitialSpreading = 0.0;
    parameters.speedInitialSpreading = 1.0;
    parameters.scaleInitialSpreading = vector<float> (scale_dim);
    parameters.rotationInitialSpreading = vector<float> (rotation_dim);
    
    // Spreading parameters: initial value for each variation (e.g. speed start at 1.0 [i.e. original speed])
    for (int k = 0; k < scale_dim; k++)
        parameters.scaleInitialSpreading[k]=1.0f;
    for (int k = 0;k < rotation_dim; k++)
        parameters.rotationInitialSpreading[k]=0.0f;
}


//--------------------------------------------------------------
void ofxGVF::initParticleFilter() {
    
    // Init Particle Filter
    initMat(X, parameters.numberParticles, pdim);           // Matrix of NS particles
    initVec(g, parameters.numberParticles);                 // Vector of gesture class
    initVec(w, parameters.numberParticles);                 // Weights
    
    
    // ADAPTATION OF THE TOLERANCE IF DEFAULT PARAMTERS
    // ---------------------------
    //        if (parametersSetAsDefault) {
    
    float obsMeanRange = 0.0f;
    for (int gt=0; gt<gestureTemplates.size(); gt++) {
        for (int d=0; d<config.inputDimensions; d++)
            obsMeanRange += (gestureTemplates[gt].getMaxRange()[d] - gestureTemplates[gt].getMinRange()[d])
            /config.inputDimensions;
    }
    obsMeanRange /= gestureTemplates.size();
    parameters.tolerance = obsMeanRange / 8.0f;  // dividing by an heuristic factor [to be learned?]
    
    //}
    // ---------------------------
}



//--------------------------------------------------------------
ofxGVFGesture & ofxGVF::getGestureTemplate(int index){
    assert(index < gestureTemplates.size());
    return gestureTemplates[index];
}

//--------------------------------------------------------------
vector<ofxGVFGesture> & ofxGVF::getAllGestureTemplates(){
    return gestureTemplates;
}

//--------------------------------------------------------------
int ofxGVF::getNumberOfGestureTemplates(){
    return gestureTemplates.size();
}

//--------------------------------------------------------------
void ofxGVF::removeGestureTemplate(int index){
    assert(index < gestureTemplates.size());
    gestureTemplates.erase(gestureTemplates.begin() + index);
}

//--------------------------------------------------------------
void ofxGVF::removeAllGestureTemplates(){
    gestureTemplates.clear();
}



////////////////////////////////////////////////////////////////
//
// STATE SET & GET - eg., clear, learn, follow
//
////////////////////////////////////////////////////////////////

//--------------------------------------------------------------
void ofxGVF::setState(ofxGVFState _state){
    switch (_state) {
        case STATE_CLEAR:
            clear();
            break;
        case STATE_LEARNING:
            state = _state;
            break;
        case STATE_FOLLOWING:
            state = _state;
            // if following and some templates have laready been recorded start learning
            if (gestureTemplates.size() > 0)
            {
                learn();
                spreadParticles();
            }
            break;
    }
}

//--------------------------------------------------------------
ofxGVF::ofxGVFState ofxGVF::getState(){
    return state;
}

//--------------------------------------------------------------
string ofxGVF::getStateAsString(){
    switch (state) {
        case STATE_CLEAR:
            return "STATE_CLEAR";
            break;
        case STATE_LEARNING:
            return "STATE_LEARNING";
            break;
        case STATE_FOLLOWING:
            return "STATE_FOLLOWING";
            break;
    }
}

////////////////////////////////////////////////////////////////
//
// CORE FUNCTIONS & MATH
//
////////////////////////////////////////////////////////////////

//--------------------------------------------------------------
void ofxGVF::spreadParticles(){
    
    if (has_learned)
    {
        spreadParticles(parameters);
    }
    // ???: else
}

//--------------------------------------------------------------
// Spread the particles by sampling values from intervals given my their means and ranges.
// Note that the current implemented distribution for sampling the particles is the uniform distribution
//void GVF::spreadParticles(vector<float> & means, vector<float> & ranges){
void ofxGVF::spreadParticles(ofxGVFParameters _parameters){
    
    
    // USE BOOST FOR UNIFORM DISTRIBUTION!!!!
    // deprecated class should use uniform_real_distribution
#if BOOSTLIB
	boost::uniform_real<float> ur(0,1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
	
	unsigned int ngestures = gestureTemplates.size();// numTemplates+1;
    
    float spreadRangePhase = 0.1;
    float spreadRange = 0.1;
    int scalingCoefficients  = _parameters.scaleInitialSpreading.size();
    int numberRotationAngles = _parameters.rotationInitialSpreading.size();
    
    
    // Spread particles using a uniform distribution
	//for(int i = 0; i < pdim; i++)
    for(int n = 0; n < parameters.numberParticles; n++){
        X[n][0] = (rnduni() - 0.5) * spreadRangePhase + _parameters.phaseInitialSpreading;
        X[n][1] = (rnduni() - 0.5) * spreadRange + _parameters.speedInitialSpreading;
        for (int nn=0; nn<scalingCoefficients; nn++)
            X[n][2+nn] = (rnduni() - 0.5) * spreadRange
            + _parameters.scaleInitialSpreading[nn];
        for (int nn=0; nn<numberRotationAngles; nn++)
            X[n][2+scalingCoefficients+nn] = (rnduni() - 0.5) * _parameters.rotationInitialSpreading[nn] //spreadRange/2
            + 0.0;
    }
    
    
    // Weights are also uniformly spread
    initweights();
    
    // Spread uniformly the gesture class among the particles
	for(int n = 0; n < parameters.numberParticles; n++){
		g[n] = n % ngestures;
        
        // offsets are set to 0
        for (int k=0; k < config.inputDimensions; k++)
            offS[n][k] = 0.0;
    }
    
}


//--------------------------------------------------------------
// Spread the particles by sampling values from intervals given my their means and ranges.
// Note that the current implemented distribution for sampling the particles is the uniform distribution
void ofxGVF::spreadParticles(vector<float> & means, vector<float> & ranges){
    
    
    // USE BOOST FOR UNIFORM DISTRIBUTION!!!!
    // deprecated class should use uniform_real_distribution
#if BOOSTLIB
	boost::uniform_real<float> ur(0,1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
	
    //	unsigned int ngestures = numTemplates+1;
	
    // Spread particles using a uniform distribution
	for(int i = 0; i < pdim; i++)
		for(int n = 0; n < parameters.numberParticles; n++)
			X[n][i] = (rnduni() - 0.5) * ranges[i] + means[i];
    
    // Weights are also uniformly spread
    initweights();
    initweights2();

	
    // Spread uniformly the gesture class among the particles
	for(int n = 0; n < parameters.numberParticles; n++)
    {
		g[n] = n % getNumberOfGestureTemplates();
    
        // offsets are set to 0
        for (int k=0; k < config.inputDimensions; k++)
            offS[n][k]=0.0;
    }
    
    
}


//--------------------------------------------------------------
// Initialialize the weights of the particles. The initial values of the weights is a
// unifrom weight over the particles
void ofxGVF::initweights(){
    for (int k = 0; k < parameters.numberParticles; k++){
        w[k] = 1.0 / (float) parameters.numberParticles;
    }
    
}


//--------------------------------------------------------------
// Initialialize the weights of the particles. The initial values of the weights is a
// unifrom weight over the particles
void ofxGVF::initweights2(){
    for (int k = 0; k < parameters.numberParticles; k++){
        for (int l = 0; l < X[0].size(); l++){
            w2[k][l] = 1.0 / (float) parameters.numberParticles;
        }
    }
}



//--------------------------------------------------------------
float distance_weightedEuclidean(vector<float> x, vector<float> y, vector<float> w){
    int count = x.size();
    // the size must be > 0
    if (count <= 0)
        return 0;
    
    float dist = 0.0;
    for(int k = 0; k < count; k++){
        dist += w[k] * pow((x[k] - y[k]), 2);
    }
    return dist;
}

//--------------------------------------------------------------
// Performs the inference based on a given new observation. This is the core algorithm: does
// one step of inference using particle filtering. It is the optimized version of the
// function ParticleFilter(). Note that the inference is possible only if some templates
// have been learned beforehand
//
// The inferring values are the weights of each particle that represents a possible gesture,
// plus a possible configuration of the features (value of speec, scale,...)
void ofxGVF::particleFilter(vector<float> & obs){
    
    
    // random generators
#if BOOSTLIB
	boost::uniform_real<float> ur(0,1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
    
    
    
    // zero abs weights
    for(int i = 0 ; i < getNumberOfGestureTemplates(); i++){
        abs_weights[i] = 0.0;
    }
    
    // clear any previous information about the particles' positions
    // (this is used for possible visualization but not in the inference)
    particlesPositions.clear();
    
    float sumw = 0.0;
    vector<float> sumw2(pdim,0.0);
    

    
    // MAIN LOOP: same process for EACH particle (row n in X)
    for(int n = 0; n<parameters.numberParticles; n++)
    {
        
        
        // Move the particle
        // Position respects a first order dynamic: p = p + v/L
        X[n][0] += (*rndnorm)() * featVariances[0] + X[n][1]/gestureTemplates[g[n]].getTemplateLength(); //gestureLengths[g[n]];
        
        
		// Move the other state elements according a gaussian noise
        // featVariances vector of variances
        for(int l= 1; l < X[n].size(); l++)
			X[n][l] += (*rndnorm)() * featVariances[l];
        
		vector<float> x_n = X[n];
        
        
        //        if (!config.segmentation){ ???
        if(x_n[0] < 0.0 || x_n[0] > 1.0) {
            w[n] = 0.0;

        }
        else {       // ...otherwise we propagate the particle's values and update its weight
            
            
            int pgi = g[n];
            
            // given the phase between 0 and 1 (first value of the particle x),
            // return the index of the corresponding gesture, given by g(n)
            int frameindex = min((int)(gestureTemplates[pgi].getTemplateLength() - 1),(int)(floor(x_n[0] * gestureTemplates[pgi].getTemplateLength() ) ) ); //min((int)(gestureLengths[pgi]-1),(int)(floor(x_n[0] * gestureLengths[pgi])));
            
            // given the index, return the gesture template value at this index
            vector<float> vref(config.inputDimensions);
            setVec(vref, gestureTemplates[pgi].getTemplate()[frameindex]);

            // if normalization
            if (config.normalization)
                for (int kk=0; kk<vref.size(); kk++)
                    vref[kk] = vref[kk] / globalNormalizationFactor;
            
            //setVec(vref, R_single[pgi][frameindex]);
            
            
            vector<float> vobs(config.inputDimensions);
            if (config.translate)
                for (int j=0; j < config.inputDimensions; j++)
                    vobs[j]=obs[j]-offS[n][j];
            else
                setVec(vobs, obs);
            
            // if normalization
            if (config.normalization)
                for (int kk=0; kk<vobs.size(); kk++)
                    vobs[kk] = vobs[kk] / globalNormalizationFactor;
            
            
            // If incoming data is 2-dimensional: we estimate phase, speed, scale, angle
            if (config.inputDimensions == 2){
                
                // scaling
                for (int k=0;k < config.inputDimensions;k++)
                    vref[k] *= x_n[2];
                
                // rotation
                float alpha = x_n[3];
                float tmp0=vref[0]; float tmp1=vref[1];
                vref[0] = cos(alpha)*tmp0 - sin(alpha)*tmp1;
                vref[1] = sin(alpha)*tmp0 + cos(alpha)*tmp1;
                
                // put the positions into vector
                // [used for visualization]
                std::vector<float> temp;
                temp.push_back(vref[0]);
                temp.push_back(vref[1]);
                particlesPositions.push_back(temp);
                
            }
            // If incoming data is 3-dimensional
            else if (config.inputDimensions == 3){
                
                // Scale template sample according to the estimated scaling coefficients
                int numberScaleCoefficients = parameters.scaleInitialSpreading.size();
                for (int k = 0; k < numberScaleCoefficients; k++)
                    vref[k] *= x_n[2+k];
                
                // Rotate template sample according to the estimated angles of rotations (3d)
                vector<vector< float> > RotMatrix = return_RotationMatrix_3d(x_n[2+numberScaleCoefficients],
                                                                             x_n[2+numberScaleCoefficients+1],
                                                                             x_n[2+numberScaleCoefficients+2]);
                vref = multiplyMat(RotMatrix, vref);
                
                // put the positions into vector
                // [used for visualization]
                std::vector<float> temp;
                for (int ndi=0; ndi < config.inputDimensions; ndi++)
                    temp.push_back(vref[ndi]);
                particlesPositions.push_back(temp);
                
            }
            else {
                
                // sca1ing
                for (int k=0;k < config.inputDimensions; k++)
                    vref[k] *= x_n[2];
                
                // put the positions into vector
                // [used for visualization]
                std::vector<float> temp;
                for (int ndi=0; ndi < config.inputDimensions; ndi++)
                    temp.push_back(vref[ndi]);
                particlesPositions.push_back(temp);
                
            }
            
            
            
            // compute distance between estimation given the current particle values
            // and the incoming observation
            
            // define weights here on the dimension if needed
            vector<float> dimWeights(config.inputDimensions);
            for(int k = 0; k < config.inputDimensions; k++) dimWeights[k] = 1.0 / config.inputDimensions;
            
            // observation likelihood and update weights
            float dist = distance_weightedEuclidean(vref,vobs,dimWeights) * 1 / (parameters.tolerance * parameters.tolerance);
            
            
            //cout << "n="<< n << " " << dist << " " << distance_weightedEuclidean(vref,obs,dimWeights) << " " << parameters.tolerance << endl;
            
            if(parameters.distribution == 0.0f)    // Gaussian distribution
            {
                //                cout << "Dist is " << dist << endl;
                //                cout << "exp(-dist) " << exp(-dist) << endl;
                w[n]   *= exp(-dist);
                
//                for (int kk=0; kk<pdim; kk++)
//                    w2[n][kk] *= exp(-dist);
                
                abs_weights[g[n]] += exp(-dist);

                //    cout << n << "- " << g[n] << " -- " << vref[0] << "," << vobs[0] << " | " << vref[1]
                //        << "," << vobs[1] << " | " << offS[n][0] << " " << offS[n][1] << " " << dist << "," << w[n] << " " << X[n][0] << endl;
            }
            else            // Student's distribution
            {
                w[n]   *= pow(dist/nu + 1, -nu/2 - 1);    // dimension is 2 .. pay attention if editing]
            }
            
            
            
        }
        
        sumw += w[n];
//        for (int kk=0; kk<pdim; kk++)
//            sumw2[kk] += w2[n][kk];
        
    }
    //    }
    
    // normalize weights and compute criterion for degeneracy
    //	w /= w.sum();
    //	float neff = 1./w.dot(w);
    float dotProdw = 0.0;
    vector<float> dotProdw2(pdim,0.0);
    for (int k = 0; k < parameters.numberParticles; k++){
        w[k] /= sumw;
        dotProdw+=w[k]*w[k];
        
//        for (int kk=0; kk<pdim; kk++){
//            w2[k][kk] /= sumw2[kk];
//            dotProdw2[kk]+=w2[k][kk]*w2[k][kk];
//        }
    }
    float neff = 1./dotProdw;
    
    
    
    // avoid degeneracy (no particles active, i.e. weights = 0) by resampling
    // around the active particles
    //cout << "resamplingThreshold: " << resamplingThreshold << " " << neff << endl;
	if(neff < parameters.resamplingThreshold)
    {
        //    post("resampling");
        //cout << "Resampling" << endl;
        resampleAccordingToWeights(obs);
        initweights();
    }
    
    
}

//--------------------------------------------------------------
// Resampling function. The function resamples the particles based on the weights.
// Particles with negligeable weights will be respread near the particles with non-
// neglieable weigths (which means the most likely estimation).
// This steps is important to avoid degeneracy problem
void ofxGVF::resampleAccordingToWeights(vector<float> obs)
{
    
    int numOfPart = parameters.numberParticles;
    
#if BOOSTLIB
    boost::uniform_real<float> ur(0,1);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
    
    vector< vector<float> > oldX;
    setMat(oldX, X);
    vector<int> oldG;
    setVec(oldG, g);
    vector<float> c(numOfPart);
    
    c[0] = 0;
    for(int i = 1; i < numOfPart; i++)
        c[i] = c[i-1] + w[i];
    int i = 0;
    float u0 = rnduni()/numOfPart;
    
    
    // defining here the number of particles allocated to reinitialisation
    // used for segmentation
    int free_pool = 0;
    if (config.segmentation)
        free_pool = round(3*numOfPart/100);
    
    for (int j = 0; j < numOfPart; j++)
    {
        float uj = u0 + (j + 0.) / numOfPart;
        
        while (uj > c[i] && i < numOfPart - 1){
            i++;
        }
        
        for (int kk=0;kk<X[0].size();kk++)
            X[j][kk] = oldX[i][kk];
        g[j] = oldG[i];
        
        w[j] = 1.0/(float)numOfPart;
    }
    
    for (int j = 0; j < free_pool; j++){
        
        int index = round(rnduni()*numOfPart);
        
        if (index == numOfPart)
            index = 0;
        
        w[index] = 1.0/(float)(numOfPart * numOfPart);
        
        
        float spreadRange = 0.02;
        int scalingCoefficients  = parameters.scaleInitialSpreading.size();
        int numberRotationAngles = parameters.rotationInitialSpreading.size();
        // Spread particles using a uniform distribution
        X[index][0] = (rnduni() - 0.5) * spreadRange + parameters.phaseInitialSpreading;
        X[index][1] = (rnduni() - 0.5) * spreadRange + parameters.speedInitialSpreading;
        for (int nn=0; nn<scalingCoefficients; nn++)
            X[index][2+nn] = (rnduni() - 0.5) * spreadRange + parameters.scaleInitialSpreading[nn];
        for (int nn=0; nn<numberRotationAngles; nn++)
            X[index][2+scalingCoefficients+nn] = (rnduni() - 0.5) * 0.0 + parameters.rotationInitialSpreading[nn];
        
        if (config.translate)
        {
            for (int jj=0; jj<config.inputDimensions; jj++)
                offS[index][jj]=obs[jj];
        }
        
        g[index] = index % getNumberOfGestureTemplates(); // distribute particles across templates
    }
    //initweights();
    
    if (config.segmentation)
    {
        float sumw = 0.0;
        for (int j = 0; j < numOfPart; j++)
            sumw += w[j];
        for (int j = 0; j < numOfPart; j++)
            w[j] /= sumw;
    }
    
    
}

//--------------------------------------------------------------
// Step function is the function called outside for inference. It
// has been originally created to be able to infer on a new observation or
// a set of observation.
void ofxGVF::infer(vector<float> vect){

    particleFilter(vect);
    updateEstimatedStatus();
    
}


void ofxGVF::updateEstimatedStatus(){
    
    
    int numOfPart = parameters.numberParticles;
    
    
    setMat(status, 0.0f, getNumberOfGestureTemplates(), pdim + 1);   // rows are gestures, cols are features + probabilities
	
    
	// compute the estimated features by computing the expected values
    // sum ( feature values * weights)
	for(int n = 0; n < numOfPart; n++){
        int gi = g[n];
        for(int m = 0; m < pdim; m++)
            status[gi][m] += X[n][m] * w[n];
        
        // compute probability
		status[gi][pdim] += w[n];
    }
	
    // calculate most probable index during scaling...
    float maxProbability = 0.0f;
    mostProbableIndex = -1;
    
	for(int gi = 0; gi < getNumberOfGestureTemplates(); gi++){
        for(int m = 0; m < pdim; m++){
            status[gi][m] /= status[gi][pdim];
        }
        if(status[gi][pdim] > maxProbability){
            maxProbability = status[gi][pdim];
            mostProbableIndex = gi;
        }
		//es.block(gi,0,1,pdim) /= es(gi,pdim);
	}
    
    if(mostProbableIndex > -1)
        mostProbableStatus = status[mostProbableIndex];
    
    UpdateOutcomes();
}

////////////////////////////////////////////////////////////////
//
// PROBABILITY AND TEMPLATE ACCESS
//
////////////////////////////////////////////////////////////////

// GESTURE PROBABILITIES + POSITIONS

//--------------------------------------------------------------
// Returns the index of the currently recognized gesture
// NOW CACHED DURING 'infer' see updateEstimatedStatus()
int ofxGVF::getMostProbableGestureIndex(){
    return mostProbableIndex;
}

//--------------------------------------------------------------
// Returns the estimates features. It calls status to refer to the status of the state
// space which comprises the features to be adapted. If features are phase, speed, scale and angle,
// the function will return these estimateed features for each gesture, plus their probabilities.
// The returned matrix is nxm
//   rows correspond to the gestures in the vocabulary
//   cols correspond to the features (the last column is the [conditionnal] probability of each gesture)
// The output matrix is an Eigen matrix
// NOW CACHED DURING 'infer' see updateEstimatedStatus()
vector< vector<float> > ofxGVF::getEstimatedStatus(){
    return status;
}

ofxGVFOutcomes ofxGVF::getOutcomes() {
    return outcomes;
}

void ofxGVF::UpdateOutcomes() {
    
    outcomes.most_probable = mostProbableIndex;
    
    outcomes.estimations.clear(); // FIXME: edit later to only clear when necessary
    
    // Fill estimation for each gesture
    for (int i = 0; i < gestureTemplates.size(); ++i) {
        ofxGVFEstimation estimation;
        
        estimation.phase = status[i][0];
        
        estimation.speed = status[i][1];
        
        estimation.scale = vector<float> (scale_dim);
        for (int j = 0; j < scale_dim; ++j)
            estimation.scale[j] = status[i][2 + j];
        
        estimation.rotation = vector<float> (rotation_dim);
        for (int j = 0; j < rotation_dim; ++j)
            estimation.rotation[j] = status[i][2 + scale_dim + j];
        
        estimation.probability = status[i][status[0].size() - 1]; // !!!: Probability is last in list (counter-intuitive!)
        
        outcomes.estimations.push_back(estimation);
    }
    
    assert(outcomes.estimations.size() == gestureTemplates.size());
    
}

//--------------------------------------------------------------
ofxGVFEstimation ofxGVF::getTemplateRecogInfo(int templateNumber) { // FIXME: Rename!
    
    // ???: Later transfer to an assert here.
    //    assert(templateNumber >= 0 && templateNumber < getOutcomes().estimations.size());
    
    if (getOutcomes().estimations.size() <= templateNumber) {
        ofxGVFEstimation estimation;
        return estimation; // blank
    }
    else
        return getOutcomes().estimations[templateNumber];
}

//--------------------------------------------------------------
ofxGVFEstimation ofxGVF::getRecogInfoOfMostProbable() // FIXME: Rename!
{
    int indexMostProbable = getMostProbableGestureIndex();
    
    if ((getState() == ofxGVF::STATE_FOLLOWING) && (getMostProbableGestureIndex() != -1)) {
        return getTemplateRecogInfo(indexMostProbable);
    }
    else {
        ofxGVFEstimation estimation;
        return estimation; // blank
    }
}


//--------------------------------------------------------------
// Returns the probabilities of each gesture. This probability is conditionnal
// because it depends on the other gestures in the vocabulary:
// probability to be in gesture A knowing that we have gesture A, B, C, ... in the vocabulary
vector<float> ofxGVF::getGestureProbabilities()
{
    //	unsigned int ngestures = numTemplates+1;
    
	vector<float> gp(getNumberOfGestureTemplates());
    setVec(gp, 0.0f);
	for(int n = 0; n < parameters.numberParticles; n++)
		gp[g[n]] += w[n];
    
	return gp;
}

//--------------------------------------------------------------
vector< vector<float> > ofxGVF::getParticlesPositions(){
    return particlesPositions;
}

////////////////////////////////////////////////////////////////
//
// GET & SET FUNCTIONS FOR ALL INTERNAL VALUES
//
////////////////////////////////////////////////////////////////

// CONFIGURATION

//--------------------------------------------------------------
void ofxGVF::setConfig(ofxGVFConfig _config){
    config = _config;
}

//--------------------------------------------------------------
ofxGVFConfig ofxGVF::getConfig(){
    return config;
}


// PARAMETERS

//--------------------------------------------------------------
void ofxGVF::setParameters(ofxGVFParameters _parameters){
    
    if (_parameters.numberParticles != parameters.numberParticles)
    {
        parameters = _parameters;
        
        if (parameters.numberParticles < 4)     // minimum number of particles allowed
            parameters.numberParticles = 4;
        
        initMat(X, parameters.numberParticles, pdim);           // Matrix of NS particles
        initVec(g, parameters.numberParticles);                 // Vector of gesture class
        initVec(w, parameters.numberParticles);                 // Weights
        
        // Offset for segmentation
        offS=vector<vector<float> >(parameters.numberParticles);
        for (int k = 0; k < parameters.numberParticles; k++)
        {
            offS[k] = vector<float>(config.inputDimensions);
            for (int j = 0; j < config.inputDimensions; j++)
                offS[k][j] = 0.0;
        }
        
        if (parameters.numberParticles <= parameters.resamplingThreshold) {
            parameters.resamplingThreshold = parameters.numberParticles / 4;
        }
        
        spreadParticles();
    }
    else {
        parameters = _parameters;
    }

}

ofxGVFParameters ofxGVF::getParameters(){
    return parameters;
}

//--------------------------------------------------------------
// Update the number of particles
void ofxGVF::setNumberOfParticles(int numberOfParticles){
    particlesPositions.clear();
    initMat(X, numberOfParticles, pdim);          // Matrix of NS particles
    initVec(g, numberOfParticles);               // Vector of gesture class
    initVec(w, numberOfParticles);               // Weights
    //    logW = VectorXf(newNs);
    spreadParticles();
}

//--------------------------------------------------------------
int ofxGVF::getNumberOfParticles(){
    return parameters.numberParticles; // Return the number of particles
}

//--------------------------------------------------------------
// Update the resampling threshold used to avoid degeneracy problem
void ofxGVF::setResamplingThreshold(int _resamplingThreshold){
    if (_resamplingThreshold >= parameters.numberParticles) _resamplingThreshold = floor(parameters.numberParticles/2.0f); // TODO: we should provide feedback to the GUI!!! maybe a get max resampleThresh func??
    parameters.resamplingThreshold = _resamplingThreshold;
}

//--------------------------------------------------------------
// Return the resampling threshold used to avoid degeneracy problem
int ofxGVF::getResamplingThreshold(){
    return parameters.resamplingThreshold;
}

//--------------------------------------------------------------
// Update the standard deviation of the observation distribution
// this value acts as a tolerance for the algorithm
// low value: less tolerant so more precise but can diverge
// high value: more tolerant so less precise but converge more easily
void ofxGVF::setTolerance(float _tolerance){
    if (_tolerance == 0.0) _tolerance = 0.1; // TODO: we should provide feedback to the GUI!!!
    //_tolerance = 1.0f / (_tolerance * _tolerance);
	//tolerance = _tolerance > 0.0f ? _tolerance : tolerance;
    parameters.tolerance = _tolerance;
}

//--------------------------------------------------------------
float ofxGVF::getTolerance(){
    return parameters.tolerance;
}

//--------------------------------------------------------------
void ofxGVF::setDistribution(float _distribution){
    nu = _distribution;
    parameters.distribution = _distribution;
}

//--------------------------------------------------------------
float ofxGVF::getDistribution(){
    return parameters.distribution;
}

////--------------------------------------------------------------
//void ofxGVF::setGestureType(ofxGVFGestureType type){
//    parameters.gestureType = type;
//    kGestureType = parameters.gestureType;
//}
//
////--------------------------------------------------------------
//ofxGVFGestureType ofxGVF::getGestureType(){
//    return kGestureType;
//}

// VARIANCE COEFFICIENTS

//--------------------------------------------------------------
void ofxGVF::setPhaseVariance(float phaseVariance){
    parameters.phaseVariance = phaseVariance;
    featVariances[0] = sqrt(phaseVariance);
}

//--------------------------------------------------------------
float ofxGVF::getPhaseVariance(){
    return parameters.phaseVariance;
}

//--------------------------------------------------------------
void ofxGVF::setSpeedVariance(float speedVariance){
    parameters.speedVariance = speedVariance;
    featVariances[1] = sqrt(speedVariance);
}

//--------------------------------------------------------------
float ofxGVF::getSpeedVariance(){
    return parameters.speedVariance;
}

//--------------------------------------------------------------
void ofxGVF::setScaleVariance(float scaleVariance, int dim){
    
    vector<float> scale_variance = parameters.scaleVariance;
    
    scale_variance[dim] = scaleVariance;
    
    setScaleVariance(scale_variance);
}

//--------------------------------------------------------------
void ofxGVF::setScaleVariance(vector<float> scaleVariance){
    parameters.scaleVariance = scaleVariance;
//    initVariances(scale_dim, rotation_dim);
    for (int k = 0; k < scale_dim; k++)
        featVariances[2+k] = sqrt(parameters.scaleVariance[k]);
}

//--------------------------------------------------------------
vector<float> ofxGVF::getScaleVariance(){
    return parameters.scaleVariance;
}

//--------------------------------------------------------------
void ofxGVF::setRotationVariance(float rotationVariance, int dim){
    
    vector<float> rotation_variance = parameters.rotationVariance;
    
    rotation_variance[dim] = rotationVariance;
    
    setRotationVariance(rotation_variance);
}

//--------------------------------------------------------------
void ofxGVF::setRotationVariance(vector<float> rotationVariance){
    
    parameters.rotationVariance = rotationVariance;
    //initVariances(scale_dim, rotation_dim);
    for (int k = 0; k < rotation_dim; k++)
        featVariances[2+scale_dim+k] = sqrt(parameters.rotationVariance[k]);
    
}

//--------------------------------------------------------------
vector<float> ofxGVF::getRotationVariance(){
    return parameters.rotationVariance;
}

// MATHS

//--------------------------------------------------------------
// Return the standard deviation of the observation likelihood
float ofxGVF::getObservationStandardDeviation(){
    return parameters.tolerance;
}

//--------------------------------------------------------------
// Return the particle data (each row is a particle)
vector< vector<float> > ofxGVF::getX(){
    return X;
}

//--------------------------------------------------------------
// Return the gesture index for each particle
vector<int> ofxGVF::getG(){
    return g;
}

//--------------------------------------------------------------
// Return particles' weights
vector<float> ofxGVF::getW(){
    return w;
}

// MISC

//--------------------------------------------------------------
// Get a vector of Offsets corresponging to each particle's
// assigned offset
vector<vector<float> > ofxGVF::getIndividualOffset() {
    return offS;
}

//--------------------------------------------------------------
// Get the offset for a specific particle
vector<float> ofxGVF::getIndividualOffset(int particleIndex) {
    return offS[particleIndex];
}


// UTILITIES

//--------------------------------------------------------------
// Save function. This function is used by applications to save the
// vocabulary in a text file given by filename (filename is also the complete path + filename)
void ofxGVF::saveTemplates(string filename){
    
    std::string directory = filename;
    
    std::ofstream file_write(directory.c_str());
    
    for(int i=0; i < gestureTemplates.size(); i++) // Number of gesture templates
    {
        file_write << "template " << i << " " << config.inputDimensions << endl;
        vector<vector<float> > templateTmp = gestureTemplates[i].getTemplate();
        for(int j = 0; j < templateTmp.size(); j++)
        {
            for(int k = 0; k < config.inputDimensions; k++)
                file_write << templateTmp[j][k] << " ";
            file_write << endl;
        }
    }
    file_write.close();
    
    //    std::string directory = filename;
    //
    //    std::ofstream file_write(directory.c_str());
    //    for(int i=0; i<R_single.size(); i++){
    //        file_write << "template " << i << " " << inputDim << endl;
    //        for(int j=0; j<R_single[i].size(); j++)
    //        {
    //            for(int k=0; k<inputDim; k++)
    //                file_write << R_single[i][j][k] << " ";
    //            file_write << endl;
    //        }
    //    }
    //    file_write.close();
}




//--------------------------------------------------------------
// Load function. This function is used by applications to load a vocabulary
// given by filename (filename is also the complete path + filename)
void ofxGVF::loadTemplates(string filename){
    //    clear();
    //
    
    ofxGVFGesture loadedGesture;
    loadedGesture.clear();
    
    ifstream infile;
    stringstream doung;
    
    infile.open (filename.c_str(), ifstream::in);
    //
    string line;
    vector<string> list;
    int cl = -1;
    while(!infile.eof())
    {
        cl++;
        infile >> line;
        
        list.push_back(line);
    }
    
    int k = 0;
    int template_id = -1;
    int template_dim = 0;
    
    
    while (k < (list.size() - 1)){ // TODO to be changed if dim>2
        
        
        if (!strcmp(list[k].c_str(),"template"))
        {
            template_id = atoi(list[k+1].c_str());
            template_dim = atoi(list[k+2].c_str());
            k = k + 3;
            
            if (loadedGesture.getNumberOfTemplates() > 0){
                addGestureTemplate(loadedGesture);
                loadedGesture.clear();
            }
        }
        
        if (template_dim <= 0){
            //post("bug dim = -1");
        }
        else{
            
            vector<float> vect(template_dim);
            
            for (int kk = 0; kk < template_dim; kk++)
                vect[kk] = (float) atof(list[k + kk].c_str());
            
            loadedGesture.addObservation(vect);
        }
        k += template_dim;
        
    }
    
    if (loadedGesture.getTemplateLength() > 0){
        addGestureTemplate(loadedGesture);
        loadedGesture.clear();
    }
    
    infile.close();
}


float ofxGVF::getGlobalNormalizationFactor(){
    return globalNormalizationFactor;
}



//--------------------------------------------------------------
string ofxGVF::getStateAsString(ofxGVFState state){
    switch(state){
        case STATE_CLEAR:
            return "STATE_CLEAR";
            break;
        case STATE_LEARNING:
            return "STATE_LEARNING";
            break;
        case STATE_FOLLOWING:
            return "STATE_FOLLOWING";
            break;
        default:
            return "STATE_UNKNOWN";
            break;
    }
}




///////// ROTATION MATRIX

vector<vector<float> > return_RotationMatrix_3d(float phi, float theta, float psi)
{
    vector< vector<float> > M;
    initMat(M,3,3);
    
    M[0][0] = cos(theta)*cos(psi);
    M[0][1] = -cos(phi)*sin(psi)+sin(phi)*sin(theta)*cos(psi);
    M[0][2] = sin(phi)*sin(psi)+cos(phi)*sin(theta)*cos(psi);
    
    M[1][0] = cos(theta)*sin(psi);
    M[1][1] = cos(phi)*cos(psi)+sin(phi)*sin(theta)*sin(psi);
    M[1][2] = -sin(phi)*cos(psi)+cos(phi)*sin(theta)*sin(psi);
    
    M[2][0] = -sin(theta);
    M[2][1] = sin(phi)*cos(theta);
    M[2][2] = cos(phi)*cos(theta);
    
    return M;
    
}


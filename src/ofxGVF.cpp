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
//
// CONSTRUCTOR, DESTRUCTOR & SETUP
//
////////////////////////////////////////////////////////////////

// Constructor of the class ofxGVF
// This creates an object that is able to learn gesture template,
// to recognize in realtime the live gesture and to estimate the
// live gesture variations according to the template (e.g. scale, speed, ...)
//
// typical use
//   ofxGVF *myGVF;
//   myGVF = new ofxGVF(NS, Sigs, Icov, ResThresh, Nu)
// 
// ns is the number of particles
// Sigs is the variance for each varying feature that has to be estimated (speed, scale, twisting angle)
//
// Note about the current implementation: it involves geometric features: phase, speed, scale, angle of rotation
//    that are meant to be used for 2-dimensional input shapes. For general N-dimensional input, the class will
//    only consider phase, speed, scaling
//ofxGVF::ofxGVF(int ns, VectorXf sigs, float icov, int resThresh, float nu)

//--------------------------------------------------------------
ofxGVF::ofxGVF(){
    // nothing?
}

//--------------------------------------------------------------
ofxGVF::ofxGVF(ofxGVFConfig _config){
    setup(_config);
}

//--------------------------------------------------------------
//ofxGVF::ofxGVF(ofxGVFParameters _parameters, ofxGVFVarianceCoefficents _coefficents){
ofxGVF::ofxGVF(ofxGVFConfig _config, ofxGVFParameters _parameters){
    setup(_config, _parameters);
}

//--------------------------------------------------------------
void ofxGVF::setup(){
    
    // use defualt parameters
    
    ofxGVFConfig defaultConfig;
    
    defaultConfig.inputDimensions   = 2;
    defaultConfig.translate         = true;
    defaultConfig.segmentation      = true;
    
    setup(defaultConfig);
}

//--------------------------------------------------------------
void ofxGVF::setup(ofxGVFConfig _config){
    
    ofxGVFParameters defaultParameters;
    
    defaultParameters.numberParticles = 2000;
    defaultParameters.tolerance = 0.1f;
    defaultParameters.resamplingThreshold = 500;
    defaultParameters.distribution = 0.0f;
    defaultParameters.phaseVariance = 0.000001;
    defaultParameters.speedVariance = 0.001;
    defaultParameters.scaleVariance = vector<float>(1, 0.00001); // TODO: Check that default works like this.
    defaultParameters.rotationVariance = vector<float>(1, 0.00001);
    
    setup(_config, defaultParameters);
}

//--------------------------------------------------------------
//void ofxGVF::setup(ofxGVFParameters _parameters, ofxGVFVarianceCoefficents _coefficents){
void ofxGVF::setup(ofxGVFConfig _config, ofxGVFParameters _parameters){
    
    clear(); // just in case
    
    
    // Set parameters:
    //    input dimensions
    //    translate flag
    //    translate flag
    config = _config;
    
    
    // Set variances for variation tracking:
    //    num of particles
    //    tolerance
    //    resampling threshold
    //    distribution (nu value of the Student's T distribution [default=0])
    //    phase
    //    speed
    //    scale
    //    rotation
    parameters = _parameters;
    
    setStateDimensions(config.inputDimensions);
    
    // MARK: Adjust to dimensions necessary (added by AVZ)
    parameters.scaleVariance = vector<float>(scale_dim, parameters.scaleVariance[0]);
    parameters.rotationVariance = vector<float>(rotation_dim, parameters.rotationVariance[0]);
    
    
    initVariances(scale_dim, rotation_dim);
    
    has_learned = false;
    
    ns = parameters.numberParticles;
    
    /*
     
     //MATT: everything below about variance coefficients, matrix inits will be moved to GVF::learn()
     //  the function is created but empty
     
     if(parameters.phaseVariance != -1.0f) featVariances.push_back(sqrt(parameters.phaseVariance));
     if(parameters.speedVariance != -1.0f) featVariances.push_back(sqrt(parameters.speedVariance));
     if(parameters.scaleVariance != -1.0f) featVariances.push_back(sqrt(parameters.scaleVariance));
     if(parameters.rotationVariance != -1.0f) featVariances.push_back(sqrt(parameters.rotationVariance));
     
     pdim = featVariances.size();
     
     initMat(X, ns, pdim);           // Matrix of NS particles
     initVec(g, ns);                 // Vector of gesture class
     initVec(w, ns);                 // Weights
     initMat(offS, ns, inputDim);    // Offsets
     
     resamplingThreshold = parameters.resamplingThreshold;   // Set resampling threshold (usually NS/2)
     tolerance = parameters.tolerance;                       // inverse of the global tolerance (variance)
     nu = parameters.distribution;                           // Set Student's distribution parameter Nu
     
     //    kGestureType = parameters.gestureType;
     
     //    numTemplates=-1;                        // Set num. of learned gesture to -1
     //    gestureLengths = vector<int>();         // Vector of gesture lengths
     */
    
#if !BOOSTLIB
    normdist = new std::tr1::normal_distribution<float>();
    unifdist = new std::tr1::uniform_real<float>();
    rndnorm  = new std::tr1::variate_generator<std::tr1::mt19937, std::tr1::normal_distribution<float> >(rng, *normdist);
#endif
    
    // absolute weights
    abs_weights = vector<float>();
    
    // Offset for segmentation
    offS=vector<vector<float> >(ns);
    for (int k = 0; k < ns; k++)
    {
        offS[k] = vector<float>(config.inputDimensions);
        
        for (int j = 0; j < config.inputDimensions; j++)
            offS[k][j] = 0.0;
    }
    
}


//----------------------------------------------
//
// Learn Internal Configuration
//
void ofxGVF::learn(){
    
    if (gestureTemplates.size() > 0){
        
        
        // ADAPTATION OF THE TOLERANCE
        // ---------------------------
        float obsMeanRange = 0.0f;
        for (int gt=0; gt<gestureTemplates.size(); gt++) {
            for (int d=0; d<config.inputDimensions; d++)
                obsMeanRange += (gestureTemplates[gt].getMaxRange()[d] - gestureTemplates[gt].getMinRange()[d])
                /config.inputDimensions;
        }
        obsMeanRange /= gestureTemplates.size();
        parameters.tolerance = obsMeanRange / 3.0f;  // dividing by an heuristic factor [to be learned?]
        // ---------------------------
        
        featVariances.clear();
        
        config.inputDimensions = gestureTemplates[0].getTemplateRaw()[0].size(); //TODO - checked if good! need method!!
        
        // Set Scale and Rotation dimensions, according to input dimensions
        
        setStateDimensions(config.inputDimensions);
        
        // Initialize Variances
        initVariances(scale_dim, rotation_dim);
        
        initMat(X, parameters.numberParticles, pdim);           // Matrix of NS particles
        initVec(g, parameters.numberParticles);                 // Vector of gesture class
        initVec(w, parameters.numberParticles);                 // Weights
        
        has_learned = true; // ???: Should there be a verification that learning was successful?
    }
    
}

void ofxGVF::setStateDimensions(int input_dim) {
    
    if (input_dim == 2){
        
        //post(" dim 2 -> pdim 4");
        
        // state space dimension = 4
        // phase, speed, scale, rotation
        
        scale_dim = 1;
        rotation_dim = 1;
        
    }
    else if (input_dim == 3){
        
        // state space dimension = 8
        // phase, speed, scale (1d), rotation (3d)
        
        scale_dim = 3;
        rotation_dim = 3;
        
    }
    else {
        
        scale_dim = 1;
        rotation_dim = 0;
        
    }
    
    pdim = 2 + scale_dim + rotation_dim;
    
}

void ofxGVF::initVariances(int scaleDim, int rotationDim) {
    
    assert(parameters.scaleVariance.size() == scale_dim);
    assert(parameters.rotationVariance.size() == rotation_dim);
    
    featVariances = vector<float> (pdim);
    
    featVariances[0] = sqrt(parameters.phaseVariance);
    featVariances[1] = sqrt(parameters.speedVariance);
    for (int k = 0; k < scaleDim; k++)
        featVariances[2+k] = sqrt(parameters.scaleVariance[k]);
    for (int k = 0; k < rotationDim; k++)
        featVariances[2+scaleDim+k] = sqrt(parameters.rotationVariance[k]);
    
    // Spreading parameters: initial value for each variation (e.g. speed start at 1.0 [i.e. original speed])
    parameters.phaseInitialSpreading = 0.1;
    parameters.speedInitialSpreading = 1.0;
    parameters.scaleInitialSpreading = vector<float> (scaleDim);
    parameters.rotationInitialSpreading = vector<float> (rotationDim);
    // Spreading parameters: initial value for each variation (e.g. speed start at 1.0 [i.e. original speed])
    for (int k = 0; k < scaleDim; k++)
        parameters.scaleInitialSpreading[k]=1.0f;
    for (int k = 0;k < rotationDim; k++)
        parameters.rotationInitialSpreading[k]=0.0f;
    
}

// Destructor of the class
ofxGVF::~ofxGVF(){
#if !BOOSTLIB
    if(normdist != NULL)
        delete (normdist);
    if(unifdist != NULL)
        delete (unifdist);
#endif
    
    // should we free here other variables such X, ...??
    //TODO(baptiste)
    
    
    clear(); // not really necessary but it's polite ;)
    
}

////////////////////////////////////////////////////////////////
//
// ADD & FILL TEMPLATES FOR GESTURES
//
////////////////////////////////////////////////////////////////

//--------------------------------------------------------------
void ofxGVF::addGestureTemplate(ofxGVFGesture & gestureTemplate){
    
    //int inputDimension = gestureTemplate.getTemplateRaw(0)[0].size(); //TODO Define a method instead!!!
    int inputDimension = gestureTemplate.getNumberDimensions();
    
    
    if(minRange.size() == 0){
        minRange.resize(inputDimension);
        maxRange.resize(inputDimension);
    }
    
    for(int j = 0; j < inputDimension; j++){
        minRange[j] = INFINITY;
        maxRange[j] = -INFINITY;
    }
    
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

//--------------------------------------------------------------
// Clear the internal data (templates)
void ofxGVF::clear(){
    
    state = STATE_CLEAR;
    
    gestureTemplates.clear();
    
    //	gestureLengths.clear();
    mostProbableIndex = -1;
    mostProbableStatus.clear();
    //	numTemplates=-1;
    
    
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
            if (gestureTemplates.size() > 0){
                learn();
                spreadParticles(); // TODO provide setter for mean and range on init
            }
            state = _state;
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
    
    // use default means and ranges - taken from gvfhandler
    if (has_learned)
    {
        spreadParticles(parameters);
        //obsOffset.clear();
    }
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
    
    float spreadRangePhase = 0.3;
    float spreadRange = 0.1;
    int scalingCoefficients  = _parameters.scaleInitialSpreading.size();
    int numberRotationAngles = _parameters.rotationInitialSpreading.size();
    
    //post("scale %i rotation %i", scalingCoefficients, numberRotationAngles);
    
    
    // Spread particles using a uniform distribution
	//for(int i = 0; i < pdim; i++)
    for(int n = 0; n < ns; n++){
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
    //    logW.setConstant(0.0);
	
    //post("ngestures=%i",ngestures);
    
    // Spread uniformly the gesture class among the particles
	for(int n = 0; n < ns; n++){
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
    
    
	// we copy the initial means and ranges to be able to restart the algorithm
    meansCopy  = means;
    rangesCopy = ranges;
    
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
		for(int n = 0; n < ns; n++)
			X[n][i] = (rnduni() - 0.5) * ranges[i] + means[i];
    
    // Weights are also uniformly spread
    initweights();
    //    logW.setConstant(0.0);
	
    // Spread uniformly the gesture class among the particles
	for(int n = 0; n < ns; n++){
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
    for (int k = 0; k < ns; k++){
        w[k] = 1.0 / (float) ns;
    }
}

//--------------------------------------------------------------
float distance_weightedEuclidean(vector<float> x, vector<float> y, vector<float> w){
    int count = x.size();
    // the size must be > 0
    if (count <= 0)
        return 0;
    
    float dist = 0;
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
    
    
    
    // MAIN LOOP: same process for EACH particle (row n in X)
    for(int n = ns - 1; n >= 0; --n)
    {
        
        // Move the particle
        // Position respects a first order dynamic: p = p + v/L
        //cout << X[n][0] << " " << featVariances[0] << " " << X[n][1] << " " << featVariances[1] << " " << g[n] << " " << gestureTemplates[g[n]].getTemplateLength() << endl;
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
            
            //setVec(vref, R_single[pgi][frameindex]);
            
            
            vector<float> vobs(config.inputDimensions);
            if (config.translate)
                for (int j=0; j < config.inputDimensions; j++)
                    vobs[j]=obs[j]-offS[n][j];
            else
                setVec(vobs, obs);
            
            
            
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
    }
    //    }
    
    // normalize weights and compute criterion for degeneracy
    //	w /= w.sum();
    //	float neff = 1./w.dot(w);
    float dotProdw = 0.0;
    for (int k = 0; k < ns; k++){
        w[k] /= sumw;
        dotProdw+=w[k]*w[k];
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
    vector<float> c(ns);
    
    c[0] = 0;
    for(int i = 1; i < ns; i++)
        c[i] = c[i-1] + w[i];
    int i = 0;
    float u0 = rnduni()/ns;
    
    
    // defining here the number of particles allocated to reinitialisation
    // used for segmentation
    int free_pool = 0;
    if (config.segmentation)
        free_pool = round(3*ns/100);
    
    for (int j = 0; j < ns; j++)
    {
        float uj = u0 + (j + 0.) / ns;
        
        while (uj > c[i] && i < ns - 1){
            i++;
        }
        
        for (int kk=0;kk<X[0].size();kk++)
            X[j][kk] = oldX[i][kk];
        g[j] = oldG[i];
        
        w[j] = 1.0/(float)ns;
    }
    
    for (int j = 0; j < free_pool; j++){
        
        int index = round(rnduni()*parameters.numberParticles);
        
        if (index == parameters.numberParticles)
            index = 0;
        
        w[index] = 1.0/(float)(ns*ns);
        
        
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
        for (int j = 0; j < ns; j++)
            sumw += w[j];
        for (int j = 0; j < ns; j++)
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
    
    // get the number of gestures in the vocabulary
    //	unsigned int ngestures = numTemplates+1;
    
    //    vector< vector<float> > es;
    setMat(status, 0.0f, getNumberOfGestureTemplates(), pdim + 1);   // rows are gestures, cols are features + probabilities
	//printMatf(es);
    
	// compute the estimated features by computing the expected values
    // sum ( feature values * weights)
	for(int n = 0; n < ns; n++){
        int gi = g[n];
        for(int m = 0; m < pdim; m++){
            //            cout << "S[" << gi << "][" << m << "] " << S[gi][m] << endl;
            //            cout << "X[" << n << "][" << m << "] " << X[n][m] << endl;
            //            cout << "ues: w[" << n << "] " << w[n] << endl;
            status[gi][m] += X[n][m] * w[n];
        }
		status[gi][pdim] += w[n]; // probability
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
// Returns the probabilities of each gesture. This probability is conditionnal
// because it depends on the other gestures in the vocabulary:
// probability to be in gesture A knowing that we have gesture A, B, C, ... in the vocabulary
vector<float> ofxGVF::getGestureProbabilities()
{
    //	unsigned int ngestures = numTemplates+1;
    
	vector<float> gp(getNumberOfGestureTemplates());
    setVec(gp, 0.0f);
	for(int n = 0; n < ns; n++)
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
    parameters = _parameters;
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
    return ns; // Return the number of particles
}

//--------------------------------------------------------------
// Update the resampling threshold used to avoid degeneracy problem
void ofxGVF::setResamplingThreshold(int _resamplingThreshold){
    if (_resamplingThreshold >= ns) _resamplingThreshold = floor(ns/2.0f); // TODO: we should provide feedback to the GUI!!! maybe a get max resampleThresh func??
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
}

//--------------------------------------------------------------
float ofxGVF::getDistribution(){
    return nu;
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
    featVariances[0] = phaseVariance;
}

//--------------------------------------------------------------
float ofxGVF::getPhaseVariance(){
    return parameters.phaseVariance;
}

//--------------------------------------------------------------
void ofxGVF::setSpeedVariance(float speedVariance){
    parameters.speedVariance = speedVariance;
    featVariances[1] = speedVariance;
}

//--------------------------------------------------------------
float ofxGVF::getSpeedVariance(){
    return parameters.speedVariance;
}

//--------------------------------------------------------------
void ofxGVF::setScaleVariance(float scaleVariance){
    setScaleVariance(vector<float>(scale_dim, scaleVariance));
}

//--------------------------------------------------------------
void ofxGVF::setScaleVariance(vector<float> scaleVariance){
    parameters.scaleVariance = scaleVariance;
    initVariances(scale_dim, rotation_dim);
}

//--------------------------------------------------------------
vector<float> ofxGVF::getScaleVariance(){
    return parameters.scaleVariance;
}

//--------------------------------------------------------------
void ofxGVF::setRotationVariance(float rotationVariance){
    setRotationVariance(vector<float>(rotation_dim, rotationVariance));
}

//--------------------------------------------------------------
void ofxGVF::setRotationVariance(vector<float> rotationVariance){
    
    parameters.rotationVariance = rotationVariance;
    initVariances(scale_dim, rotation_dim);
    
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

//--------------------------------------------------------------
string ofxGVF::getStateAsString(ofxGVFState state){
    switch(state){
        case STATE_CLEAR:
            return "STATE_CLEAR";
            break;
        case STATE_WAIT:
            return "STATE_WAIT";
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


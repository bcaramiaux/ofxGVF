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
#include "ext.h"

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
    defaultParameters.dynamicsVariance = vector<float>(1,0.00001f);
    defaultParameters.scalingsVariance = vector<float>(1,0.00001f);
    
    setup(_config,  defaultParameters);

}

//--------------------------------------------------------------
void ofxGVF::setup(ofxGVFConfig _config, ofxGVFParameters _parameters){
    
    clear(); // just in case
    
    // Set configuration and parameters
    config      = _config;
    parameters  = _parameters;

    // absolute weights
    abs_weights = vector<float>();

    // flag
    has_learned = false;
}


//--------------------------------------------------------------
void ofxGVF::restart(){

    // TODO: switch to learn maybe? or initStateValues + initPrior?
    spreadParticles();
    
    // and maybe more after...
    
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
        
        dynamicsDim = 2;    // speed + acceleration
        scalingsDim = config.inputDimensions;
        
        // Init state space
        initVec(classes, parameters.numberParticles);                 // Vector of gesture class
        initVec(alignment, parameters.numberParticles);                 // Vector of phase values (alignment)
        initMat(dynamics, parameters.numberParticles, dynamicsDim);               // Order 2
        initMat(scalings, parameters.numberParticles, scalingsDim);  // Matrix of scaling
        initMat(offsets,parameters.numberParticles, config.inputDimensions);
        initVec(weights, parameters.numberParticles);                 // Weights
        
        
        initStateValues();      // allocate partcles meoru  and compute init values
        initPrior();            // prior on init state values
        initNoiseParameters();  // init noise parameters (transition and likelihood)
        

        // weighted dimensions in case: default is not weighted
        dimWeights = vector<float> (config.inputDimensions);
        for(int k = 0; k < config.inputDimensions; k++) dimWeights[k] = 1.0 / config.inputDimensions;

        
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
// INIT VALUES OF STATES
//=========================================================
void ofxGVF::initStateValues() {
    
    
#if BOOSTLIB
	boost::uniform_real<float> ur(0,1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
    
    // Spread particles using a uniform distribution
    float spreadRange = 0.1;
    for(int n = 0; n < parameters.numberParticles; n++)
    {
        classes[n]   = n % gestureTemplates.size();
        alignment[n] = (rnduni() - 0.5) * spreadRange + 0.0;    // spread phase

        for(int l = 0; l < dynamics[n].size(); l++) dynamics[n][l] = (rnduni() - 0.5) * spreadRange + 1.0;    // spread dynamics
        for(int l = 0; l < scalings[n].size(); l++) scalings[n][l] = (rnduni() - 0.5) * spreadRange + 1.0;    // spread scalings
        for(int l = 0; l < offsets[n].size(); l++) offsets[n][l] = 0.0;
    }
    
}

//--------------------------------------------------------------
void ofxGVF::initStateValues(int particleIndex) {
    
#if BOOSTLIB
	boost::uniform_real<float> ur(0,1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
    
    int     n = particleIndex;
    float   spreadRange = 0.1;    // can be passed in argument
    
    classes[n]   = n % gestureTemplates.size();
    alignment[n] = (rnduni() - 0.5) * spreadRange + 0.0;    // spread phase
    
    for(int l = 0; l < dynamics[n].size(); l++) dynamics[n][l] = (rnduni() - 0.5) * spreadRange + 1.0;    // spread dynamics
    for(int l = 0; l < scalings[n].size(); l++) scalings[n][l] = (rnduni() - 0.5) * spreadRange + 1.0;    // spread scalings
    for(int l = 0; l < offsets[n].size(); l++) offsets[n][l] = 0.0;
    
}

//--------------------------------------------------------------
void ofxGVF::initPrior() {
    for (int k = 0; k < parameters.numberParticles; k++){
        weights[k] = 1.0 / (float) parameters.numberParticles;
    }
}

//--------------------------------------------------------------
void ofxGVF::initPrior(int particleIndex) {
    weights[particleIndex] = 1.0 / (float) parameters.numberParticles;
}


//--------------------------------------------------------------
void ofxGVF::initNoiseParameters() {
    
    
    //
    // NOISE (ADDITIVE GAUSSIAN NOISE)
    //=========================================================
    
    if (parameters.dynamicsVariance.size() != dynamicsDim) {
        float variance = parameters.dynamicsVariance[0];
        parameters.dynamicsVariance.resize(dynamicsDim);
        for (int k=0; k<dynamicsDim; k++) parameters.dynamicsVariance[k] = variance;
    }
    if (parameters.scalingsVariance.size() != scalingsDim) {
        float variance = parameters.scalingsVariance[0];
        parameters.scalingsVariance.resize(scalingsDim);
        for (int k=0; k<scalingsDim; k++) parameters.scalingsVariance[k] = variance;
    }

    
    //    if (featVariances.size() != pdim) featVariances.resize(pdim);
//    
//    
//    // init the dynamics of the states
//    // dynamics are goven by the gaussian variances as:
//    //      x_t = A x_{t-1} + v_t
//    // where v_t is the gaussian noise, x_t the state space at t
//    
//    featVariances[0] = sqrt(parameters.phaseVariance);
//    featVariances[1] = sqrt(parameters.speedVariance);
//    for (int k = 0; k < scale_dim; k++)
//        featVariances[2+k] = sqrt(parameters.scaleVariance[k]);
//    for (int k = 0; k < rotation_dim; k++)
//        featVariances[2+scale_dim+k] = sqrt(parameters.rotationVariance[k]);
    
    
    
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
                learn();
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


int ofxGVF::getDynamicsDim(){
    return dynamicsDim;
}
int ofxGVF::getScalingsDim(){
    return scalingsDim;
}



////////////////////////////////////////////////////////////////
//
// CORE FUNCTIONS & MATH
//
////////////////////////////////////////////////////////////////


//--------------------------------------------------------------
// Spread the particles by sampling values from intervals given my their means and ranges.
// Note that the current implemented distribution for sampling the particles is the uniform distribution
void ofxGVF::spreadParticles(){
    
    
    // USE BOOST FOR UNIFORM DISTRIBUTION!!!!
    // deprecated class should use uniform_real_distribution
#if BOOSTLIB
	boost::uniform_real<float> ur(0,1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
    
    if (has_learned){

        // INIT VALUES OF STATES
        //=========================================================
        
        float spreadRange = 0.1;
        for(int n = 0; n < parameters.numberParticles; n++)
        {
            classes[n]   = n % gestureTemplates.size();
            alignment[n] = (rnduni() - 0.5) * spreadRange + 0.0;    // spread phase
            
            for(int l = 0; l < dynamics[n].size(); l++) dynamics[n][l] = (rnduni() - 0.5) * spreadRange + 1.0;    // spread dynamics
            for(int l = 0; l < scalings[n].size(); l++) scalings[n][l] = (rnduni() - 0.5) * spreadRange + 1.0;    // spread scalings
            for(int l = 0; l < offsets[n].size(); l++) offsets[n][l] = 0.0;
        }
 
        // PRIOR
        //=========================================================
        initPrior();
        
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
    

    // zero abs weights
    for(int i = 0 ; i < getNumberOfGestureTemplates(); i++){
        abs_weights[i] = 0.0;
    }
    
    float sumw = 0.0;

    // MAIN LOOP: same process for EACH particle (row n in X)
    for(int n = 0; n<parameters.numberParticles; n++)
    {

        updatePrior(n);             // update particle values in X[n]
		updateLikelihood(obs, n);   // update likelihood: here the weight of the particle n given by w[n]
        
        sumw += weights[n];   // sum weights
    }
    
    // normalize the weights and compute the resampling criterion
    float dotProdw = 0.0;
    for (int k = 0; k < parameters.numberParticles; k++){
        weights[k] /= sumw;
        dotProdw   += weights[k] * weights[k];
    }

    // avoid degeneracy (no particles active, i.e. weights = 0) by resampling
    // around the active particles
	if( (1./dotProdw) < parameters.resamplingThreshold)
    {
        resampleAccordingToWeights(obs);
        initPrior();
    }

}


//--------------------------------------------------------------
void ofxGVF::updatePrior(int particleIndex) {

    
    rndnorm = new tr1::variate_generator<tr1::mt19937, tr1::normal_distribution<float> >(rng, *normdist);
    
    // convenient ...
    int n = particleIndex;
    
    // Position respects a first order dynamic: p = p + v/L + a
    alignment[n] += (*rndnorm)() * sqrt(parameters.phaseVariance) +
        dynamics[n][0]/gestureTemplates[classes[n]].getTemplateLength();
    
    // Move dynamics
    for(int l= 0; l < dynamics[n].size(); l++)
        dynamics[n][l] += (*rndnorm)() * parameters.dynamicsVariance[l];

    // Move scalings
    for(int l= 0; l < scalings[n].size(); l++)
        scalings[n][l] += (*rndnorm)() * parameters.scalingsVariance[l];
    
}


//--------------------------------------------------------------
void ofxGVF::updateLikelihood(vector<float> obs, int particleIndex) {
    
    // convenient stuff ...
    int n = particleIndex;
    vector<float> x_n = X[n];
    

    // Test if the particle's phase is > 1 or < 0
    if(alignment[n] < 0.0 || alignment[n] > 1.0)
    {
        weights[n] = 0.0;
        
    }
    else {       // ...otherwise we propagate the particle's values and update its weight
        
        
        // vref is value in the template given by the phase of the particle at particleIndex
        vector<float> vref(config.inputDimensions);
        
        int frameindex = min((int)(gestureTemplates[classes[n]].getTemplateLength() - 1),
                             (int)(floor(alignment[n] * gestureTemplates[classes[n]].getTemplateLength() ) ) );

        setVec(vref, gestureTemplates[classes[n]].getTemplate()[frameindex]);
        
        
        // SCALING
        for (int k=0;k < config.inputDimensions; k++) {
            if (config.normalization){
                vref[k] = vref[k] / globalNormalizationFactor;
            }
            vref[k] *= scalings[n][k];
        }
        
        vector<float> vobs(config.inputDimensions);
        if (config.translate)
            for (int j=0; j < config.inputDimensions; j++)
                vobs[j]=obs[j]-offsets[n][j];
        else
            setVec(vobs, obs);
        
        
        // if normalization
        if (config.normalization)
            for (int kk=0; kk<vobs.size(); kk++)
                vobs[kk] = vobs[kk] / globalNormalizationFactor;
        
        
        // define weights here on the dimension if needed and compute the distance
        float dist = distance_weightedEuclidean(vref,vobs,dimWeights) * 1 / (parameters.tolerance * parameters.tolerance);
        
        
        if(parameters.distribution == 0.0f){    // Gaussian distribution
            weights[n]   *= exp(-dist);
            abs_weights[classes[n]] += exp(-dist);
        }
        else {            // Student's distribution
            weights[n]   *= pow(dist/nu + 1, -nu/2 - 1);    // dimension is 2 .. pay attention if editing]
        }
        
        
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
    
    vector<float>           oldClasses;
    vector<float>           oldAlignment;
    vector< vector<float> > oldDynamics;
    vector< vector<float> > oldScalings;
    
    setVec(oldClasses,   classes);
    setVec(oldAlignment, alignment);
    setMat(oldDynamics,  dynamics);
    setMat(oldScalings,  scalings);

    vector<float> c(numOfPart);
    
    c[0] = 0;
    for(int i = 1; i < numOfPart; i++)
        c[i] = c[i-1] + weights[i];
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
        
        for (int l=0;l<dynamicsDim;l++)
            dynamics[j][l] = oldDynamics[i][l];
        
        for (int l=0;l<scalingsDim;l++)
            scalings[j][l] = oldScalings[i][l];

        classes[j] = oldClasses[i];
        
        weights[j] = 1.0/(float)numOfPart;
    }
    
    
    for (int j = 0; j < free_pool; j++){
        
        int index = round(rnduni()*numOfPart);
        if (index == numOfPart) index = 0;

        initStateValues(index);
        weights[index] = 1.0/(float)(numOfPart * numOfPart);
        
//        float spreadRange = 0.02;
//        int scalingCoefficients  = parameters.scaleInitialSpreading.size();
//        int numberRotationAngles = parameters.rotationInitialSpreading.size();
//        // Spread particles using a uniform distribution
//        X[index][0] = (rnduni() - 0.5) * spreadRange + parameters.phaseInitialSpreading;
//        X[index][1] = (rnduni() - 0.5) * spreadRange + parameters.speedInitialSpreading;
//        for (int nn=0; nn<scalingCoefficients; nn++)
//            X[index][2+nn] = (rnduni() - 0.5) * spreadRange + parameters.scaleInitialSpreading[nn];
//        for (int nn=0; nn<numberRotationAngles; nn++)
//            X[index][2+scalingCoefficients+nn] = (rnduni() - 0.5) * 0.0 + parameters.rotationInitialSpreading[nn];
        
        if (config.translate)
            for (int jj=0; jj<config.inputDimensions; jj++) offsets[index][jj]=obs[jj];
        
        classes[index] = index % getNumberOfGestureTemplates(); // distribute particles across templates
        
    }
    

    
    if (config.segmentation)
    {
        float sumw = 0.0;
        for (int j = 0; j < numOfPart; j++)
            sumw += weights[j];
        for (int j = 0; j < numOfPart; j++)
            weights[j] /= sumw;
    }
    
    
}

//--------------------------------------------------------------
// Step function is the function called outside for inference. It
// has been originally created to be able to infer on a new observation or
// a set of observation.
void ofxGVF::infer(vector<float> obs){

    particleFilter(obs);
    updateEstimatedStatus();
    
}


void ofxGVF::updateEstimatedStatus(){
  
    
    int numOfPart = parameters.numberParticles;
    
    setVec(estimatedAlignment, 0.0f, getNumberOfGestureTemplates());            // rows are gestures
    setMat(estimatedDynamics,  0.0f, getNumberOfGestureTemplates(), dynamicsDim);  // rows are gestures, cols are features + probabilities
    setMat(estimatedScalings,  0.0f, getNumberOfGestureTemplates(), scalingsDim);   // rows are gestures, cols are features + probabilities
    setVec(likelihoods, 0.0f, getNumberOfGestureTemplates());            // rows are gestures
    
	// compute the estimated features and likelihoods
	for(int n = 0; n < numOfPart; n++)
    {
        estimatedAlignment[classes[n]] += alignment[n] * weights[n];
        for(int m = 0; m < dynamicsDim; m++) estimatedDynamics[classes[n]][m] += dynamics[n][m] * weights[n];
        for(int m = 0; m < scalingsDim; m++) estimatedScalings[classes[n]][m] += scalings[n][m] * weights[n];
		likelihoods[classes[n]] += weights[n];
    }
	
    // calculate most probable index during scaling...
    float maxProbability = 0.0f;
    mostProbableIndex = -1;
    
	for(int gi = 0; gi < getNumberOfGestureTemplates(); gi++)
    {
        estimatedAlignment[gi] /= likelihoods[gi];
        for(int m = 0; m < dynamicsDim; m++) estimatedDynamics[gi][m] /= likelihoods[gi];
        for(int m = 0; m < scalingsDim; m++) estimatedScalings[gi][m] /= likelihoods[gi];
        
        if(likelihoods[gi] > maxProbability){
            maxProbability      = likelihoods[gi];
            mostProbableIndex   = gi;
        }
	}
    
    // update outcomes
    UpdateOutcomes();

}

////////////////////////////////////////////////////////////////
//
// PROBABILITY AND TEMPLATE ACCESS
//
////////////////////////////////////////////////////////////////



//--------------------------------------------------------------
// Returns the index of the currently recognized gesture
int ofxGVF::getMostProbableGestureIndex(){
    return mostProbableIndex;
}

//--------------------------------------------------------------
// Returns the estimates features.
ofxGVFOutcomes ofxGVF::getOutcomes() {
    return outcomes;
}

//--------------------------------------------------------------
void ofxGVF::UpdateOutcomes() {
    
    // most probable gesture index
    outcomes.most_probable = mostProbableIndex;
    
    // Fill estimation for each gesture
    for (int gi = 0; gi < gestureTemplates.size(); ++gi) {
        
        ofxGVFEstimation estimation;
        
        estimation.probability = likelihoods[gi];
        estimation.alignment = alignment[gi];
        for (int j = 0; j < dynamicsDim; ++j) estimation.dynamics[j] = estimatedDynamics[gi][j];
        for (int j = 0; j < scalingsDim; ++j) estimation.scalings[j] = estimatedScalings[gi][j];
        
        // push estimation for gesture gi in outcomes
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
		gp[classes[n]] += weights[n];
    
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
    
    // if the number of particles has changed, we have to re-allocate matrices
    if (_parameters.numberParticles != parameters.numberParticles)
    {
        parameters = _parameters;
        
        if (parameters.numberParticles < 4)     // minimum number of particles allowed
            parameters.numberParticles = 4;
        
//        initMat(X, parameters.numberParticles, pdim);           // Matrix of NS particles
//        initVec(g, parameters.numberParticles);                 // Vector of gesture class
//        initVec(w, parameters.numberParticles);                 // Weights
//        
//        // Offset for segmentation
//        offS=vector<vector<float> >(parameters.numberParticles);
//        for (int k = 0; k < parameters.numberParticles; k++)
//        {
//            offS[k] = vector<float>(config.inputDimensions);
//            for (int j = 0; j < config.inputDimensions; j++)
//                offS[k][j] = 0.0;
//        }

        learn();
        
        if (parameters.numberParticles <= parameters.resamplingThreshold) {
            parameters.resamplingThreshold = parameters.numberParticles / 4;
        }
        
        //spreadParticles();
    }
    else
        parameters = _parameters;
    

}

ofxGVFParameters ofxGVF::getParameters(){
    return parameters;
}

//--------------------------------------------------------------
// Update the number of particles
void ofxGVF::setNumberOfParticles(int numberOfParticles){
  
    parameters.numberParticles = numberOfParticles;
    
    if (parameters.numberParticles < 4)     // minimum number of particles allowed
        parameters.numberParticles = 4;
    
    learn();
    
    if (parameters.numberParticles <= parameters.resamplingThreshold) {
        parameters.resamplingThreshold = parameters.numberParticles / 4;
    }

}

//--------------------------------------------------------------
int ofxGVF::getNumberOfParticles(){
    return parameters.numberParticles; // Return the number of particles
}

//--------------------------------------------------------------
// Update the resampling threshold used to avoid degeneracy problem
void ofxGVF::setResamplingThreshold(int _resamplingThreshold){
    if (_resamplingThreshold >= parameters.numberParticles)
        _resamplingThreshold = floor(parameters.numberParticles/2.0f);
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
    if (_tolerance <= 0.0) _tolerance = 0.1;
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


// VARIANCE COEFFICIENTS: PHASE
//--------------------------------------------------------------
void ofxGVF::setPhaseVariance(float phaseVariance){
    parameters.phaseVariance = phaseVariance;
}
//--------------------------------------------------------------
float ofxGVF::getPhaseVariance(){
    return parameters.phaseVariance;
}


// VARIANCE COEFFICIENTS: DYNAMICS
//--------------------------------------------------------------
void ofxGVF::setDynamicsVariance(float dynVariance, int dim){
    // here dim should start at 1!!!
    if (dim<parameters.dynamicsVariance.size())
        parameters.dynamicsVariance[dim-1] = dynVariance;
}
//--------------------------------------------------------------
void ofxGVF::setDynamicsVariance(vector<float> dynVariance){
    parameters.dynamicsVariance = dynVariance;
}
//--------------------------------------------------------------
vector<float> ofxGVF::getDynamicsVariance(){
    return parameters.dynamicsVariance;
}


// VARIANCE COEFFICIENTS: SCALINGS
//--------------------------------------------------------------
void ofxGVF::setScalingsVariance(float scaleVariance, int dim){
    // here dim should start at 1!!!
    if (dim<parameters.scalingsVariance.size())
        parameters.scalingsVariance[dim-1] = scaleVariance;
}
//--------------------------------------------------------------
void ofxGVF::setScalingsVariance(vector<float> scaleVariance){
     parameters.scaleVariance = scaleVariance;
}
//--------------------------------------------------------------
vector<float> ofxGVF::getScalingsVariance(){
    return parameters.scalingsVariance;
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


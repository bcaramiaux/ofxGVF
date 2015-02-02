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

#include <Accelerate/Accelerate.h>

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tr1/memory>
#include <unistd.h>



using namespace std;


// used math functions -------
vector<vector<float> > return_RotationMatrix_3d(float phi, float theta, float psi);
float distance_weightedEuclidean(vector<float> x, vector<float> y, vector<float> w);
// ---------------------------





////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
//              CONSTRUCTORS and RESTART (GENERAL)
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
    defaultParameters.numberParticles       = 1000;
    defaultParameters.tolerance             = 0.2f;
    defaultParameters.resamplingThreshold   = 250;
    defaultParameters.distribution          = 0.0f;
    defaultParameters.alignmentVariance     = sqrt(0.000001f);
    defaultParameters.dynamicsVariance      = vector<float>(1,sqrt(0.01f));
    defaultParameters.scalingsVariance      = vector<float>(1,sqrt(0.0000001f));
    defaultParameters.predictionLoops       = 1;
    
    setup(_config,  defaultParameters);

}

//--------------------------------------------------------------
void ofxGVF::setup(ofxGVFConfig _config, ofxGVFParameters _parameters){
    
    clear(); // just in case
    
    // Set configuration and parameters
    config      = _config;
    parameters  = _parameters;

    normdist = new std::tr1::normal_distribution<float>();
    unifdist = new std::tr1::uniform_real<float>();
    rndnorm  = new std::tr1::variate_generator<std::tr1::mt19937, std::tr1::normal_distribution<float> >(rng, *normdist);
    
    // flag
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
void ofxGVF::clear(){
    state = STATE_CLEAR;
    gestureTemplates.clear();
    mostProbableIndex = -1;

}




////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
//  MANAGE GESTURE / TEMPLATES
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


//--------------------------------------------------------------
void ofxGVF::addGestureTemplate(ofxGVFGesture & gestureTemplate){
    
    if (getState() != ofxGVF::STATE_LEARNING)
        setState(ofxGVF::STATE_LEARNING);
    
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
    
}

//----------------------------------------------
vector<float>& ofxGVF::getGestureTemplateSample(int gestureIndex, float cursor) {
    
    // cursor shoudl be between 0 and 1
    int frameindex = min((int)(gestureTemplates[gestureIndex].getTemplateLength() - 1),
                         (int)(floor(cursor * gestureTemplates[gestureIndex].getTemplateLength() ) ) );

    return gestureTemplates[gestureIndex].getTemplate()[frameindex];
    
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
    
    if (gestureTemplates.size() > 0)
    {
        
        // get the number of dimension in templates
        config.inputDimensions = gestureTemplates[0].getTemplateDimension();
        
        dynamicsDim = 2;    // speed + acceleration
        scalingsDim = config.inputDimensions;
        
        // Init state space
        initVec(classes, parameters.numberParticles);                           // Vector of gesture class
        initVec(alignment, parameters.numberParticles);                         // Vector of phase values (alignment)
        initMat(dynamics, parameters.numberParticles, dynamicsDim);
        initMat(scalings, parameters.numberParticles, scalingsDim);             // Matrix of scaling
        initMat(offsets, parameters.numberParticles, config.inputDimensions);
        initVec(weights, parameters.numberParticles);                           // Weights
        
        // bayesian elements
        initVec(prior, parameters.numberParticles);
        initVec(posterior, parameters.numberParticles);
        initVec(likelihood, parameters.numberParticles);
        
        
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
        
        //post("%i globalnorm=%f",config.normalization,globalNormalizationFactor);
        
        
        // only for logs
        if (config.logOn) {
            vecRef = vector<vector<float> > (parameters.numberParticles);
            vecObs = vector<float> (config.inputDimensions);
            stateNoiseDist = vector<float> (parameters.numberParticles);
        }
        
    }
}


// GENERAL FUNCTIONS FOR STATE SPACE BAYESIAN MODEL

//--------------------------------------------------------------
void ofxGVF::initPrior() {
    
    // PATICLE FILTERING
    for (int k = 0; k < parameters.numberParticles; k++)
        initPrior(k);
    
}

//--------------------------------------------------------------
void ofxGVF::initPrior(int pf_n) {
    
    float range = 0.1;
    
    // random generators
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
    
    classes[pf_n]   = pf_n % gestureTemplates.size();
    alignment[pf_n] = (rnduni() - 0.5) * range;    // spread phase
    
    for(int l = 0; l < dynamics[pf_n].size(); l++) dynamics[pf_n][l] = (rnduni() - 0.5) * range * 3 + 1.0;    // spread dynamics
    for(int l = 0; l < scalings[pf_n].size(); l++) scalings[pf_n][l] = (rnduni() - 0.5) * range * 3 + 1.0;    // spread scalings
    
    for(int l = 0; l < offsets[pf_n].size(); l++) offsets[pf_n][l] = 0.0;
    
    //weights[pf_n] = 1.0 / (float) parameters.numberParticles;
    prior[pf_n] = 1.0 / (float) parameters.numberParticles;

    // set the posterior to the prior at the initialization
    posterior[pf_n] = prior[pf_n];
    
}

//--------------------------------------------------------------
void ofxGVF::initNoiseParameters() {

    // NOISE (ADDITIVE GAUSSIAN NOISE)
    // ---------------------------
    
    if (parameters.dynamicsVariance.size() != dynamicsDim)
    {
        float variance = parameters.dynamicsVariance[0];
        parameters.dynamicsVariance.resize(dynamicsDim);
        for (int k=0; k<dynamicsDim; k++)
            parameters.dynamicsVariance[k] = variance;
    }
    
    if (parameters.scalingsVariance.size() != scalingsDim)
    {
        float variance = parameters.scalingsVariance[0];
        parameters.scalingsVariance.resize(scalingsDim);
        for (int k=0; k<scalingsDim; k++)
            parameters.scalingsVariance[k] = variance;
    }
    
    
    // ADAPTATION OF THE TOLERANCE IF DEFAULT PARAMTERS
    // ---------------------------
    
    float obsMeanRange = 0.0f;
    for (int gt=0; gt<gestureTemplates.size(); gt++) {
        for (int d=0; d<config.inputDimensions; d++)
            obsMeanRange += (gestureTemplates[gt].getMaxRange()[d] - gestureTemplates[gt].getMinRange()[d])
            /config.inputDimensions;
    }
    obsMeanRange /= gestureTemplates.size();
    parameters.tolerance = obsMeanRange / 8.0f;  // dividing by an heuristic factor [to be learned?]
    
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

vector<int> ofxGVF::getClasses(){
    return classes;
}
vector<float> ofxGVF::getAlignment(){
    return alignment;
}
vector<float> ofxGVF::getEstimatedAlignment(){
    return estimatedAlignment;
}
vector< vector<float> > ofxGVF::getDynamics(){
    return dynamics;
}
vector< vector<float> > ofxGVF::getEstimatedDynamics(){
    return estimatedDynamics;
}
vector< vector<float> > ofxGVF::getScalings(){
    return scalings;
}
vector< vector<float> > ofxGVF::getEstimatedScalings(){
    return estimatedScalings;
}
vector<float> ofxGVF::getEstimatedLikelihoods(){
    return estimatedLikelihoods;
}
vector<float> ofxGVF::getWeights(){
    return weights;
}
vector<float> ofxGVF::getPrior(){
    return prior;
}

// only for logs
vector<vector<float> > ofxGVF::getVecRef() {
    return vecRef;
}
vector<float> ofxGVF::getVecObs() {
    return vecObs;
}

vector<float> ofxGVF::getStateNoiseDist(){
    return stateNoiseDist;
}


int ofxGVF::getScalingsDim(){
    return scalingsDim;
}


//--------------------------------------------------------------
void ofxGVF::restart(){
    initPrior();
}







////////////////////////////////////////////////////////////////
//
//
//  ******************
//  PARTICLE FILTERING
//  ******************
//
//
//  updatePrior(p) * dynamics of the state space, compute prior distribution
//
//  updateLikelihood(obs,p) * compute observation distribution
//
//  updatePosterior(p) *
//
//  update(p) * main inference function calling previous ones
//
////////////////////////////////////////////////////////////////




//--------------------------------------------------------------
//  updatePrior(p) * dynamics of the state space, compute prior distribution
//--------------------------------------------------------------
void ofxGVF::updatePrior(int n) {
    
    // Update alignment / dynamics / scalings
    alignment[n] += (*rndnorm)() * parameters.alignmentVariance + dynamics[n][0]/gestureTemplates[classes[n]].getTemplateLength();
    
    
    for(int l= 0; l < dynamics[n].size(); l++)  dynamics[n][l] += (*rndnorm)() * parameters.dynamicsVariance[l];
    for(int l= 0; l < scalings[n].size(); l++)  scalings[n][l] += (*rndnorm)() * parameters.scalingsVariance[l];
 
    // update prior (bayesian incremental inference)
    prior[n] = posterior[n];
}



//--------------------------------------------------------------
//  updateLikelihood(obs,p) * compute observation distribution
//--------------------------------------------------------------
void ofxGVF::updateLikelihood(vector<float> obs, int n) {
    
    
    // OBSERVATIONS
    // ============
    vector<float> vobs(config.inputDimensions);
    setVec(vobs, obs);
    
    if (config.translate)
        for (int j=0; j < config.inputDimensions; j++) vobs[j] = vobs[j] - offsets[n][j];
    if (config.normalization)
        for (int kk=0; kk<vobs.size(); kk++) vobs[kk] = vobs[kk] / globalNormalizationFactor;

    
    // TEMPLATE SAMPLE GIVEN FROM STATE VALUES
    // =======================================
    
    if(alignment[n] < 0.0)      alignment[n] = fabs(alignment[n]);  // re-spread at the beginning
    else if(alignment[n] > 1.0) alignment[n] = fabs(2.0-alignment[n]); // re-spread at the end
    
    // take vref from template at the given alignment
    vector<float> vref = getGestureTemplateSample(classes[n], alignment[n]);
    
    // Apply scaling coefficients
    for (int k=0;k < config.inputDimensions; k++)
    {
        if (config.normalization) vref[k] = vref[k] / globalNormalizationFactor;
        vref[k] *= scalings[n][k];
    }
    

    // COMPUTE LIKELIHOOD
    // ==================
    
    // weighted euclidean distance
    float dist = distance_weightedEuclidean(vref,vobs,dimWeights);
    
    if(parameters.distribution == 0.0f){    // Gaussian distribution
        likelihood[n] = exp(- dist * 1 / (parameters.tolerance * parameters.tolerance));
    }
    else {            // Student's distribution
        likelihood[n] = pow(dist/nu + 1, -nu/2 - 1);    // dimension is 2 .. pay attention if editing]
    }
    
    // if log on keep track on vref and vobs
    if (config.logOn){
        vecRef.push_back(vref);
        vecObs = vobs;
    }
    
}



//--------------------------------------------------------------
//  updatePosterior(p) *
//--------------------------------------------------------------
void ofxGVF::updatePosterior(int n) {
    posterior[n]  = prior[n] * likelihood[n];
}



//--------------------------------------------------------------
//  update(p) *
//--------------------------------------------------------------
void ofxGVF::update(vector<float> & obs){
    

    if (getState() != ofxGVF::STATE_FOLLOWING)
        setState(ofxGVF::STATE_FOLLOWING);
    
    
    // If logging
    if (config.logOn){
        vecRef.clear();
        stateNoiseDist.clear();
    }
    
    
    // INFERENCE
    // =========
    
    // for each particle: perform updates of state space / likelihood / prior (weights)
    float sumw = 0.0;
    for(int n = 0; n< parameters.numberParticles; n++)
    {
        
        for (int m=0; m<parameters.predictionLoops; m++){
        updatePrior(n);
		updateLikelihood(obs, n);
        updatePosterior(n);
        }
    
        sumw += posterior[n];   // sum posterior to normalise the distrib afterwards
    }
    
    
    // RESAMPLING if needed
    // ====================
    
    // normalize the weights and compute the resampling criterion
    float dotProdw = 0.0;
    for (int k = 0; k < parameters.numberParticles; k++){
        posterior[k] /= sumw;
        dotProdw   += posterior[k] * posterior[k];
    }
    // avoid degeneracy (no particles active, i.e. weights = 0) by resampling
	if( (1./dotProdw) < parameters.resamplingThreshold)
        resampleAccordingToWeights(obs);
    
    
    
    // ESTIMATES for each Gesture
    // ==========================
    
    estimates();

    
}



//--------------------------------------------------------------
//  resampleAccordingToWeights(obs)
//--------------------------------------------------------------
void ofxGVF::resampleAccordingToWeights(vector<float> obs)
{
    
    // random generator: uniform distribution
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
    
    // covennient
    int numOfPart = parameters.numberParticles;
    
    // cumulative dist
    vector<float>           c(numOfPart);
    
    // tmp matrices
    vector<int>             oldClasses;
    vector<float>           oldAlignment;
    vector< vector<float> > oldDynamics;
    vector< vector<float> > oldScalings;
    
    setVec(oldClasses,   classes);
    setVec(oldAlignment, alignment);
    setMat(oldDynamics,  dynamics);
    setMat(oldScalings,  scalings);
    
    
    c[0] = 0;
    for(int i = 1; i < numOfPart; i++) c[i] = c[i-1] + posterior[i];
    
    
    float u0 = rnduni()/numOfPart;
    
    int i = 0;
    for (int j = 0; j < numOfPart; j++)
    {
        float uj = u0 + (j + 0.) / numOfPart;
        
        while (uj > c[i] && i < numOfPart - 1){
            i++;
        }
        
        classes[j]   = oldClasses[i];
        alignment[j] = oldAlignment[i];
        
        for (int l=0;l<dynamicsDim;l++)     dynamics[j][l] = oldDynamics[i][l];
        for (int l=0;l<scalingsDim;l++)     scalings[j][l] = oldScalings[i][l];
        
        // update posterior (partilces' weights)
        posterior[j] = 1.0/(float)numOfPart;
    }
    
    
    // defining here the number of particles allocated to reinitialisation
    // used for segmentation
    int free_pool = 0;
    
    // If segmentation
    if (config.segmentation) {
        
        free_pool = round(3*numOfPart/100);
        
        for (int j = 0; j < free_pool; j++)
        {
            int index = round(rnduni()*numOfPart);
            if (index == numOfPart) index = 0;
            
            initPrior(index);
            //initStateValues(index, 0.1);
            //weights[index] = 1.0/(float)(numOfPart * numOfPart);
            
            if (config.translate) for (int jj=0; jj<config.inputDimensions; jj++) offsets[index][jj]=obs[jj];
            
            classes[index] = index % getNumberOfGestureTemplates(); // distribute particles across templates
        }
        float sumw = 0.0;
        for (int j = 0; j < numOfPart; j++) {
            // sumw += weights[j];
            sumw += posterior[j];
        }
        for (int j = 0; j < numOfPart; j++){
            //weights[j] /= sumw;
            posterior[j] /= sumw;
        }
    }
    
}


//--------------------------------------------------------------
void ofxGVF::estimates(){
  
    
    int numOfPart = parameters.numberParticles;
    
    setVec(estimatedAlignment, 0.0f, getNumberOfGestureTemplates());            // rows are gestures
    setMat(estimatedDynamics,  0.0f, getNumberOfGestureTemplates(), dynamicsDim);  // rows are gestures, cols are features + probabilities
    setMat(estimatedScalings,  0.0f, getNumberOfGestureTemplates(), scalingsDim);   // rows are gestures, cols are features + probabilities
    setVec(estimatedLikelihoods, 0.0f, getNumberOfGestureTemplates());            // rows are gestures
    
	// compute the estimated features and likelihoods
	for(int n = 0; n < numOfPart; n++)
    {
        estimatedAlignment[classes[n]] += alignment[n] * posterior[n];
        for(int m = 0; m < dynamicsDim; m++) estimatedDynamics[classes[n]][m] += dynamics[n][m] * posterior[n];
        for(int m = 0; m < scalingsDim; m++) estimatedScalings[classes[n]][m] += scalings[n][m] * posterior[n];
        if (!isnan(posterior[n]))
            estimatedLikelihoods[classes[n]] += posterior[n];

    }
	
    // calculate most probable index during scaling...
    float maxProbability = 0.0f;
    mostProbableIndex = -1;
    
	for(int gi = 0; gi < getNumberOfGestureTemplates(); gi++)
    {
        if(estimatedLikelihoods[gi] > maxProbability){
            maxProbability      = estimatedLikelihoods[gi];
            mostProbableIndex   = gi;
        }
	}
    
    outcomes.estimations.clear();
    
    // most probable gesture index
    outcomes.most_probable = mostProbableIndex;
    
            //cerr << "on passe la et most prob ind est " << mostProbableIndex << endl;
    
    // Fill estimation for each gesture
    for (int gi = 0; gi < gestureTemplates.size(); ++gi) {
        
        ofxGVFEstimation estimation;
        
        estimation.probability = estimatedLikelihoods[gi];
        estimation.alignment   = estimatedAlignment[gi];
        
        estimation.dynamics    = vector<float>(dynamicsDim);
        for (int j = 0; j < dynamicsDim; ++j) estimation.dynamics[j] = estimatedDynamics[gi][j];
        
        estimation.scalings    = vector<float>(scalingsDim);
        for (int j = 0; j < scalingsDim; ++j) estimation.scalings[j] = estimatedScalings[gi][j];
        
        // push estimation for gesture gi in outcomes
        outcomes.estimations.push_back(estimation);
    }

    
    assert(outcomes.estimations.size() == gestureTemplates.size());

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

    outcomes.estimations.clear();
    
    // most probable gesture index
    outcomes.most_probable = mostProbableIndex;

    
    // Fill estimation for each gesture
    for (int gi = 0; gi < gestureTemplates.size(); ++gi) {
        
        ofxGVFEstimation estimation;
        
        estimation.probability = estimatedLikelihoods[gi];
        estimation.alignment   = estimatedAlignment[gi];
        
        estimation.dynamics    = vector<float>(dynamicsDim);
        for (int j = 0; j < dynamicsDim; ++j) estimation.dynamics[j] = estimatedDynamics[gi][j];

        estimation.scalings    = vector<float>(scalingsDim);
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
		gp[classes[n]] += posterior[n];
    
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
    
    if (!config.logOn)
        if (_config.logOn){
            // only for logs
            vecRef = vector<vector<float> > (parameters.numberParticles);
            vecObs = vector<float> (config.inputDimensions);
            stateNoiseDist = vector<float> (parameters.numberParticles);
        }
            
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
        
        // minimum number of particles allowed
        if (parameters.numberParticles < 4) parameters.numberParticles = 4;

        // re-learn
        learn();
        
        // adapt the resampling threshold in case if RT < NS
        if (parameters.numberParticles <= parameters.resamplingThreshold)
            parameters.resamplingThreshold = parameters.numberParticles / 4;
        
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
// Update the number of particles
void ofxGVF::setPredictionLoops(int predictionLoops){
    if (predictionLoops<1)
        parameters.predictionLoops = 1;
    else
        parameters.predictionLoops = predictionLoops;
}

//--------------------------------------------------------------
int ofxGVF::getPredictionLoops(){
    return parameters.predictionLoops; // Return the number of particles
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
void ofxGVF::setAlignmentVariance(float alignmentVariance){
    parameters.alignmentVariance = sqrt(alignmentVariance);
}
//--------------------------------------------------------------
float ofxGVF::getAlignmentVariance(){
    return parameters.alignmentVariance;
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

//--------------------------------------------------------------
float distance_weightedEuclidean(vector<float> x, vector<float> y, vector<float> w){
    int count = x.size();
    if (count <= 0) return 0;
    float dist = 0.0;
    for(int k = 0; k < count; k++) dist += w[k] * pow((x[k] - y[k]), 2);
    return dist;
}


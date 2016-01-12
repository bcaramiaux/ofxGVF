/**
 * Gesture Variation Follower class allows for early gesture recognition and variation tracking
 *
 * @details Original algorithm designed and implemented in 2011 at Ircam Centre Pompidou
 * by Baptiste Caramiaux and Nicola Montecchio. The library has been created and is maintained by Baptiste Caramiaux
 *
 * Copyright (C) 2015 Baptiste Caramiaux, Nicola Montecchio
 * STMS lab Ircam-CRNS-UPMC, University of Padova, Goldsmiths College University of London
 *
 * The library is under the GNU Lesser General Public License (LGPL v3)
 */

#include "ofxGVF.h"

//#include <Accelerate/Accelerate.h>

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory> //tr1/memory
#include <unistd.h>

//debug max
//#include "ext.h"


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
    defaultParameters.dynamicsVariance      = vector<float>(1,sqrt(0.001f));
    defaultParameters.scalingsVariance      = vector<float>(1,sqrt(0.00001f));
    defaultParameters.rotationsVariance     = vector<float>(1,sqrt(0.0f));
    defaultParameters.predictionLoops       = 1;
    defaultParameters.dimWeights            = vector<float>(1,sqrt(1.0f));
    
    // default spreading
    defaultParameters.alignmentSpreadingCenter = 0.0;
    defaultParameters.alignmentSpreadingRange  = 0.2;
    
    defaultParameters.dynamicsSpreadingCenter = 1.0;
    defaultParameters.dynamicsSpreadingRange  = 0.3;
    
    defaultParameters.scalingsSpreadingCenter = 1.0;
    defaultParameters.scalingsSpreadingRange  = 0.3;
    
    defaultParameters.rotationsSpreadingCenter = 0.0;
    defaultParameters.rotationsSpreadingRange  = 0.0;
    
    tolerancesetmanually = false;
    
    setup(_config,  defaultParameters);

}

//--------------------------------------------------------------
void ofxGVF::setup(ofxGVFConfig _config, ofxGVFParameters _parameters){
    
    clear(); // just in case
    
    // Set configuration and parameters
    config      = _config;
    parameters  = _parameters;
    
    // Init random generators
    normgen = std::mt19937(rd());
    rndnorm = new std::normal_distribution<float>(0.0,1.0);
    unifgen = std::default_random_engine(rd());
    rndunif = new std::uniform_real_distribution<float>(0.0,1.0);
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
    activeGestures.push_back(gestureTemplates.size());
    
    train();
    
}

//--------------------------------------------------------------
void ofxGVF::replaceGestureTemplate(ofxGVFGesture & gestureTemplate, int ID){
    
    if (getState() != ofxGVF::STATE_LEARNING)
        setState(ofxGVF::STATE_LEARNING);
    
    if(gestureTemplate.getNumberDimensions()!=config.inputDimensions)
        return;
    
    if(minRange.size() == 0){
        minRange.resize(config.inputDimensions);
        maxRange.resize(config.inputDimensions);
    }
    
    for(int j = 0; j < config.inputDimensions; j++){
        minRange[j] = INFINITY;
        maxRange[j] = -INFINITY;
    }
    
    // compute min/max from the data
    for(int i = 0; i < gestureTemplates.size(); i++){
        ofxGVFGesture& tGestureTemplate = gestureTemplates[i];
        vector<float>& tMinRange = tGestureTemplate.getMinRange();
        vector<float>& tMaxRange = tGestureTemplate.getMaxRange();
        for(int j = 0; j < config.inputDimensions; j++){
            if(tMinRange[j] < minRange[j]) minRange[j] = tMinRange[j];
            if(tMaxRange[j] > maxRange[j]) maxRange[j] = tMaxRange[j];
        }
    }
    
    gestureTemplates[ID]=gestureTemplate;
    
    for(int i = 0; i < gestureTemplates.size(); i++){
        ofxGVFGesture& tGestureTemplate = gestureTemplates[i];
        tGestureTemplate.setMinRange(minRange);
        tGestureTemplate.setMaxRange(maxRange);
    }
    
}

//--------------------------------------------------------------
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
    return (int)gestureTemplates.size();
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
//              LEARNING and INIT FUNCTIONS
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


//----------------------------------------------
//
// Learn Internal Configuration
//
void ofxGVF::train(){
    
    if (gestureTemplates.size() > 0)
    {
        
        // get the number of dimension in templates
        config.inputDimensions = gestureTemplates[0].getTemplateDimension();
        
        dynamicsDim = 2;    // hard coded: just speed now
        scalingsDim = config.inputDimensions;
        
        // manage orientation
        if (config.inputDimensions==2) rotationsDim=1;
        else if (config.inputDimensions==3) rotationsDim=3;
        else rotationsDim=0;
        
        // Init state space
        initVec(classes, parameters.numberParticles);                           // Vector of gesture class
        initVec(alignment, parameters.numberParticles);                         // Vector of phase values (alignment)
        initMat(dynamics, parameters.numberParticles, dynamicsDim);             // Matric of dynamics
        initMat(scalings, parameters.numberParticles, scalingsDim);             // Matrix of scaling
        if (rotationsDim!=0) initMat(rotations, parameters.numberParticles, rotationsDim);             // Matrix of rotations
        initMat(offsets, parameters.numberParticles, config.inputDimensions);
        initVec(weights, parameters.numberParticles);                           // Weights

        initMat(particles, parameters.numberParticles, 3);
//            std::cout << particles.size() << " "  << parameters.numberParticles << std::endl;
        
        // bayesian elements
        initVec(prior, parameters.numberParticles);
        initVec(posterior, parameters.numberParticles);
        initVec(likelihood, parameters.numberParticles);
        
        
        initPrior();            // prior on init state values
        initNoiseParameters();  // init noise parameters (transition and likelihood)
        
        
        // weighted dimensions in case: default is not weighted
        if (parameters.dimWeights.size()!=config.inputDimensions){
            parameters.dimWeights = vector<float> (config.inputDimensions);
            for(int k = 0; k < config.inputDimensions; k++) parameters.dimWeights[k] = 1.0 / config.inputDimensions;
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
void ofxGVF::initPrior()
{
    
    // PATICLE FILTERING
    for (int k = 0; k < parameters.numberParticles; k++)
    {
        initPrior(k);
        
        classes[k] = activeGestures[k % activeGestures.size()] - 1;
    }
    
}

//--------------------------------------------------------------
void ofxGVF::initPrior(int pf_n)
{
    
    // alignment
    alignment[pf_n] = ((*rndunif)(unifgen) - 0.5) * parameters.alignmentSpreadingRange + parameters.alignmentSpreadingCenter;    // spread phase
    

    // dynamics
    dynamics[pf_n][0] = ((*rndunif)(unifgen) - 0.5) * parameters.dynamicsSpreadingRange + parameters.dynamicsSpreadingCenter; // spread speed
    if (dynamics[pf_n].size()>1)
    {
        dynamics[pf_n][1] = ((*rndunif)(unifgen) - 0.5) * parameters.dynamicsSpreadingRange; // spread accel
    }
    
    // scalings
    for(int l = 0; l < scalings[pf_n].size(); l++) {
        scalings[pf_n][l] = ((*rndunif)(unifgen) - 0.5) * parameters.scalingsSpreadingRange + parameters.scalingsSpreadingCenter; // spread scalings
    }
    
    // rotations
    if (rotationsDim!=0)
        for(int l = 0; l < rotations[pf_n].size(); l++)
            rotations[pf_n][l] = ((*rndunif)(unifgen) - 0.5) * parameters.rotationsSpreadingRange + parameters.rotationsSpreadingCenter;    // spread rotations
    
    if (config.translate) for(int l = 0; l < offsets[pf_n].size(); l++) offsets[pf_n][l] = 0.0;
    
    
    prior[pf_n] = 1.0 / (float) parameters.numberParticles;

    // set the posterior to the prior at the initialization
    posterior[pf_n] = prior[pf_n];
    
}

////--------------------------------------------------------------
//void ofxGVF::initPriorV1(int pf_n) {
//    
//    float range = 0.1;
//    
//    classes[pf_n]   = pf_n % gestureTemplates.size();
//    alignment[pf_n] = ((*rndunif)(unifgen) - 0.5) * range;    // spread phase
//    
//    int maxdyn=4.0; int mindyn=-4.0;
//    for(int l = 0; l < dynamics[pf_n].size(); l++) dynamics[pf_n][l] = (maxdyn-mindyn)/(1.0*pf_n)+mindyn; //dynamics between -4 and 4
//
//    int maxscale=4.0; int minscale=0.0;
//    for(int l = 0; l < scalings[pf_n].size(); l++) scalings[pf_n][l] = (maxscale-minscale)/(1.0*pf_n)+minscale; //scalings between 0 and 4
//   
//    if (rotationsDim!=0) {
//        int maxrot=180.0; int minrot=-180.0;
//        for(int l = 0; l < rotations[pf_n].size(); l++) rotations[pf_n][l] = (maxrot-minrot)/(1.0*pf_n)+minrot; // rotations
//    }
//    
//    if (config.translate) for(int l = 0; l < offsets[pf_n].size(); l++) offsets[pf_n][l] = 0.0;
//    
//    
//    prior[pf_n] = 1.0 / (float) parameters.numberParticles;
//    
//    // set the posterior to the prior at the initialization
//    posterior[pf_n] = prior[pf_n];
//    
//}

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
    
    if (rotationsDim!=0)
    {
        if (parameters.rotationsVariance.size() != rotationsDim)
        {
            float variance = parameters.rotationsVariance[0];
            parameters.rotationsVariance.resize(rotationsDim);
            for (int k=0; k<rotationsDim; k++)
                parameters.rotationsVariance[k] = variance;
        }
    }
    
    // ADAPTATION OF THE TOLERANCE IF DEFAULT PARAMTERS
    // ---------------------------
    if (!tolerancesetmanually){
    float obsMeanRange = 0.0f;
    for (int gt=0; gt<gestureTemplates.size(); gt++) {
        for (int d=0; d<config.inputDimensions; d++)
            obsMeanRange += (gestureTemplates[gt].getMaxRange()[d] - gestureTemplates[gt].getMinRange()[d])
            /config.inputDimensions;
    }
    obsMeanRange /= gestureTemplates.size();
    parameters.tolerance = obsMeanRange / 4.0f;  // dividing by an heuristic factor [to be learned?]
    }
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
//            std::cout << "- " << gestureTemplates.size() << std::endl;
            if (gestureTemplates.size() > 0)
                train();
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

//--------------------------------------------------------------
int ofxGVF::getDynamicsDim(){
    return dynamicsDim;
}
//--------------------------------------------------------------
vector<int> ofxGVF::getClasses(){
    return classes;
}
//--------------------------------------------------------------
vector<float> ofxGVF::getAlignment(){
    return alignment;
}
//--------------------------------------------------------------
vector<float> ofxGVF::getEstimatedAlignment(){
    return estimatedAlignment;
}
//--------------------------------------------------------------
vector< vector<float> > ofxGVF::getDynamics(){
    return dynamics;
}
//--------------------------------------------------------------
vector< vector<float> > ofxGVF::getEstimatedDynamics(){
    return estimatedDynamics;
}
//--------------------------------------------------------------
vector< vector<float> > ofxGVF::getScalings(){
    return scalings;
}
//--------------------------------------------------------------
vector< vector<float> > ofxGVF::getEstimatedScalings(){
    return estimatedScalings;
}
//--------------------------------------------------------------
vector< vector<float> > ofxGVF::getRotations(){
    return rotations;
}
//--------------------------------------------------------------
vector< vector<float> > ofxGVF::getEstimatedRotations(){
    return estimatedRotations;
}
//--------------------------------------------------------------
vector<float> ofxGVF::getEstimatedProbabilities(){
    return estimatedProbabilities;
}
//--------------------------------------------------------------
vector<float> ofxGVF::getEstimatedLikelihoods(){
    return estimatedLikelihoods;
}
//--------------------------------------------------------------
vector<float> ofxGVF::getWeights(){
    return weights;
}
//--------------------------------------------------------------
vector<float> ofxGVF::getPrior(){
    return prior;
}
//--------------------------------------------------------------
vector<vector<float> > ofxGVF::getVecRef() {
    return vecRef;
}
//--------------------------------------------------------------
vector<float> ofxGVF::getVecObs() {
    return vecObs;
}
//--------------------------------------------------------------
vector<float> ofxGVF::getStateNoiseDist(){
    return stateNoiseDist;
}
//--------------------------------------------------------------
int ofxGVF::getScalingsDim(){
    return scalingsDim;
}
//--------------------------------------------------------------
int ofxGVF::getRotationsDim(){
    return rotationsDim;
}
//--------------------------------------------------------------
void ofxGVF::restart(){
    initPrior();
}



void ofxGVF::setSpreadDynamics(float center, float range){
    parameters.dynamicsSpreadingCenter = center;
    parameters.dynamicsSpreadingRange = range;
}
void ofxGVF::setSpreadScalings(float center, float range){
    parameters.scalingsSpreadingCenter = center;
    parameters.scalingsSpreadingRange = range;
}
void ofxGVF::setSpreadRotations(float center, float range){
    parameters.rotationsSpreadingCenter = center;
    parameters.rotationsSpreadingRange = range;
}



////////////////////////////////////////////////////////////////
//
//
//  PARTICLE FILTERING
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
//
////////////////////////////////////////////////////////////////




//--------------------------------------------------------------
//  updatePrior(p) * dynamics of the state space, compute prior distribution
//--------------------------------------------------------------
void ofxGVF::updatePrior(int n) {
    
    // Update alignment / dynamics / scalings
    float L = gestureTemplates[classes[n]].getTemplateLength();
    alignment[n] += (*rndnorm)(normgen) * parameters.alignmentVariance + dynamics[n][0]/L; // + dynamics[n][1]/(L*L);
    
    if (dynamics[n].size()>1){
        dynamics[n][0] += (*rndnorm)(normgen) * parameters.dynamicsVariance[0] + dynamics[n][1]/L;
        dynamics[n][1] += (*rndnorm)(normgen) * parameters.dynamicsVariance[1];
    }
    else {
        dynamics[n][0] += (*rndnorm)(normgen) * parameters.dynamicsVariance[0];
    }
    
//    for(int l= 0; l < dynamics[n].size(); l++)  dynamics[n][l] += (*rndnorm)(normgen) * parameters.dynamicsVariance[l];
    for(int l= 0; l < scalings[n].size(); l++)  scalings[n][l] += (*rndnorm)(normgen) * parameters.scalingsVariance[l];
    if (rotationsDim!=0) for(int l= 0; l < rotations[n].size(); l++)  rotations[n][l] += (*rndnorm)(normgen) * parameters.rotationsVariance[l];
    
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
    
    if (config.translate) for (int j=0; j < config.inputDimensions; j++) vobs[j] = vobs[j] - offsets[n][j];
    if (config.normalization) for (int kk=0; kk<vobs.size(); kk++) vobs[kk] = vobs[kk] / globalNormalizationFactor;

    
    // TEMPLATE SAMPLE GIVEN FROM STATE VALUES
    // =======================================
    
    if(alignment[n] < 0.0)      {
        alignment[n] = fabs(alignment[n]);  // re-spread at the beginning
        if (config.segmentation)
            classes[n]   = n % getNumberOfGestureTemplates();
    }
    else if(alignment[n] > 1.0) {
        if (config.segmentation){
            alignment[n] = fabs(1.0-alignment[n]); // re-spread at the beginning
            classes[n]   = n % getNumberOfGestureTemplates();
        }
        else{
            alignment[n] = fabs(2.0-alignment[n]); // re-spread at the end
        }
    }
    
    // take vref from template at the given alignment
    vector<float> vref = getGestureTemplateSample(classes[n], alignment[n]);
    
    // Apply scaling coefficients
    for (int k=0;k < config.inputDimensions; k++)
    {
        if (config.normalization) vref[k] = vref[k] / globalNormalizationFactor;
        vref[k] *= scalings[n][k];
    }

    // Apply rotation coefficients
    if (config.inputDimensions==2) {
        float tmp0=vref[0]; float tmp1=vref[1];
        vref[0] = cos(rotations[n][0])*tmp0 - sin(rotations[n][0])*tmp1;
        vref[1] = sin(rotations[n][0])*tmp0 + cos(rotations[n][0])*tmp1;
    }
    else if (config.inputDimensions==3) {
        // Rotate template sample according to the estimated angles of rotations (3d)
        vector<vector< float> > RotMatrix = return_RotationMatrix_3d(rotations[n][0],rotations[n][1],rotations[n][2]);
        vref = multiplyMat(RotMatrix, vref);
    }
    

    // COMPUTE LIKELIHOOD
    // ==================
    
    // weighted euclidean distance
    float dist = distance_weightedEuclidean(vref,vobs,parameters.dimWeights);
    
    if(parameters.distribution == 0.0f){    // Gaussian distribution
        likelihood[n] = exp(- dist * 1 / (parameters.tolerance * parameters.tolerance));
    }
    else {            // Student's distribution
        likelihood[n] = pow(dist/parameters.distribution + 1, -parameters.distribution/2 - 1);    // dimension is 2 .. pay attention if editing]
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
ofxGVFOutcomes & ofxGVF::update(vector<float> & obs){
    

    if (getState() != ofxGVF::STATE_FOLLOWING)
        setState(ofxGVF::STATE_FOLLOWING);
        
    // // If logging
    // if (config.logOn){
    //     vecRef.clear();
    //     stateNoiseDist.clear();
    // }
    
    
    // INFERENCE
    // =========
    
    // for each particle: perform updates of state space / likelihood / prior (weights)
    float sumw = 0.0;
    for(int n = 0; n< parameters.numberParticles; n++)
    {
        
        for (int m=0; m<parameters.predictionLoops; m++)
        {
            updatePrior(n);
            updateLikelihood(obs, n);
            updatePosterior(n);
        }
    
        sumw += posterior[n];   // sum posterior to normalise the distrib afterwards
        
        particles[n][0] = alignment[n];
        particles[n][1] = scalings[n][0];
        particles[n][2] = classes[n];
    }
    
    
    
    // RESAMPLING if needed
    // ====================
    
    // normalize the weights and compute the resampling criterion
    float dotProdw = 0.0;
    for (int k = 0; k < parameters.numberParticles; k++){
        posterior[k] /= sumw;
        dotProdw   += posterior[k] * posterior[k];
    }
    // avoid degeneracy (no particles active, i.e. weight = 0) by resampling
	if( (1./dotProdw) < parameters.resamplingThreshold)
        resampleAccordingToWeights(obs);
    
    
    
    // ESTIMATES for each Gesture
    // ==========================
    
    estimates();
    
    return outcomes;
    
}

// Old way with infer [DEPRECATED]
void ofxGVF::infer(vector<float> obs) {
    update(obs);
}



//--------------------------------------------------------------
//  resampleAccordingToWeights(obs)
//--------------------------------------------------------------
void ofxGVF::resampleAccordingToWeights(vector<float> obs)
{
    
    // random generator: uniform distribution
    // std::tr1::uniform_real<float> ur(0,1);
    // std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
    
    // covennient
    int numOfPart = parameters.numberParticles;
    
    // cumulative dist
    vector<float>           c(numOfPart);
    
    // tmp matrices
    vector<int>             oldClasses;
    vector<float>           oldAlignment;
    vector< vector<float> > oldDynamics;
    vector< vector<float> > oldScalings;
    vector< vector<float> > oldRotations;
    
    setVec(oldClasses,   classes);
    setVec(oldAlignment, alignment);
    setMat(oldDynamics,  dynamics);
    setMat(oldScalings,  scalings);
    if (rotationsDim!=0) setMat(oldRotations, rotations);
    
    
    c[0] = 0;
    for(int i = 1; i < numOfPart; i++) c[i] = c[i-1] + posterior[i];
    
    
    float u0 = (*rndunif)(unifgen)/numOfPart;
    
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
        if (rotationsDim!=0) for (int l=0;l<rotationsDim;l++) rotations[j][l] = oldRotations[i][l];
        
        // update posterior (partilces' weights)
        posterior[j] = 1.0/(float)numOfPart;
    }
    
}


//--------------------------------------------------------------
void ofxGVF::estimates(){
  
    
    int numOfPart = parameters.numberParticles;
    vector<float> probabilityNormalisation(getNumberOfGestureTemplates());
    setVec(probabilityNormalisation, 0.0f, getNumberOfGestureTemplates());            // rows are gestures
    setVec(estimatedAlignment, 0.0f, getNumberOfGestureTemplates());            // rows are gestures
    setMat(estimatedDynamics,  0.0f, getNumberOfGestureTemplates(), dynamicsDim);  // rows are gestures, cols are features + probabilities
    setMat(estimatedScalings,  0.0f, getNumberOfGestureTemplates(), scalingsDim);   // rows are gestures, cols are features + probabilities
    if (rotationsDim!=0) setMat(estimatedRotations,  0.0f, getNumberOfGestureTemplates(), rotationsDim);   // ..
    setVec(estimatedProbabilities, 0.0f, getNumberOfGestureTemplates());            // rows are gestures
    setVec(estimatedLikelihoods, 0.0f, getNumberOfGestureTemplates());            // rows are gestures
    
//    float sumposterior = 0.;
    
    for(int n = 0; n < numOfPart; n++)
    {
        probabilityNormalisation[classes[n]] += posterior[n];
    }
    
    
	// compute the estimated features and likelihoods
	for(int n = 0; n < numOfPart; n++)
    {

//        sumposterior += posterior[n];
        estimatedAlignment[classes[n]] += alignment[n] * posterior[n];
        
        for(int m = 0; m < dynamicsDim; m++)
            estimatedDynamics[classes[n]][m] += dynamics[n][m] * (posterior[n]/probabilityNormalisation[classes[n]]);
        
        for(int m = 0; m < scalingsDim; m++)
            estimatedScalings[classes[n]][m] += scalings[n][m] * (posterior[n]/probabilityNormalisation[classes[n]]);
        
        if (rotationsDim!=0)
            for(int m = 0; m < rotationsDim; m++)
                estimatedRotations[classes[n]][m] += rotations[n][m] * (posterior[n]/probabilityNormalisation[classes[n]]);
        
        if (!isnan(posterior[n]))
            estimatedProbabilities[classes[n]] += posterior[n];
        estimatedLikelihoods[classes[n]] += likelihood[n];

//        post("dynamics[n][0] * posterior[n] = %f * %f", dynamics[n][0], posterior[n]);
    }
//    post(" estimated scal = %f", estimatedScalings[1][0]);
//    post(" sum posterior = %f", sumposterior);

    
//    for(int n = 0; n < estimatedScalings.size(); n++){
//        for(int m = 0; m < estimatedScalings[0].size(); m++){
//            post("est scal [%i][%i] = %f",n,m,estimatedScalings[classes[n]][m]);
//        }
//    }
    
	
    // calculate most probable index during scaling...
    float maxProbability = 0.0f;
    mostProbableIndex = -1;
    
	for(int gi = 0; gi < getNumberOfGestureTemplates(); gi++)
    {
        if(estimatedProbabilities[gi] > maxProbability){
            maxProbability      = estimatedProbabilities[gi];
            mostProbableIndex   = gi;
        }
	}
    
    outcomes.estimations.clear();
    
    // most probable gesture index
    outcomes.most_probable = mostProbableIndex;
    
    // Fill estimation for each gesture
    for (int gi = 0; gi < gestureTemplates.size(); ++gi) {
        
        ofxGVFEstimation estimation;
        
        estimation.probability = estimatedProbabilities[gi];
        estimation.alignment   = estimatedAlignment[gi];
        
        estimation.dynamics    = vector<float>(dynamicsDim);
        for (int j = 0; j < dynamicsDim; ++j) estimation.dynamics[j] = estimatedDynamics[gi][j];
        
        estimation.scalings    = vector<float>(scalingsDim);
        for (int j = 0; j < scalingsDim; ++j) estimation.scalings[j] = estimatedScalings[gi][j];
        
        estimation.rotations   = vector<float>();
        if (rotationsDim!=0)
        {
            estimation.rotations.resize(rotationsDim);
            for (int j = 0; j < rotationsDim; ++j) estimation.rotations[j] = estimatedRotations[gi][j];
        }
            
        estimation.likelihood = estimatedLikelihoods[gi];
        
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
vector<float> & ofxGVF::getGestureProbabilities()
{
    //	unsigned int ngestures = numTemplates+1;
    
//	vector<float> gp(getNumberOfGestureTemplates());
    gestureProbabilities.resize(getNumberOfGestureTemplates());
    setVec(gestureProbabilities, 0.0f);
	for(int n = 0; n < parameters.numberParticles; n++)
		gestureProbabilities[classes[n]] += posterior[n];
    
	return gestureProbabilities;
}

//--------------------------------------------------------------
const vector<vector<float> > & ofxGVF::getParticlesPositions(){
    return particles;
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
        train();
        
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
    
    train();
    
    if (parameters.numberParticles <= parameters.resamplingThreshold) {
        parameters.resamplingThreshold = parameters.numberParticles / 4;
    }

}

//--------------------------------------------------------------
int ofxGVF::getNumberOfParticles(){
    return parameters.numberParticles; // Return the number of particles
}

//--------------------------------------------------------------
void ofxGVF::setActiveGestures(vector<int> activeGestureIds)
{
    int argmax = *std::max_element(activeGestureIds.begin(), activeGestureIds.end());
    if (activeGestureIds[argmax] <= gestureTemplates.size())
    {
        activeGestures = activeGestureIds;
    }
    else
    {
        activeGestures.resize(gestureTemplates.size());
        std::iota(activeGestures.begin(), activeGestures.end(), 1);
    }
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
    tolerancesetmanually = true;
}

//--------------------------------------------------------------
float ofxGVF::getTolerance(){
    return parameters.tolerance;
}

//--------------------------------------------------------------
void ofxGVF::setDistribution(float _distribution){
    //nu = _distribution;
    parameters.distribution = _distribution;
}

//--------------------------------------------------------------
float ofxGVF::getDistribution(){
    return parameters.distribution;
}

//void ofxGVF::setDimWeights(vector<float> dimWeights){
//    if (dimWeights.size()!=parameters.dimWeights.size())
//        parameters.dimWeights.resize(dimWeights.size());
//    parameters.dimWeights = dimWeights;
//}
//
//vector<float> ofxGVF::getDimWeights(){
//    return parameters.dimWeights;
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
void ofxGVF::setDynamicsVariance(float dynVariance){
    for (int k=0; k< parameters.dynamicsVariance.size(); k++)
        parameters.dynamicsVariance[k] = dynVariance;
}
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
void ofxGVF::setScalingsVariance(float scaleVariance)
{
    for (int k=0; k< parameters.scalingsVariance.size(); k++)
        parameters.scalingsVariance[k] = scaleVariance;
}
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

// VARIANCE COEFFICIENTS: ROTATIONS
//--------------------------------------------------------------
void ofxGVF::setRotationsVariance(float rotationVariance, int dim){
    // here dim should start at 1!!!
    if (dim<parameters.rotationsVariance.size())
        parameters.scalingsVariance[dim-1] = rotationVariance;
}
//--------------------------------------------------------------
void ofxGVF::setRotationsVariance(vector<float> rotationVariance){
    parameters.scaleVariance = rotationVariance;
}
//--------------------------------------------------------------
vector<float> ofxGVF::getRotationsVariance(){
    return parameters.rotationsVariance;
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




void ofxGVF::testRndNum(){


    std::map<int, int> hist;
    cout << "===== UNIFORM" << endl;
    for(int n=0; n<10000; ++n) {
        ++hist[std::round((*rndunif)(unifgen)*5.0)];
        cout << (*rndunif)(unifgen) << endl;
    }
    for(auto p : hist) {
        std::cout << std::fixed << std::setprecision(1) << std::setw(2)
                  << p.first << ' ' << std::string(p.second/200, '*') << '\n';
    }
    cout << "===== NORMAL" << endl;
    hist.clear();
    for(int n=0; n<10000; ++n) {
        ++hist[std::round((*rndnorm)(normgen)*5.0)];
    }
    for(auto p : hist) {
        std::cout << std::fixed << std::setprecision(1) << std::setw(2)
                  << p.first << ' ' << std::string(p.second/200, '*') << '\n';
    }
    
}









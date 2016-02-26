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

#include "GVF.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <numeric>

//debug max
//#include "ext.h"


using namespace std;

//--------------------------------------------------------------
GVF::GVF()
{
    config.inputDimensions   = 2;
    config.translate         = true;
    config.segmentation      = false;
    
    parameters.numberParticles       = 1000;
    parameters.tolerance             = 0.2f;
    parameters.resamplingThreshold   = 250;
    parameters.distribution          = 0.0f;
    parameters.alignmentVariance     = sqrt(0.000001f);
    parameters.dynamicsVariance      = vector<float>(1,sqrt(0.001f));
    parameters.scalingsVariance      = vector<float>(1,sqrt(0.00001f));
    parameters.rotationsVariance     = vector<float>(1,sqrt(0.0f));
    parameters.predictionSteps       = 1;
    parameters.dimWeights            = vector<float>(1,sqrt(1.0f));
    parameters.alignmentSpreadingCenter     = 0.0;
    parameters.alignmentSpreadingRange      = 0.2;
    parameters.dynamicsSpreadingCenter      = 1.0;
    parameters.dynamicsSpreadingRange       = 0.3;
    parameters.scalingsSpreadingCenter      = 1.0;
    parameters.scalingsSpreadingRange       = 0.3;
    parameters.rotationsSpreadingCenter     = 0.0;
    parameters.rotationsSpreadingRange      = 0.5;
    
    tolerancesetmanually = false;
    learningGesture = -1;
    
    normgen = std::mt19937(rd());
    rndnorm = new std::normal_distribution<float>(0.0,1.0);
    unifgen = std::default_random_engine(rd());
    rndunif = new std::uniform_real_distribution<float>(0.0,1.0);
    
}

////--------------------------------------------------------------
//GVF::GVF(GVFConfig _config){
//    setup(_config);
//}
//
////--------------------------------------------------------------
//GVF::GVF(GVFConfig _config, GVFParameters _parameters){
//    setup(_config, _parameters);
//}
//
////--------------------------------------------------------------
//void GVF::setup(){
//    
//    // use defualt parameters
//    GVFConfig defaultConfig;
//    
//    defaultConfig.inputDimensions   = 2;
//    defaultConfig.translate         = true;
//    defaultConfig.segmentation      = false;
//    
//    setup(defaultConfig);
//}
//
////--------------------------------------------------------------
//void GVF::setup(GVFConfig _config){
//    
//    clear(); // just in case
//    
//    learningGesture = -1;
//    
//    // Set configuration:
//    config      = _config;
//    
//    // default parameters
//    GVFParameters defaultParameters;
//    defaultParameters.numberParticles       = 1000;
//    defaultParameters.tolerance             = 0.2f;
//    defaultParameters.resamplingThreshold   = 250;
//    defaultParameters.distribution          = 0.0f;
//    defaultParameters.alignmentVariance     = sqrt(0.000001f);
//    defaultParameters.dynamicsVariance      = vector<float>(1,sqrt(0.001f));
//    defaultParameters.scalingsVariance      = vector<float>(1,sqrt(0.00001f));
//    defaultParameters.rotationsVariance     = vector<float>(1,sqrt(0.0f));
//    defaultParameters.predictionSteps       = 1;
//    defaultParameters.dimWeights            = vector<float>(1,sqrt(1.0f));
//    
//    // default spreading
//    defaultParameters.alignmentSpreadingCenter = 0.0;
//    defaultParameters.alignmentSpreadingRange  = 0.2;
//    
//    defaultParameters.dynamicsSpreadingCenter = 1.0;
//    defaultParameters.dynamicsSpreadingRange  = 0.3;
//    
//    defaultParameters.scalingsSpreadingCenter = 1.0;
//    defaultParameters.scalingsSpreadingRange  = 0.3;
//    
//    defaultParameters.rotationsSpreadingCenter = 0.0;
//    defaultParameters.rotationsSpreadingRange  = 0.0;
//    
//    tolerancesetmanually = false;
//    
//    setup(_config,  defaultParameters);
//    
//}
//
////--------------------------------------------------------------
//void GVF::setup(GVFConfig _config, GVFParameters _parameters)
//{
//    clear(); // just in case
//    // Set configuration and parameters
//    config      = _config;
//    parameters  = _parameters;
//    // Init random generators
//    normgen = std::mt19937(rd());
//    rndnorm = new std::normal_distribution<float>(0.0,1.0);
//    unifgen = std::default_random_engine(rd());
//    rndunif = new std::uniform_real_distribution<float>(0.0,1.0);
//}

//--------------------------------------------------------------
GVF::~GVF()
{
    if (rndnorm != NULL)
        delete (rndnorm);
    clear(); // not really necessary but it's polite ;)
}

//--------------------------------------------------------------
void GVF::clear()
{
    state = STATE_CLEAR;
    gestureTemplates.clear();
    mostProbableIndex = -1;
}

//--------------------------------------------------------------
void GVF::startGesture()
{
    if (state==STATE_FOLLOWING)
    {
        restart();
    }
    else if (state==STATE_LEARNING)
    {
        if (theGesture.getNumberOfTemplates()>0)
        {
            if (theGesture.getTemplateLength()>0)
                addGestureTemplate(theGesture);
        }
        theGesture.clear();
    }
}

//--------------------------------------------------------------
void GVF::addObservation(vector<float> data)
{
    theGesture.addObservation(data);
}

//--------------------------------------------------------------
void GVF::addGestureTemplate(GVFGesture & gestureTemplate)
{
    
    //    if (getState() != GVF::STATE_LEARNING)
    //        setState(GVF::STATE_LEARNING);
    
    int inputDimension = gestureTemplate.getNumberDimensions();
    config.inputDimensions = inputDimension;
    
    gestureTemplates.push_back(gestureTemplate);
    activeGestures.push_back(gestureTemplates.size());
    
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
        GVFGesture& tGestureTemplate = gestureTemplates[i];
        vector<float>& tMinRange = tGestureTemplate.getMinRange();
        vector<float>& tMaxRange = tGestureTemplate.getMaxRange();
        for(int j = 0; j < inputDimension; j++){
            if(tMinRange[j] < minRange[j]) minRange[j] = tMinRange[j];
            if(tMaxRange[j] > maxRange[j]) maxRange[j] = tMaxRange[j];
        }
    }
    
    for(int i = 0; i < gestureTemplates.size(); i++){
        GVFGesture& tGestureTemplate = gestureTemplates[i];
        tGestureTemplate.setMinRange(minRange);
        tGestureTemplate.setMaxRange(maxRange);
    }
    train();
    
}

//--------------------------------------------------------------
void GVF::replaceGestureTemplate(GVFGesture & gestureTemplate, int index)
{
    if(gestureTemplate.getNumberDimensions()!=config.inputDimensions)
        return;
    if(minRange.size() == 0)
    {
        minRange.resize(config.inputDimensions);
        maxRange.resize(config.inputDimensions);
    }
    for(int j = 0; j < config.inputDimensions; j++)
    {
        minRange[j] = INFINITY;
        maxRange[j] = -INFINITY;
    }
    if (index<=gestureTemplates.size())
        gestureTemplates[index-1]=gestureTemplate;
    for(int i = 0; i < gestureTemplates.size(); i++)
    {
        GVFGesture& tGestureTemplate = gestureTemplates[i];
        vector<float>& tMinRange = tGestureTemplate.getMinRange();
        vector<float>& tMaxRange = tGestureTemplate.getMaxRange();
        for(int j = 0; j < config.inputDimensions; j++){
            if(tMinRange[j] < minRange[j]) minRange[j] = tMinRange[j];
            if(tMaxRange[j] > maxRange[j]) maxRange[j] = tMaxRange[j];
        }
    }
    for(int i = 0; i < gestureTemplates.size(); i++)
    {
        GVFGesture& tGestureTemplate = gestureTemplates[i];
        tGestureTemplate.setMinRange(minRange);
        tGestureTemplate.setMaxRange(maxRange);
    }
}

////--------------------------------------------------------------
//vector<float>& GVF::getGestureTemplateSample(int gestureIndex, float cursor)
//{
//    int frameindex = min((int)(gestureTemplates[gestureIndex].getTemplateLength() - 1),
//                         (int)(floor(cursor * gestureTemplates[gestureIndex].getTemplateLength() ) ) );
//    return gestureTemplates[gestureIndex].getTemplate()[frameindex];
//}

//--------------------------------------------------------------
GVFGesture & GVF::getGestureTemplate(int index){
    assert(index < gestureTemplates.size());
    return gestureTemplates[index];
}

//--------------------------------------------------------------
vector<GVFGesture> & GVF::getAllGestureTemplates(){
    return gestureTemplates;
}

//--------------------------------------------------------------
int GVF::getNumberOfGestureTemplates(){
    return (int)gestureTemplates.size();
}

//--------------------------------------------------------------
void GVF::removeGestureTemplate(int index){
    assert(index < gestureTemplates.size());
    gestureTemplates.erase(gestureTemplates.begin() + index);
}

//--------------------------------------------------------------
void GVF::removeAllGestureTemplates(){
    gestureTemplates.clear();
}

//----------------------------------------------
void GVF::train(){
    
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
//        if (config.normalization) {     // update the global normaliation factor
//            globalNormalizationFactor = -1.0;
//            // loop on previous gestures already learned
//            // take the max of all the gesture learned ...
//            for (int k=0; k<getNumberOfGestureTemplates() ; k++){
//                for(int j = 0; j < config.inputDimensions; j++){
//                    float rangetmp = fabs(getGestureTemplate(k).getMaxRange()[j]-getGestureTemplate(k).getMinRange()[j]);
//                    if (rangetmp > globalNormalizationFactor)
//                        globalNormalizationFactor=rangetmp;
//                }
//            }
//        }
//        // only for logs
//        if (config.logOn) {
//            vecRef = vector<vector<float> > (parameters.numberParticles);
//            vecObs = vector<float> (config.inputDimensions);
//            stateNoiseDist = vector<float> (parameters.numberParticles);
//        }
    }
}

//--------------------------------------------------------------
//void GVF::initPrior()
//{
//    
//    // PATICLE FILTERING
//    for (int k = 0; k < parameters.numberParticles; k++)
//    {
//        initPrior(k);
//        
//        classes[k] = activeGestures[k % activeGestures.size()] - 1;
//    }
//    
//}

//--------------------------------------------------------------
void GVF::initPrior() //int pf_n)
{
    for (int pf_n = 0; pf_n < parameters.numberParticles; pf_n++)
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
        
                classes[pf_n] = activeGestures[pf_n % activeGestures.size()] - 1;
    }
    
}

//--------------------------------------------------------------
void GVF::initNoiseParameters() {
    
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

//--------------------------------------------------------------
void GVF::setState(GVFState _state, vector<int> indexes)
{
    switch (_state)
    {
        case STATE_CLEAR:
            clear();
            theGesture.clear();
            break;
            
        case STATE_LEARNING:
            if ((state==STATE_LEARNING) && (theGesture.getNumberOfTemplates()>0))
            {
                if (learningGesture==-1)
                    addGestureTemplate(theGesture);
                else
                {
                    replaceGestureTemplate(theGesture, learningGesture);
                    learningGesture=-1;
                }
                if (indexes.size()!=0)
                    learningGesture=indexes[0];
            }
            state = _state;
            theGesture.clear();
            break;
            
        case STATE_FOLLOWING:
            if ((state==STATE_LEARNING) && (theGesture.getNumberOfTemplates()>0))
            {
                if (learningGesture==-1)
                    addGestureTemplate(theGesture);
                else
                {
                    replaceGestureTemplate(theGesture, learningGesture);
                    learningGesture=-1;
                }
            }
            if (gestureTemplates.size() > 0)
            {
                train();
                state = _state;
            }
            else
                state = STATE_CLEAR;
            theGesture.clear();
            break;
            
        default:
            theGesture.clear();
            break;
    }
}

//--------------------------------------------------------------
GVF::GVFState GVF::getState()
{
    return state;
}

////--------------------------------------------------------------
//int GVF::getDynamicsDimension(){
//    return dynamicsDim;
//}

//--------------------------------------------------------------
vector<int> GVF::getGestureClasses()
{
    return classes;
}

////--------------------------------------------------------------
//vector<float> GVF::getAlignment(){
//    return alignment;
//}
//
////--------------------------------------------------------------
//vector<float> GVF::getEstimatedAlignment(){
//    return estimatedAlignment;
//}
//
////--------------------------------------------------------------
//vector< vector<float> > GVF::getDynamics(){
//    return dynamics;
//}
//
////--------------------------------------------------------------
//vector< vector<float> > GVF::getEstimatedDynamics(){
//    return estimatedDynamics;
//}
//
////--------------------------------------------------------------
//vector< vector<float> > GVF::getScalings(){
//    return scalings;
//}
//
////--------------------------------------------------------------
//vector< vector<float> > GVF::getEstimatedScalings(){
//    return estimatedScalings;
//}
//
////--------------------------------------------------------------
//vector< vector<float> > GVF::getRotations(){
//    return rotations;
//}
//
////--------------------------------------------------------------
//vector< vector<float> > GVF::getEstimatedRotations(){
//    return estimatedRotations;
//}

////--------------------------------------------------------------
//vector<float> GVF::getEstimatedProbabilities(){
//    return estimatedProbabilities;
//}
//
////--------------------------------------------------------------
//vector<float> GVF::getEstimatedLikelihoods(){
//    return estimatedLikelihoods;
//}
//
////--------------------------------------------------------------
//vector<float> GVF::getWeights(){
//    return weights;
//}
//
////--------------------------------------------------------------
//vector<float> GVF::getPrior(){
//    return prior;
//}

////--------------------------------------------------------------
//vector<vector<float> > GVF::getVecRef() {
//    return vecRef;
//}
//
////--------------------------------------------------------------
//vector<float> GVF::getVecObs() {
//    return vecObs;
//}
//
////--------------------------------------------------------------
//vector<float> GVF::getStateNoiseDist(){
//    return stateNoiseDist;
//}

////--------------------------------------------------------------
//int GVF::getScalingsDim(){
//    return scalingsDim;
//}
//
////--------------------------------------------------------------
//int GVF::getRotationsDim(){
//    return rotationsDim;
//}

//--------------------------------------------------------------
void GVF::restart()
{
    theGesture.clear();
    initPrior();
}

#pragma mark - PARTICLE FILTERING

//--------------------------------------------------------------
void GVF::updatePrior(int n) {
    
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
void GVF::updateLikelihood(vector<float> obs, int n)
{

//    if (config.normalization) for (int kk=0; kk<vobs.size(); kk++) vobs[kk] = vobs[kk] / globalNormalizationFactor;
    
    if(alignment[n] < 0.0)
    {
        alignment[n] = fabs(alignment[n]);  // re-spread at the beginning
//        if (config.segmentation)
//            classes[n]   = n % getNumberOfGestureTemplates();
    }
    else if(alignment[n] > 1.0)
    {
        if (config.segmentation)
        {
//            alignment[n] = fabs(1.0-alignment[n]); // re-spread at the beginning
            alignment[n] = fabs((*rndunif)(unifgen) * 0.5);    //
            classes[n]   = n % getNumberOfGestureTemplates();
            offsets[n]   = obs;
            // dynamics
            dynamics[n][0] = ((*rndunif)(unifgen) - 0.5) * parameters.dynamicsSpreadingRange + parameters.dynamicsSpreadingCenter; // spread speed
            if (dynamics[n].size()>1)
                dynamics[n][1] = ((*rndunif)(unifgen) - 0.5) * parameters.dynamicsSpreadingRange;
            // scalings
            for(int l = 0; l < scalings[n].size(); l++)
                scalings[n][l] = ((*rndunif)(unifgen) - 0.5) * parameters.scalingsSpreadingRange + parameters.scalingsSpreadingCenter; // spread scalings
            // rotations
            if (rotationsDim!=0)
                for(int l = 0; l < rotations[n].size(); l++)
                    rotations[n][l] = ((*rndunif)(unifgen) - 0.5) * parameters.rotationsSpreadingRange + parameters.rotationsSpreadingCenter;    // spread rotations
            // prior
            prior[n] = 1/(float)parameters.numberParticles;
        }
        else{
            alignment[n] = fabs(2.0-alignment[n]); // re-spread at the end
        }
    }
    
    vector<float> vobs(config.inputDimensions);
    setVec(vobs, obs);
    
    if (config.translate)
        for (int j=0; j < config.inputDimensions; j++)
            vobs[j] = vobs[j] - offsets[n][j];
    
    
    // take vref from template at the given alignment
    int gestureIndex = classes[n];
    float cursor = alignment[n];
    int frameindex = min((int)(gestureTemplates[gestureIndex].getTemplateLength() - 1),
                         (int)(floor(cursor * gestureTemplates[gestureIndex].getTemplateLength() ) ) );
//    return gestureTemplates[gestureIndex].getTemplate()[frameindex];
    vector<float> vref = gestureTemplates[gestureIndex].getTemplate()[frameindex];; //getGestureTemplateSample(classes[n], alignment[n]);
    
    // Apply scaling coefficients
    for (int k=0;k < config.inputDimensions; k++)
    {
//        if (config.normalization) vref[k] = vref[k] / globalNormalizationFactor;
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
        vector<vector< float> > RotMatrix = getRotationMatrix3d(rotations[n][0],rotations[n][1],rotations[n][2]);
        vref = multiplyMat(RotMatrix, vref);
    }
    
    // weighted euclidean distance
    float dist = distance_weightedEuclidean(vref,vobs,parameters.dimWeights);
    
    if(parameters.distribution == 0.0f){    // Gaussian distribution
        likelihood[n] = exp(- dist * 1 / (parameters.tolerance * parameters.tolerance));
    }
    else {            // Student's distribution
        likelihood[n] = pow(dist/parameters.distribution + 1, -parameters.distribution/2 - 1);    // dimension is 2 .. pay attention if editing]
    }
//    // if log on keep track on vref and vobs
//    if (config.logOn){
//        vecRef.push_back(vref);
//        vecObs = vobs;
//    }
}

//--------------------------------------------------------------
void GVF::updatePosterior(int n) {
    posterior[n]  = prior[n] * likelihood[n];
}

//--------------------------------------------------------------
GVFOutcomes & GVF::update(vector<float> & observation)
{
    
    if (state != GVF::STATE_FOLLOWING) setState(GVF::STATE_FOLLOWING);
    
    theGesture.addObservation(observation);
    vector<float> obs = theGesture.getLastObservation();
    
    //    std::cout << obs[0] << " " << obs[0] << " "
    //                << gestureTemplates[0].getTemplate()[20][0] << " " << gestureTemplates[0].getTemplate()[20][1] << " "
    //                << gestureTemplates[1].getTemplate()[20][0] << " " << gestureTemplates[1].getTemplate()[20][1] << std::endl;
    
    
    // for each particle: perform updates of state space / likelihood / prior (weights)
    float sumw = 0.0;
    for(int n = 0; n< parameters.numberParticles; n++)
    {
        
        for (int m=0; m<parameters.predictionSteps; m++)
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
    
    // normalize the weights and compute the resampling criterion
    float dotProdw = 0.0;
    for (int k = 0; k < parameters.numberParticles; k++){
        posterior[k] /= sumw;
        dotProdw   += posterior[k] * posterior[k];
    }
    // avoid degeneracy (no particles active, i.e. weight = 0) by resampling
    if( (1./dotProdw) < parameters.resamplingThreshold)
        resampleAccordingToWeights(obs);
    
    // estimate outcomes
    estimates();
    
    return outcomes;
    
}

//--------------------------------------------------------------
void GVF::resampleAccordingToWeights(vector<float> obs)
{
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
void GVF::estimates(){
    
    
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
    }
    
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
    //    std::cout << estimatedProbabilities[0] << " " << estimatedProbabilities[1] << std::endl;
    
    //    outcomes.estimations.clear();
    outcomes.likelihoods.clear();
    outcomes.alignments.clear();
    outcomes.scalings.clear();
    outcomes.dynamics.clear();
    outcomes.rotations.clear();
    
    // most probable gesture index
    outcomes.likeliestGesture = mostProbableIndex;
    
    // Fill estimation for each gesture
    for (int gi = 0; gi < gestureTemplates.size(); ++gi) {
        
        //        GVFEstimation estimation;
        outcomes.likelihoods.push_back(estimatedProbabilities[gi]);
        outcomes.alignments.push_back(estimatedAlignment[gi]);
        //        estimation.probability = estimatedProbabilities[gi];
        //        estimation.alignment   = estimatedAlignment[gi];
        
        
        vector<float> gDynamics(dynamicsDim, 0.0);
        for (int j = 0; j < dynamicsDim; ++j) gDynamics[j] = estimatedDynamics[gi][j];
        outcomes.dynamics.push_back(gDynamics);
        
        vector<float> gScalings(scalingsDim, 0.0);
        for (int j = 0; j < scalingsDim; ++j) gScalings[j] = estimatedScalings[gi][j];
        outcomes.scalings.push_back(gScalings);
        
        vector<float> gRotations;
        if (rotationsDim!=0)
        {
            gRotations.resize(rotationsDim);
            for (int j = 0; j < rotationsDim; ++j) gRotations[j] = estimatedRotations[gi][j];
            outcomes.rotations.push_back(gRotations);
        }
        
        //        estimation.likelihood = estimatedLikelihoods[gi];
        
        // push estimation for gesture gi in outcomes
        //        outcomes.estimations.push_back(estimation);
    }
    
    
    //    assert(outcomes.estimations.size() == gestureTemplates.size());
    
}

////--------------------------------------------------------------
//int GVF::getMostProbableGestureIndex()
//{
//    return mostProbableIndex;
//}

////--------------------------------------------------------------
//GVFOutcomes GVF::getOutcomes()
//{
//    return outcomes;
//}

////--------------------------------------------------------------
//GVFEstimation GVF::getTemplateRecogInfo(int templateNumber)
//{
//    if (getOutcomes().estimations.size() <= templateNumber) {
//        GVFEstimation estimation;
//        return estimation; // blank
//    }
//    else
//        return getOutcomes().estimations[templateNumber];
//}
//
////--------------------------------------------------------------
//GVFEstimation GVF::getRecogInfoOfMostProbable() // FIXME: Rename!
//{
//    int indexMostProbable = getMostProbableGestureIndex();
//
//    if ((getState() == GVF::STATE_FOLLOWING) && (getMostProbableGestureIndex() != -1)) {
//        return getTemplateRecogInfo(indexMostProbable);
//    }
//    else {
//        GVFEstimation estimation;
//        return estimation; // blank
//    }
//}


////--------------------------------------------------------------
//vector<float> & GVF::getGestureProbabilities()
//{
//    gestureProbabilities.resize(getNumberOfGestureTemplates());
//    setVec(gestureProbabilities, 0.0f);
//    for(int n = 0; n < parameters.numberParticles; n++)
//        gestureProbabilities[classes[n]] += posterior[n];
//    
//    return gestureProbabilities;
//}

//--------------------------------------------------------------
const vector<vector<float> > & GVF::getParticlesPositions(){
    return particles;
}

////--------------------------------------------------------------
//void GVF::setParameters(GVFParameters _parameters){
//    
//    // if the number of particles has changed, we have to re-allocate matrices
//    if (_parameters.numberParticles != parameters.numberParticles)
//    {
//        parameters = _parameters;
//        
//        // minimum number of particles allowed
//        if (parameters.numberParticles < 4) parameters.numberParticles = 4;
//        
//        // re-learn
//        train();
//        
//        // adapt the resampling threshold in case if RT < NS
//        if (parameters.numberParticles <= parameters.resamplingThreshold)
//            parameters.resamplingThreshold = parameters.numberParticles / 4;
//        
//    }
//    else
//        parameters = _parameters;
//    
//    
//}
//
//GVFParameters GVF::getParameters(){
//    return parameters;
//}

//--------------------------------------------------------------
// Update the number of particles
void GVF::setNumberOfParticles(int numberOfParticles){
    
    parameters.numberParticles = numberOfParticles;
    
    if (parameters.numberParticles < 4)     // minimum number of particles allowed
        parameters.numberParticles = 4;
    
    train();
    
    if (parameters.numberParticles <= parameters.resamplingThreshold) {
        parameters.resamplingThreshold = parameters.numberParticles / 4;
    }
    
}

//--------------------------------------------------------------
int GVF::getNumberOfParticles(){
    return parameters.numberParticles; // Return the number of particles
}

//--------------------------------------------------------------
void GVF::setActiveGestures(vector<int> activeGestureIds)
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
void GVF::setPredictionSteps(int predictionSteps)
{
    if (predictionSteps<1)
        parameters.predictionSteps = 1;
    else
        parameters.predictionSteps = predictionSteps;
}

//--------------------------------------------------------------
int GVF::getPredictionSteps()
{
    return parameters.predictionSteps; // Return the number of particles
}

//--------------------------------------------------------------
// Update the resampling threshold used to avoid degeneracy problem
void GVF::setResamplingThreshold(int _resamplingThreshold){
    if (_resamplingThreshold >= parameters.numberParticles)
        _resamplingThreshold = floor(parameters.numberParticles/2.0f);
    parameters.resamplingThreshold = _resamplingThreshold;
}

//--------------------------------------------------------------
// Return the resampling threshold used to avoid degeneracy problem
int GVF::getResamplingThreshold(){
    return parameters.resamplingThreshold;
}

//--------------------------------------------------------------
// Update the standard deviation of the observation distribution
// this value acts as a tolerance for the algorithm
// low value: less tolerant so more precise but can diverge
// high value: more tolerant so less precise but converge more easily
void GVF::setTolerance(float _tolerance){
    if (_tolerance <= 0.0) _tolerance = 0.1;
    parameters.tolerance = _tolerance;
    tolerancesetmanually = true;
}

//--------------------------------------------------------------
float GVF::getTolerance(){
    return parameters.tolerance;
}

////--------------------------------------------------------------
void GVF::setDistribution(float _distribution){
    //nu = _distribution;
    parameters.distribution = _distribution;
}
//
////--------------------------------------------------------------
//float GVF::getDistribution(){
//    return parameters.distribution;
//}

//void GVF::setDimWeights(vector<float> dimWeights){
//    if (dimWeights.size()!=parameters.dimWeights.size())
//        parameters.dimWeights.resize(dimWeights.size());
//    parameters.dimWeights = dimWeights;
//}
//
//vector<float> GVF::getDimWeights(){
//    return parameters.dimWeights;
//}


//// VARIANCE COEFFICIENTS: PHASE
////--------------------------------------------------------------
//void GVF::setAlignmentVariance(float alignmentVariance){
//    parameters.alignmentVariance = sqrt(alignmentVariance);
//}
////--------------------------------------------------------------
//float GVF::getAlignmentVariance(){
//    return parameters.alignmentVariance;
//}


// VARIANCE COEFFICIENTS: DYNAMICS
//--------------------------------------------------------------
//void GVF::setDynamicsVariance(float dynVariance)
//{
//    for (int k=0; k< parameters.dynamicsVariance.size(); k++)
//        parameters.dynamicsVariance[k] = dynVariance;
//}
//--------------------------------------------------------------
void GVF::setDynamicsVariance(float dynVariance, int dim)
{
    if (dim == -1)
    {
        for (int k=0; k< parameters.dynamicsVariance.size(); k++)
            parameters.dynamicsVariance[k] = dynVariance;
    }
    else
    {
        if (dim<parameters.dynamicsVariance.size())
            parameters.dynamicsVariance[dim-1] = dynVariance;
    }
}

//--------------------------------------------------------------
void GVF::setDynamicsVariance(vector<float> dynVariance)
{
    parameters.dynamicsVariance = dynVariance;
}
//--------------------------------------------------------------
vector<float> GVF::getDynamicsVariance()
{
    return parameters.dynamicsVariance;
}

//--------------------------------------------------------------
void GVF::setScalingsVariance(float scaleVariance, int dim)
{
    if (dim == -1)
    {
        for (int k=0; k< parameters.scalingsVariance.size(); k++)
            parameters.scalingsVariance[k] = scaleVariance;
    }
    else
    {
        if (dim<parameters.scalingsVariance.size())
            parameters.scalingsVariance[dim-1] = scaleVariance;
    }
}

//--------------------------------------------------------------
void GVF::setScalingsVariance(vector<float> scaleVariance)
{
    parameters.scaleVariance = scaleVariance;
}

//--------------------------------------------------------------
vector<float> GVF::getScalingsVariance()
{
    return parameters.scalingsVariance;
}

//--------------------------------------------------------------
void GVF::setRotationsVariance(float rotationVariance, int dim)
{
    if (dim == -1)
    {
        for (int k=0; k< parameters.rotationsVariance.size(); k++)
            parameters.rotationsVariance[k] = rotationVariance;
    }
    else
    {
        if (dim<parameters.rotationsVariance.size())
            parameters.scalingsVariance[dim-1] = rotationVariance;
    }
}

//--------------------------------------------------------------
void GVF::setRotationsVariance(vector<float> rotationVariance)
{
    parameters.scaleVariance = rotationVariance;
}

//--------------------------------------------------------------
vector<float> GVF::getRotationsVariance()
{
    return parameters.rotationsVariance;
}

//--------------------------------------------------------------
void GVF::setSpreadDynamics(float center, float range, int dim)
{
    parameters.dynamicsSpreadingCenter = center;
    parameters.dynamicsSpreadingRange = range;
}

//--------------------------------------------------------------
void GVF::setSpreadScalings(float center, float range, int dim)
{
    parameters.scalingsSpreadingCenter = center;
    parameters.scalingsSpreadingRange = range;
}

//--------------------------------------------------------------
void GVF::setSpreadRotations(float center, float range, int dim)
{
    parameters.rotationsSpreadingCenter = center;
    parameters.rotationsSpreadingRange = range;
}

//--------------------------------------------------------------
void GVF::translate(bool translateFlag)
{
    config.translate = translateFlag;
}

//--------------------------------------------------------------
void GVF::segmentation(bool segmentationFlag)
{
    config.segmentation = segmentationFlag;
}


// UTILITIES

//--------------------------------------------------------------
// Save function. This function is used by applications to save the
// vocabulary in a text file given by filename (filename is also the complete path + filename)
void GVF::saveTemplates(string filename){
    
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
void GVF::loadTemplates(string filename){
    //    clear();
    //
    
    GVFGesture loadedGesture;
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







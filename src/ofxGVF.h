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


#ifndef _H_OFXGVF
#define _H_OFXGVF

#include "ofxGVFTypes.h"
#include "ofxGVFGesture.h"
//#include <math.h>
#include <random>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>


using namespace std;



class ofxGVF
{
    
public:
    
    enum ofxGVFState{
        STATE_CLEAR = 0,
        STATE_LEARNING,
        STATE_FOLLOWING
    };
    
    
#pragma mark - Constructors
    
    /**
     * GVF default constructor
     * @details use default configuration and parameters, can be changed using accessors
     */
    ofxGVF();
    
    /**
     * GVF default constructor
     * @param configuration structure (nb. dimensions etc), use default parameters
     */
    ofxGVF(ofxGVFConfig _config);
    
    /**
     * GVF default constructor
     * @param configuration structure (nb. dimensions etc)
     * @param parameters structure
     */
    ofxGVF(ofxGVFConfig _config, ofxGVFParameters _parameters);
    
    /**
     * GVF default destructor
     */
    ~ofxGVF();
    
#pragma mark - Gesture templates
    
    /**
     * Add gesture template to the vocabulary
     *
     * @details a gesture template is a ofxGVFGesture object
     * @param the gesture template to be recorded
     */
    void addGestureTemplate(ofxGVFGesture & gestureTemplate);
    
    /**
     * Replace a specific gesture template by another
     *
     * @param the gesture template to be used
     * @param the gesture index (as integer) to be replaced
     */
    void replaceGestureTemplate(ofxGVFGesture & gestureTemplate, int ID);
    
    /**
     * Remove a specific template
     *
     * @param the gesture index (as integer) to be removed
     */
    void removeGestureTemplate(int index);
    
    /**
     * Remove every recorded gesture template
     */
    void removeAllGestureTemplates();
    
    /**
     * Get a specific gesture template a gesture template by another
     *
     * @param index of the template to be returned
     * @return the template
     */
    ofxGVFGesture & getGestureTemplate(int index);
    
    /**
     * Get every recorded gesture template
     *
     * @return the vecotr of gesture templates
     */
    vector<ofxGVFGesture> & getAllGestureTemplates();
    
    /**
     * Get number of gesture templates in the vocabulary
     * @return the number of templates
     */
    int getNumberOfGestureTemplates();
    
    
#pragma mark - Recognition and tracking
    
    ofxGVFState setState(ofxGVFState _state);
    
    /**
     * Compute the estimated gesture and its potential variations
     *
     * @details infers the probability that the current observation belongs to
     * one of the recorded gesture template and track the variations of this gesture
     * according to each template
     *
     * @param vector of the observation data at current time
     * @return the estimated probabilities and variaitons relative to each template
     */
    ofxGVFOutcomes & update(vector<float> & obs);
    
    /**
     * Define a subset of gesture templates on which to perform the recognition
     * and variation tracking
     *
     * @details By default every recorded gesture template is considered
     * @param set of gesture template index to consider
     */
    void setActiveGestures(vector<int> activeGestureIds);
    
    /**
     * Restart GVF
     * @details re-sample particles at the origin (i.e. initial prior)
     */
    void restart();
    
    /**
     * Clear GVF
     * @details delete templates
     */
    void clear();
    
#pragma mark - Output accessors (deprecated, use ofxGVFOutcomes)
    
    /**
     * Get gesture probabilities
     * @return vector of probabilities
     */
    vector<float> & getGestureProbabilities();
    
    /**
     *
     */
    int getMostProbableGestureIndex();
    
    /**
     * Get particle values
     * @return vector of list of estimated particles
     */
    const vector<vector<float> > & getParticlesPositions();
    
    //    ofxGVFEstimation getTemplateRecogInfo(int templateNumber);
    //    ofxGVFEstimation getRecogInfoOfMostProbable();
    
    //    /////////////////////
    //    // System Function //
    //    /////////////////////
    //
    //    int getDynamicsDim();
    //    int getScalingsDim();
    //    int getRotationsDim();
    
#pragma mark Set/Get


    
    void setConfig(ofxGVFConfig _config);
    ofxGVFConfig getConfig();
    
    void setParameters(ofxGVFParameters parameters);
    ofxGVFParameters getParameters();
    
    void setNumberOfParticles(int numberOfParticles);
    int getNumberOfParticles();
    
    void setPredictionLoops(int predictionLoops);
    int getPredictionLoops();
    
    void setResamplingThreshold(int resamplingThreshold);
    int getResamplingThreshold();
    
    void setTolerance(float tolerance);
    float getTolerance();
    
    void setDistribution(float distribution);
    float getDistribution();
    
    void setDimWeights(vector<float> dimWeights);
    vector<float> getDimWeights();
    
    // --- VARIANCES ---
    // alignement variance
    void setAlignmentVariance(float alignmentVariance);
    float getAlignmentVariance();
    
    // dynamics variance
    void setDynamicsVariance(float dynVariance);
    void setDynamicsVariance(float dynVariance, int dim);
    void setDynamicsVariance(vector<float> dynVariance);
    vector<float> getDynamicsVariance();
    
    // scalings variance
    void setScalingsVariance(float scaleVariance);
    void setScalingsVariance(float scaleVariance, int dim);
    void setScalingsVariance(vector<float> scaleVariance);
    vector<float> getScalingsVariance();
    
    // --- SPREADING CENTER/RANGE ---
    // angles
    void setSpreadDynamics(float center, float range);
    void setSpreadScalings(float center, float range);
    void setSpreadRotations(float center, float range);
    
    
    
    //    // oldies (deprecated)
    //    void setSpeedVariance(float speedVariance);
    //    float getSpeedVariance();
    //
    //    void setScaleVariance(float scaleVariance, int dim = 0);
    //    void setScaleVariance(vector<float> scaleVariance);
    //    vector<float> getScaleVariance();
    //
    //    void setRotationsVariance(float rotationsVariance, int dim = 0);
    //    void setRotationsVariance(vector<float> rotationsVariance);
    //    vector<float> getRotationsVariance();
    
    
    
    
    //    // MATHS
    //
    vector<int> getClasses();
    //    vector<float> getAlignment();
    //    vector<float> getEstimatedAlignment();
    //    vector< vector<float> > getDynamics();
    //    vector< vector<float> > getEstimatedDynamics();
    //    vector< vector<float> > getScalings();
    //    vector< vector<float> > getEstimatedScalings();
    //    vector< vector<float> > getRotations();
    //    vector< vector<float> > getEstimatedRotations();
    vector<float> getEstimatedProbabilities();
    vector<float> getEstimatedLikelihoods();
    vector<float> getWeights();
    vector<float> getPrior();
    
    //    // only fo rlogs
    //    vector<vector<float> >  getVecRef();
    //    vector<float>           getVecObs();
    //    vector<float>           getStateNoiseDist();
    //
    //    vector< vector<float> > getX();
    //    vector<int>    getG();
    //    vector<float>  getW();
    //
    //    // MISC
    //
    //    vector<vector<float> >  getIndividualOffset();
    //    vector<float>           getIndividualOffset(int particleIndex);
    
    // UTILITIES
    
    void saveTemplates(string filename);
    void loadTemplates(string filename);
    
    //    string getStateAsString(ofxGVFState state);
    
    //    float getGlobalNormalizationFactor();
    
    //    void testRndNum();
    
#pragma mark - openFrameworks convetion: setup functions
    
    /////////////
    // Set-Ups //
    /////////////
    
    void setup(); // use default config and parameters
    void setup(ofxGVFConfig _config); // default parameters
    void setup(ofxGVFConfig _config, ofxGVFParameters _parameters);
    
#pragma mark - Deprecated functions, to be removed in the next version
    
    /**
     * Perform inference, replaced by update()
     */
    void infer(vector<float> obs);
    ofxGVFOutcomes getOutcomes();

#pragma mark DEPRECATED
    ofxGVFState getState();
    
private:
    
    // private variables
    ofxGVFConfig        config;
    ofxGVFParameters    parameters;
    ofxGVFOutcomes      outcomes;
    ofxGVFState         state;
    ofxGVFGesture       theGesture;
    
    
    vector<float> dimWeights;           // TOOD: to be put in parameters?
    vector<float> maxRange;
    vector<float> minRange;
    int     dynamicsDim;                // dynamics state dimension
    int     scalingsDim;                // scalings state dimension
    int     rotationsDim;               // rotation state dimension
    float   globalNormalizationFactor;          // flagged if normalization
    int     mostProbableIndex;                  // cached most probable index
    
    vector<int>             classes;            // gesture index for each particle [ns x 1]
    vector<float >          alignment;          // alignment index (between 0 and 1) [ns x 1]
    vector<vector<float> >  dynamics;           // dynamics estimation [ns x 2]
    vector<vector<float> >  scalings;           // scalings estimation [ns x D]
    vector<vector<float> >  rotations;          // rotations estimation [ns x A]
    vector<float>           weights;            // weight of each particle [ns x 1]
    vector<float>           prior;              // prior of each particle [ns x 1]
    vector<float>           posterior;          // poserior of each particle [ns x 1]
    vector<float>           likelihood;         // likelihood of each particle [ns x 1]
    
    // estimations
    vector<float>           estimatedGesture;           // ..
    vector<float>           estimatedAlignment;         // ..
    vector<vector<float> >  estimatedDynamics;          // ..
    vector<vector<float> >  estimatedScalings;          // ..
    vector<vector<float> >  estimatedRotations;         // ..
    vector<float>           estimatedProbabilities;     // ..
    vector<float>           estimatedLikelihoods;       // ..
    vector<float>           absoluteLikelihoods;        // ..
    
    // velocity and acceleration estimations
    vector<float>           currentVelAcc;
    vector<vector<float>>   kalmanDynMatrix;
    vector<vector<float>>   kalmanMapMatrix;
    
    
    bool tolerancesetmanually;
    
    vector<ofxGVFGesture>   gestureTemplates;           // ..
    vector<vector<float> >  offsets;                    // translation offset
    
    vector<int> activeGestures;
    
    // Model mechanics
    void initStateSpace();
    void initStateValues();
    void initStateValues(int pf_n, float range);
    void initPrior();
    void initPrior(int pf_n);
    void initNoiseParameters();
    
    void updateStateSpace(int n);
    void updateLikelihood(vector<float> obs, int n);
    void updatePrior(int n);
    void updatePosterior(int n);
    
    // particle filer resampling method
    void resampleAccordingToWeights(vector<float> obs);
    
    
    // random number generator
    std::random_device                      rd;
    std::mt19937                            normgen;
    std::normal_distribution<float>         *rndnorm;
    std::default_random_engine              unifgen;
    std::uniform_real_distribution<float>   *rndunif;
    
    
    // ONLY for LOGS
    vector<vector<float> >  vecRef;
    vector<float>           vecObs;
    vector<float>           stateNoiseDist;
    
    vector<float> gestureProbabilities;
    vector< vector<float> > particles;
    //    vector<float> particlesPositions;
    
    vector<float>& getGestureTemplateSample(int gestureIndex, float cursor);
    void estimates();       // update estimated outcome
    void train();
    
    
};


class GVFGesture
{
public:
    
    GVFGesture()
    {
        inputDimensions = 2;
        setAutoAdjustRanges(true);
        templatesRaw    = vector<vector<vector<float > > >();
        templatesNormal = vector<vector<vector<float > > >();
        clear();
    }
    
    GVFGesture(int _inputdim){
        inputDimensions = _inputdim;
        setAutoAdjustRanges(true);
        templatesRaw    = vector<vector<vector<float > > >();
        templatesNormal = vector<vector<vector<float > > >();
        clear();
    }
    
    ~GVFGesture(){
        clear();
    }
    
    void setNumberDimensions(int dimensions){
        assert(dimensions > 0);
        inputDimensions = dimensions;
    }
    
    void setAutoAdjustRanges(bool b){
//        if(b) bIsRangeMinSet = bIsRangeMaxSet = false;
        bAutoAdjustNormalRange = b;
    }
    
    void setMax(float x, float y){
        assert(inputDimensions == 2);
        vector<float> r(2);
        r[0] = x; r[1] = y;
        setMaxRange(r);
    }
    
    void setMin(float x, float y){
        assert(inputDimensions == 2);
        vector<float> r(2);
        r[0] = x; r[1] = y;
        setMinRange(r);
    }
    
    void setMax(float x, float y, float z){
        assert(inputDimensions == 3);
        vector<float> r(3);
        r[0] = x; r[1] = y; r[2] = z;
        setMaxRange(r);
    }
    
    void setMin(float x, float y, float z){
        assert(inputDimensions == 3);
        vector<float> r(3);
        r[0] = x; r[1] = y; r[2] = z;
        setMinRange(r);
    }
    
    void setMaxRange(vector<float> observationRangeMax){
        this->observationRangeMax = observationRangeMax;
//        bIsRangeMaxSet = true;
        normalise();
    }
    
    void setMinRange(vector<float> observationRangeMin){
        this->observationRangeMin = observationRangeMin;
//        bIsRangeMinSet = true;
        normalise();
    }
    
    vector<float>& getMaxRange(){
        return observationRangeMax;
    }
    
    vector<float>& getMinRange(){
        return observationRangeMin;
    }
    
    void autoAdjustMinMax(vector<float> & observation){
        if(observationRangeMax.size()  < inputDimensions){
            observationRangeMax.assign(inputDimensions, -INFINITY);
            observationRangeMin.assign(inputDimensions,  INFINITY);
        }
        for(int i = 0; i < inputDimensions; i++){
            observationRangeMax[i] = MAX(observationRangeMax[i], observation[i]);
            observationRangeMin[i] = MIN(observationRangeMin[i], observation[i]);
        }
    }
    
    void addObservation(vector<float> observation, int templateIndex = 0){
        if (observation.size() != inputDimensions)
            inputDimensions = observation.size();
        
        // check we have a valid templateIndex and correct number of input dimensions
        assert(templateIndex <= templatesRaw.size());
        assert(observation.size() == inputDimensions);
        
        // if the template index is same as the number of temlates make a new template
        if(templateIndex == templatesRaw.size()){ // make a new template
            
            // reserve space in raw and normal template storage
            templatesRaw.resize(templatesRaw.size() + 1);
            templatesNormal.resize(templatesNormal.size() + 1);
            
        }
        
        if(templatesRaw[templateIndex].size() == 0)
        {
            templateInitialObservation = observation;
            templateInitialNormal = observation;
        }
        
        for(int j = 0; j < observation.size(); j++)
            observation[j] = observation[j] - templateInitialObservation[j];
        
        // store the raw observation
        templatesRaw[templateIndex].push_back(observation);

        autoAdjustMinMax(observation);
        
        normalise();
    }
    
    
    
    void normalise()
    {
        templatesNormal.resize(templatesRaw.size());
        for(int t = 0; t < templatesRaw.size(); t++)
        {
            templatesNormal[t].resize(templatesRaw[t].size());
            for(int o = 0; o < templatesRaw[t].size(); o++)
            {
                templatesNormal[t][o].resize(inputDimensions);
                for(int d = 0; d < inputDimensions; d++)
                {
                    templatesNormal[t][o][d] = templatesRaw[t][o][d] / (observationRangeMax[d] - observationRangeMin[d]);
                    templateInitialNormal[d] = templateInitialObservation[d] / (observationRangeMax[d] - observationRangeMin[d]);
                }
            }
        }
    }
    
    void setTemplate(vector< vector<float> > & observations, int templateIndex = 0){
        for(int i = 0; i < observations.size(); i++){
            addObservation(observations[i], templateIndex);
        }
    }
    
    vector< vector<float> > & getTemplate(int templateIndex = 0){
        assert(templateIndex < templatesRaw.size());
        return templatesRaw[templateIndex];
    }
    
    int getNumberOfTemplates(){
        return templatesRaw.size();
    }
    
    int getNumberDimensions(){
        return inputDimensions;
    }
    
    int getTemplateLength(int templateIndex = 0){
        return templatesRaw[templateIndex].size();
    }
    
    int getTemplateDimension(int templateIndex = 0){
        return templatesRaw[templateIndex][0].size();
    }
    
    vector<float>& getLastObservation(int templateIndex = 0){
        return templatesRaw[templateIndex][templatesRaw[templateIndex].size() - 1];
    }
    
    vector< vector< vector<float> > >& getTemplates(){
        return templatesRaw;
    }
    
    vector<float>& getInitialObservation(){
        return templateInitialObservation;
    }
    
    void deleteTemplate(int templateIndex = 0)
    {
        assert(templateIndex < templatesRaw.size());
        templatesRaw[templateIndex].clear();
        templatesNormal[templateIndex].clear();
    }
    
    void clear()
    {
        templatesRaw.clear();
        templatesNormal.clear();
        observationRangeMax.assign(inputDimensions, -INFINITY);
        observationRangeMin.assign(inputDimensions,  INFINITY);
    }

private:
    
    int inputDimensions;
    bool bAutoAdjustNormalRange;
    
    vector<float> observationRangeMax;
    vector<float> observationRangeMin;
    
    vector<float> templateInitialObservation;
    vector<float> templateInitialNormal;
    
    vector< vector< vector<float> > > templatesRaw;
    vector< vector< vector<float> > > templatesNormal;
    
    vector<vector<float> > gestureDataFromFile;
};

#endif
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


    

//    void addGestureExamples(vector<ofxGVFGesture> & gestureExamples);
//    vector<ofxGVFGesture> & getGestureExamples(int gestureIndex);       // get all the examples of a given gesture
//    ofxGVFGesture & getGestureExample(int gestureIndex, int exampleIndex = 0);   // get one example of a given gesture (default: the first one)
//    int getNumberOfGestures();                          // return the number of gesture classes (not counting the examples)
//    int getNumberOfGestureExamples(int gestureIndex);   // return the number of examples for a given gesture
    
    
#pragma mark - Recognition and tracking


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
    
    
#pragma mark - Output accessors
    
    
    //////////////////////////
    // OUTCOMES //
    //////////////////////////
    
    int getMostProbableGestureIndex();
    

    ofxGVFEstimation getTemplateRecogInfo(int templateNumber);
    ofxGVFEstimation getRecogInfoOfMostProbable(); // !!!: bad naming
    
    /*
     * Get gesture probabilities
     * @return vector of probabilities
     */
    vector<float> & getGestureProbabilities();

    /*
     * Get first sampled alignment value for each particle
     * TODO: why matrix? particlepositions not filled
     * @return ...
     */
    vector< vector<float> > & getParticlesPositions();
    
    
    
    /////////////////////
    // System Function //
    /////////////////////
    
    int getDynamicsDim();
    int getScalingsDim();
    int getRotationsDim();
    
    void restart();     // restart the GVF
    void clear();       // clear templates etc.
    
    
    
    ///////////////////////
    // Getters & Setters //
    ///////////////////////
    
    // STATES
    
    ofxGVFState getState();
    string getStateAsString();
    void setState(ofxGVFState _state);
    
    // CONFIG
    
    void setConfig(ofxGVFConfig _config);
    ofxGVFConfig getConfig();
    
    
#pragma mark Set/Get Parameters
    
    // PARAMETERS
    // ==========
    
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
    
    
    
    // oldies (deprecated)
    void setSpeedVariance(float speedVariance);
    float getSpeedVariance();
    
    void setScaleVariance(float scaleVariance, int dim = 0);
    void setScaleVariance(vector<float> scaleVariance);
    vector<float> getScaleVariance();
    
    void setRotationsVariance(float rotationsVariance, int dim = 0);
    void setRotationsVariance(vector<float> rotationsVariance);
    vector<float> getRotationsVariance();
    
    
    
    
    // MATHS
    
    vector<int> getClasses();
    vector<float> getAlignment();
    vector<float> getEstimatedAlignment();
    vector< vector<float> > getDynamics();
    vector< vector<float> > getEstimatedDynamics();
    vector< vector<float> > getScalings();
    vector< vector<float> > getEstimatedScalings();
    vector< vector<float> > getRotations();
    vector< vector<float> > getEstimatedRotations();
    vector<float> getEstimatedProbabilities();
    vector<float> getEstimatedLikelihoods();
    vector<float> getWeights();
    vector<float> getPrior();
    
    // only fo rlogs
    vector<vector<float> >  getVecRef();
    vector<float>           getVecObs();
    vector<float>           getStateNoiseDist();
    
    vector< vector<float> > getX();
    vector<int>    getG();
    vector<float>  getW();
    
    // MISC
    
    vector<vector<float> >  getIndividualOffset();
    vector<float>           getIndividualOffset(int particleIndex);
    
    // UTILITIES
    
    void saveTemplates(string filename);
    void loadTemplates(string filename);
    
    string getStateAsString(ofxGVFState state);
    
    float getGlobalNormalizationFactor();
    
    void testRndNum();
    
#pragma mark - openFrameworks convetion: setup functions
    
    /////////////
    // Set-Ups //
    /////////////
    
    void setup(); // use default config and parameters
    void setup(ofxGVFConfig _config); // default parameters
    void setup(ofxGVFConfig _config, ofxGVFParameters _parameters);
    
#pragam mark - Deprecated functions, to be removed in the next version
    
    /**
     * Perform inference, replaced by update()
     */
    void infer(vector<float> obs);
        ofxGVFOutcomes getOutcomes();
    
private:
    
    // private variables
    ofxGVFConfig        config;
    ofxGVFParameters    parameters;
    ofxGVFOutcomes      outcomes;
    ofxGVFState         state;
    
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
    vector< vector<float> > particlesPositions;
    
    
    vector<float>& getGestureTemplateSample(int gestureIndex, float cursor);
    void estimates();       // update estimated outcome
    void train();
    
    
};

#endif
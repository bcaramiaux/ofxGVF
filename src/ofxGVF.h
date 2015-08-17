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

// Recognizes gesture and tracks the variations. A set of gesture templates
// must be recorded. Then, at each new observation, the algorithm estimates
// which gesture is performed and adapts a set fo features (gesture variations)
// that are used as invariants for the recognition and gives continuous output
// parameters (e.g. for interaction)
//
// typical use:
//   ofxGVF *myGVF = new ofxGVF(...)
//   myGVF->addTemplate();
//   myGVF->fillTemplate();
//   myGVF->addTemplate();
//   myGVF->fillTemplate();
//   ...
//   myGVF->infer();
//   myGVF->getEstimatedStatus();

class ofxGVF{
	
public:
	
    enum ofxGVFState{
        STATE_CLEAR = 0,
        STATE_LEARNING,
        STATE_FOLLOWING
    };
    
    
    
    ///////////////////////////////
    // Constructors / Destructor //
	///////////////////////////////
    
	ofxGVF(); // use default config and parameters
    ofxGVF(ofxGVFConfig _config); // use default parameters
	ofxGVF(ofxGVFConfig _config, ofxGVFParameters _parameters);
    
	// destructor
    ~ofxGVF();

#pragma mark Setup Functions
    
    /////////////
    // Set-Ups //
	/////////////
	
    void setup(); // use default config and parameters
    void setup(ofxGVFConfig _config); // default parameters
    void setup(ofxGVFConfig _config, ofxGVFParameters _parameters);
    
    
#pragma mark Gesture Templates
    
    //////////////////////////
    // Gestures & Templates //
	//////////////////////////
    
    // add gestures
    void addGestureTemplate(ofxGVFGesture & gestureTemplate);
    void replaceGestureTemplate(ofxGVFGesture & gestureTemplate, int ID);

    // get gestures or infos
    ofxGVFGesture & getGestureTemplate(int index);
    vector<ofxGVFGesture> & getAllGestureTemplates();
    int getNumberOfGestureTemplates();
    vector<float>& getGestureTemplateSample(int gestureIndex, float cursor);

    // remove gesture
    void removeGestureTemplate(int index);
    void removeAllGestureTemplates();

    
    // TODO: some methods below (not implemented) to handle multi-examples for a gesture
    void addGestureExamples(vector<ofxGVFGesture> & gestureExamples);   // add examples of one gesture to the Vocabulary
    vector<ofxGVFGesture> & getGestureExamples(int gestureIndex);       // get all the examples of a given gesture
    ofxGVFGesture & getGestureExample(int gestureIndex, int exampleIndex = 0);   // get one example of a given gesture (default: the first one)
    int getNumberOfGestures();                          // return the number of gesture classes (not counting the examples)
    int getNumberOfGestureExamples(int gestureIndex);   // return the number of examples for a given gesture
    
    void setActiveGestures(vector<int> activeGestureIds);
    
    ///////////////
    // INFERENCE //
	///////////////

    void learn();               // learn parameters from the gesture examples

    void infer(vector<float> obs);     // call the inference method on the observation (DEPRECATED)
    void update(vector<float> & obs);               // incremental step of filtering given the obs

    void estimates();       // update estimated outcome
    

    

    
    
    
    //////////////////////////
    // OUTCOMES //
	//////////////////////////
    
    int getMostProbableGestureIndex();
 
    ofxGVFOutcomes getOutcomes();
    ofxGVFEstimation getTemplateRecogInfo(int templateNumber);
    ofxGVFEstimation getRecogInfoOfMostProbable(); // !!!: bad naming
    
    
    vector<float> getGestureProbabilities();
    vector< vector<float> > getParticlesPositions();
    

    
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
    
    
};

#endif
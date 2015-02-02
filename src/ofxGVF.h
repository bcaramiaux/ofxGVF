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
#include <math.h>


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

    
    
    /////////////
    // Set-Ups //
	/////////////
	
    void setup(); // use default config and parameters
    void setup(ofxGVFConfig _config); // default parameters
    void setup(ofxGVFConfig _config, ofxGVFParameters _parameters);

    
    
    //////////////////////////
    // Gestures & Templates //
	//////////////////////////
    
    void addGestureTemplate(ofxGVFGesture & gestureTemplate);
    ofxGVFGesture & getGestureTemplate(int index);
    vector<ofxGVFGesture> & getAllGestureTemplates();
    int getNumberOfGestureTemplates();
    
    vector<float>& getGestureTemplateSample(int gestureIndex, float cursor);
    
    void removeGestureTemplate(int index);
    void removeAllGestureTemplates();
    
    // TODO: clearTemplate by given ID
    void clearTemplate(int id);
    
    
    // TODO: some methods below (not implemented) to handle multi-examples for a gesture
    void addGestureExamples(vector<ofxGVFGesture> & gestureExamples);   // add examples of one gesture to the Vocabulary
    vector<ofxGVFGesture> & getGestureExamples(int gestureIndex);       // get all the examples of a given gesture
    ofxGVFGesture & getGestureExample(int gestureIndex, int exampleIndex = 0);   // get one example of a given gesture (default: the first one)
    int getNumberOfGestures();                          // return the number of gesture classes (not counting the examples)
    int getNumberOfGestureExamples(int gestureIndex);   // return the number of examples for a given gesture
    
    
    
    /////////////////////
    // Particle Filter //
	/////////////////////

    void learn();               // learn parameters from the gesture examples

    void update(vector<float> & obs);               // incremental step of filtering given the obs
    void estimates();       // update estimated outcome
    void resampleAccordingToWeights(vector<float> obs);     // resampling process
    
    
    
    ///////////////
    // Inference //
	///////////////

	void infer(vector<float> obs);     // call the inference method on the observation

    
    
    
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
    
    // PARAMETERS
    
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
    
    
    // VARIANCE COEFFICENTS (in PARAMETERS)
    
    void setAlignmentVariance(float alignmentVariance);
    float getAlignmentVariance();
    
    void setScalingsVariance(float scaleVariance, int dim);
    void setScalingsVariance(vector<float> scaleVariance);
    vector<float> getScalingsVariance();
    
    void setDynamicsVariance(float dynVariance, int dim);
    void setDynamicsVariance(vector<float> dynVariance);
    vector<float> getDynamicsVariance();
    
    
    // oldies
    void setSpeedVariance(float speedVariance);
    float getSpeedVariance();
    
    void setScaleVariance(float scaleVariance, int dim = 0);
    void setScaleVariance(vector<float> scaleVariance);
    vector<float> getScaleVariance();
    
    void setRotationVariance(float rotationVariance, int dim = 0);
    void setRotationVariance(vector<float> rotationVariance);
    vector<float> getRotationVariance();
    
    // MATHS
    
    vector<int> getClasses();
    vector<float> getAlignment();
    vector<float> getEstimatedAlignment();
    vector< vector<float> > getDynamics();
    vector< vector<float> > getEstimatedDynamics();
    vector< vector<float> > getScalings();
    vector< vector<float> > getEstimatedScalings();
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
    
	

    
private:
    
    // private variables
    

    ofxGVFConfig        config;
    ofxGVFParameters    parameters;
    ofxGVFOutcomes      outcomes;
    
    // TOOD: to be put in parameters?
    vector<float> dimWeights;
    
//    ofxGVFVarianceCoefficents coefficents;
//    ofxGVFInitialSpreadingParameters spreadingParameters;
    
    float   tolerance;          // standard deviation of the observation distribution
	float   nu;                 // degree of freedom for the t-distribution; if 0, use a gaussian
	float   sp, sv, sr, ss;     // sigma values (actually, their square root)
	int     resamplingThreshold;// resampling threshol
	int     pdim;               // number of state dimension
    int     scale_dim;          // scale state dimension
    int     rotation_dim;       // rotation state dimension
    int     dynamicsDim;          // scale state dimension
    int     scalingsDim;       // rotation state dimension
    
    
    bool    has_learned;        // true if gesture templates have been learned
    bool    parametersSetAsDefault;
    float   globalNormalizationFactor;
    
    int mostProbableIndex;                      // cached most probable index
    vector<float> mostProbableStatus;           // cached most probable status [phase, speed, scale[, rotation], probability]
    vector< vector<float> > status;                  // cached estimated status for all templates
	vector< vector<float> > X;                  // each row is a particle
	vector<int>             g;                  // gesture index for each particle [g is ns x 1]
	vector<float>           w;                  // weight of each particle [w is ns x 1]

    vector< vector<float> > offS;               // translation offset

	vector<float>           featVariances;      // vector of variances
	vector<float>           means;              // vector of means for particles initial spreading
	vector<float>           ranges;             // vector of ranges around the means for particles initial spreading
    
    
    // --- new stuff
	vector<int>             classes;            // gesture index for each particle [g is ns x 1]
	vector<float >          alignment;          // each row is a sample of dynamical features
	vector<vector<float> >  dynamics;           // each row is a sample of dynamical features
	vector<vector<float> >  scalings;           // each row is a sample of dynamical features
    
	vector<float>           weights;            // weight of each particle [w is ns x 1]
    vector<float>           prior;              // weight of each particle [w is ns x 1]
    vector<float>           posterior;          // weight of each particle [w is ns x 1]
    vector<float>           likelihood;
    vector<vector<float> >  offsets;            // translation offset
    
    // estimations
    vector<float>           estimatedGesture;   // ..
    vector<float>           estimatedAlignment; // ..
    vector<vector<float> >  estimatedDynamics;  // ..
    vector<vector<float> >  estimatedScalings;  // ..
    vector<float>           estimatedLikelihoods;        // ..
    vector<float>           absoluteLikelihoods;// ..
    
    // keep track of the estimation for the recognized gesture
//    vector<float>           mostProbableStatus; // cached most probable status [phase, speed, scale[, rotation], probability]
    
    
    // gesture 'history'
    //	map<int, vector< vector<float> > > R_single;    // gesture references (1 example)
    //    map<int, vector<float> > R_initial;             // gesture initial data
    //    vector<int> gestureLengths;                     // length of each reference gesture
    //    vector< vector<float> > EmptyTemplate;      // dummy empty template for passing as ref
    //    int     numTemplates;       // number of learned gestures (starts at 0)
    
    vector<float> maxRange;
    vector<float> minRange;
    
    vector<ofxGVFGesture> gestureTemplates;

    
    //////////////
    // MODELING
    //////////////
    void initStateSpace();
    void initStateValues();
    void initStateValues(int pf_n, float range);
    void initPrior();
    void initPrior(int pf_n);
    //void initPrior(int pf_n, float range = 0.1);
    void initNoiseParameters();
    

    
    void updateStateSpace(int n);
    void updateLikelihood(vector<float> obs, int n);
    void updatePrior(int n);
    void updatePosterior(int n);
    
    
    //in order to output particles
    vector< vector<float> > particlesPositions;
    
    ofxGVFState state;                          // store current state of the gesture follower

    // random number generator
#if BOOSTLIB
	boost::variate_generator<boost::mt19937&, boost::normal_distribution<float> > *rndnorm(rng, normdist);
	boost::mt19937 rng;
	boost::normal_distribution<float> normdist;
#else
    tr1::mt19937 rng;
    tr1::normal_distribution<float> *normdist;
    tr1::uniform_real<float> *unifdist;
	tr1::variate_generator<tr1::mt19937, tr1::normal_distribution<float> > *rndnorm; //(rng, *normdist);
#endif
    
//    typedef tr1::mt19937 pseudorandom;
//    typedef tr1::normal_distribution<float> normaldist;
//    typedef tr1::variate_generator<pseudorandom, normaldist> generator;
    
    vector<float> obsOffset;
	// Segmentation variables
	vector<float> abs_weights;
	double probThresh;
    double probThreshMin;
    int currentGest;
    bool compa;
    float old_max;
    vector<float> meansCopy;
    vector<float> rangesCopy;
    vector<float> origin;
    vector<float> *offset;

    
    void UpdateOutcomes();
    
    
    // only for logs
    vector<vector<float> >  vecRef;
    vector<float>           vecObs;
    vector<float>           stateNoiseDist;
    
    
    
};

#endif
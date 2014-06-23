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

// TEST AVZ

#ifndef _H_OFXGVF
#define _H_OFXGVF

#include "ofxGVFTypes.h"
#include "ofxGVFGesture.h"

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
        STATE_WAIT,
        STATE_LEARNING,
        STATE_FOLLOWING
    };
    
	// constructor of the gvf instance
	ofxGVF(); // use defualt parameters
	ofxGVF(ofxGVFConfig _config, ofxGVFParameters _parameters);
    
	// destructor
    ~ofxGVF();
	
    void setup();
    void setup(ofxGVFConfig _config, ofxGVFParameters _parameters);
    void learn();
	// spread particles
	void spreadParticles();         // use default parameter values
	void spreadParticles(vector<float> & means, vector<float> & ranges);
    void spreadParticles(ofxGVFParameters _parameters);
	// inference
    void particleFilter(vector<float> & obs);
	
    // resample particles according to the proba distrib given by the weights
    void resampleAccordingToWeights(vector<float> obs);
	
    // makes the inference by calling particleFilteringOptim
	void infer(vector<float> data);   // rename to update?
	void updateEstimatedStatus();       // should be private?
    
    //////////////////////////
    // Gestures & Templates //
	//////////////////////////
    
    // GESTURE PROBABILITIES + POSITIONS
    
    int getMostProbableGestureIndex();
    vector<float> getMostProbableGestureStatus();
    float getMostProbableProbability(); // this is horrible - maybe probability should be at index 0 OR we should use a struct rather than a vector??
    
    ofxGVFOutcomes getOutcomes();
    ofxGVFOutcomes getOutcomes(int gestureIndex);
    
    vector< vector<float> > getEstimatedStatus();
    
    vector<float> getGestureProbabilities();
    vector< vector<float> > getParticlesPositions();
    
    // TEMPLATES
    
    void addGestureTemplate(ofxGVFGesture & gestureTemplate);
    ofxGVFGesture & getGestureTemplate(int index);
    vector<ofxGVFGesture> & getAllGestureTemplates();
    int getNumberOfGestureTemplates();
    
    void removeGestureTemplate(int index);
    void removeAllGestureTemplates();
    
//    // add template to the vocabulary
//	void addTemplate();
//	void addTemplate(vector<float> & data);
//    
//	// fill template given by id with data vector
//	void fillTemplate(int id, vector<float> & data);
//    
//    // clear template given by id
//    void clearTemplate(int id);
//    
	// reset ofxGVF
	void clear();
//
//    // template utilities
//    int getNumberOfTemplates();
//    vector< vector<float> >& getTemplateByIndex(int index);
//    int getLengthOfTemplateByIndex(int index);
    
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
    
	void setResamplingThreshold(int resamplingThreshold);
    int getResamplingThreshold();
    
    void setTolerance(float tolerance);
    float getTolerance();
    
    void setDistribution(float distribution);
    float getDistribution();
    
    
    // VARIANCE COEFFICENTS (in PARAMETERS)
    
    void setPhaseVariance(float phaseVariance);
    float getPhaseVariance();
    
    void setSpeedVariance(float speedVariance);
    float getSpeedVariance();
    
    void setScaleVariance(float scaleVariance);
    float getScaleVariance();
    
    void setRotationVariance(float rotationVariance);
    float getRotationVariance();
    
    // MATHS
    
    float   getObservationStandardDeviation();
    
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
	

    
private:
    
    // private variables
    

    ofxGVFConfig        config;
    ofxGVFParameters    parameters;
    ofxGVFOutcomes      outcomes;
    
    
//    ofxGVFVarianceCoefficents coefficents;
//    ofxGVFInitialSpreadingParameters spreadingParameters;
    
    float   tolerance;          // standard deviation of the observation distribution
	float   nu;                 // degree of freedom for the t-distribution; if 0, use a gaussian
	float   sp, sv, sr, ss;     // sigma values (actually, their square root)
	int     resamplingThreshold;// resampling threshol
    int     ns;
	int     pdim;               // number of state dimension
    int     inputDim;           // Dimension of the input data
    
    int mostProbableIndex;                      // cached most probable index
    vector<float> mostProbableStatus;           // cached most probable status [phase, speed, scale[, rotation], probability]
    vector< vector<float> > S;                  // cached estimated status for all templates
	vector< vector<float> > X;                  // each row is a particle
	vector<int>             g;                  // gesture index for each particle [g is ns x 1]
	vector<float>           w;                  // weight of each particle [w is ns x 1]
    vector< vector<float> > offS;               // translation offset
	vector<float>           featVariances;      // vector of variances
	vector<float>           means;              // vector of means for particles initial spreading
	vector<float>           ranges;             // vector of ranges around the means for particles initial spreading
    
    // gesture 'history'
//	map<int, vector< vector<float> > > R_single;    // gesture references (1 example)
//    map<int, vector<float> > R_initial;             // gesture initial data
//    vector<int> gestureLengths;                     // length of each reference gesture
//    vector< vector<float> > EmptyTemplate;      // dummy empty template for passing as ref
//    int     numTemplates;       // number of learned gestures (starts at 0)
    
    vector<float> maxRange;
    vector<float> minRange;
    
    vector<ofxGVFGesture> gestureTemplates;

    
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
	tr1::variate_generator<tr1::mt19937, tr1::normal_distribution<float> > *rndnorm;//(rng, *normdist);
#endif
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
    
    // private functions
    void initweights();                         // initialize weights
    
};

#endif
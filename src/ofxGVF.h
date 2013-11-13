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

#include "ofxGVFMatrix.h"

#include <map>
#include <tr1/random>
#include <iostream>

#define BOOSTLIB 0
#define OPTIMISD 0
#define VDSPOPTM 0
#define GESTLEARNT 8

#if BOOSTLIB
#include <boost/random.hpp>
#endif

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
    
    typedef struct{
        int inputDimensions;
        int numberParticles;
        float phaseVariance;
        float speedVariance;
        float scaleVariance;
        float rotationVariance;
        float tolerance;
        int resamplingThreshold;
        float distribution;
    } ofxGVFParameters;
    
	// constructor of the gvf instance
	ofxGVF(); // use defualt parameters
	ofxGVF(ofxGVFParameters parameters);
    
	// destructor
    ~ofxGVF();
	
    void setup();
    void setup(ofxGVFParameters parameters);
    
	// add template to the vocabulary
	void addTemplate();
	void addTemplate(vector<float> & data);
    
	// fill template given by id with data vector
	void fillTemplate(int id, vector<float> & data);

    // clear template given by id
    void clearTemplate(int id);
    
	// clear the templates
	void clear();
    
	// spread particles
	void spreadParticles();         // use default parameter values
	void spreadParticles(vector<float> & means, vector<float> & ranges);
    
	// inference
    void particleFilter(vector<float> & obs);
	
    // resample particles according to the proba distrib given by the weights
    void resampleAccordingToWeights();
	
    // makes the inference by calling particleFilteringOptim
	void infer(vector<float> & vect); 
	
	// Gets
	/////////
	float   getObservationNoiseStd();
    int     getResamplingThreshold();
	int     getNumberOfParticles();
	int     getNumberOfTemplates();
	int     getLengthOfTemplateByIndex(int index);
    vector< vector<float> >& getTemplateByIndex(int index);
    
    vector< vector<float> > getX();
	vector<int>    getG();
	vector<float>  getW();
    
    vector<float>  getGestureProbabilities();
    vector<float>  getGestureConditionnalProbabilities(); // ----- DEPRECATED ------
    
	vector< vector<float> > getEstimatedStatus();
    vector<float>  getFeatureVariances();
	vector< vector<float> >& getParticlesPositions();
    int getMostProbableGestureIndex();
    
    // Sets
	////////
	void setNumberOfParticles(int newNs);
	void setToleranceValue(float f);
	void setAdaptSpeed(vector<float> as);
	void setResamplingThreshold(int r);
    
	// Misc
	////////
	void saveTemplates(string filename);
    void loadTemplates(string filename);
	
private:
    
    // private variables
    ofxGVFState state;
    vector< vector<float> > EmptyTemplate;      // dummy empty template for passing as ref
	vector< vector<float> > X;                  // each row is a particle
	vector<int>             g;                  // gesture index for each particle [g is ns x 1]
	vector<float>           w;                  // weight of each particle [w is ns x 1]
    vector< vector<float> > offS;               // translation offset
	vector<float>           featVariances;      // vector of variances
	vector<float>           means;              // vector of means for particles initial spreading
	vector<float>           ranges;             // vector of ranges around the means for particles initial spreading
    float   tolerance;          // standard deviation of the observation distribution
	float   nu;                 // degree of freedom for the t-distribution; if 0, use a gaussian
	float   sp, sv, sr, ss;     // sigma values (actually, their square root)
	int     resamplingThreshold;// resampling threshol
    int     ns;
	int     pdim;               // number of state dimension
	int     numTemplates;       // number of learned gestures (starts at 0)
    int     inputDim;           // Dimension of the input data
    vector<int>    gestureLengths;             // length of each reference gesture
    vector<float>  particlesPhaseLt0;          // store particles whose phase is < 0 (outside of the gesture)
    vector<float>  particlesPhaseGt1;          // store particles whose phase is > 1 (outside of the gesture)
    
	map<int, vector< vector<float> > > R_single;   // gesture references (1 example)
    
    // private functions
    void initweights();                         // initialize weights
	
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
    
    //in order to output particles
    vector< vector<float> > particlesPositions;
    
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
    
    bool new_gest;
    void setInitCoord(vector<float> s_origin);
    
};

#endif
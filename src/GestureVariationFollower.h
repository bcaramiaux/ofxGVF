///////////////////////////////////////////////////////////////////////
//
//  GestureVariationFollower class
//
//  The library (GestureVariationFollower.cpp, GestureVariationFollower.h) has been created in 2010-2011 at Ircam Centre Pompidou by
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


#ifndef GestureVariationFollower_H
#define GestureVariationFollower_H

#include <vector>
#include <Eigen/Core>
#include <tr1/random>
#include <map>


#define BOOSTLIB 0
#define OPTIMISD 0
#define VDSPOPTM 0
#define GESTLEARNT 8


#if BOOSTLIB
#include <boost/random.hpp>
#endif





// Recognizes gesture and tracks the variations. A set of gesture templates
// must be recorded. Then, at each new observation, the algorithm estimates
// which gesture is performed and adapts a set fo features (gesture variations)
// that are used as invariants for the recognition and gives continuous output
// parameters (e.g. for interaction)
//
// typical use:
//   GestureVariationFollower *myGVF = new GestureVariationFollower(...)
//   myGVF->addTemplate();
//   myGVF->fillTemplate();
//   myGVF->addTemplate();
//   myGVF->fillTemplate();
//   ...
//   myGVF->infer();
//   myGVF->getEstimatedStatus();
class GestureVariationFollower
{
private:
    

	Eigen::MatrixXf X;          // each row is a particle
	Eigen::VectorXi g;          // gesture index for each particle [g is ns x 1]
	Eigen::VectorXf w;          // weight of each particle [w is ns x 1]
    Eigen::VectorXf logW;       // non-normalized weights
    Eigen::VectorXf offS;       // translation offset
	Eigen::VectorXf sigt;       // vector of variances
	Eigen::VectorXf means;      // vector of means for particles initial spreading
	Eigen::VectorXf ranges;     // vector of ranges around the means for particles initial spreading
	std::vector<std::pair<Eigen::MatrixXf, Eigen::MatrixXf> > R_multi;  // gesture references (several examples)
	std::map<int,std::vector<std::vector<float> > > R_single;       // gesture references (1 example)
	float icov_single;          // inverse covariance (coeff. for the diagonal matrix)
	float nu;                   // degree of freedom for the t-distribution; if 0, use a gaussian
	float sp, sv, sr, ss;       // sigma values (actually, their square root)
	int resampling_threshold;   // resampling threshol
    int ns;
	int pdim;                   // number of state dimension
    int pdim_m1;
	int lrndGstr;               // number of learned gestures (starts at 0)
	std::vector<int> gestureLengths;   // length of each reference gesture
    std::string input_type;
    std::vector<float> particle_before_0;
    std::vector<float> particle_after_1;
	
	
    // random number generator
#if BOOSTLIB
	boost::variate_generator<boost::mt19937&, boost::normal_distribution<float> > *rndnorm(rng, normdist);
	boost::mt19937 rng;
	boost::normal_distribution<float> normdist;
#else
    std::tr1::mt19937 rng;
    std::tr1::normal_distribution<float> *normdist;
    std::tr1::uniform_real<float> *unifdist;
	std::tr1::variate_generator<std::tr1::mt19937, std::tr1::normal_distribution<float> > *rndnorm;//(rng, *normdist);
#endif
    
    
	// private functions
    void initweights();                         // initialize weights
    
	

    
	
	
	
public:
	
    
	// constructor of the gvf instance
	GestureVariationFollower(int ns, Eigen::VectorXf sigs, float icov, int resThresh, float nu = 0.);
	
	// destructor
    ~GestureVariationFollower();
	
	// add template to the vocabulary
	void addTemplate();
	
	// fill tempalte given by id with data vector
	void fillTemplate(int id, std::vector<float> data);

    // clear template given by id
    void clearTemplate(int id);
    
	// clear the templates
	void clear();
    
	// spread particles
	void spreadParticles(Eigen::VectorXf meanPVRS, Eigen::VectorXf rangePVRS);
    
	// inference
    void particleFilter(std::vector<float> obs);    // core algorithm but DEPRECATED
    void particleFilterOptim(std::vector<float> obs);
	
    // resample particles according to the proba distrib given by the weights
    void resampleAccordingToWeights();
	
    // makes the inference by calling particleFilteringOptim
	void infer(std::vector<float> vect);    // -- DEPRECATED --
    
    
	
	// Gets
	/////////
	float getObservationNoiseStd();
    int getResamplingThreshold();
	int getNbOfParticles();
	int getNbOfTemplates();
	int getLengthOfTemplateByInd(int Ind);
    std::vector<std::vector<float> > getTemplateByInd(int Ind);
    Eigen::MatrixXf getX();
	Eigen::VectorXi getG();
	Eigen::VectorXf getW();
    Eigen::VectorXf getGestureConditionnalProbabilities();
    Eigen::VectorXf getGestureLikelihoods();	// -- DEPRECATED --
	Eigen::VectorXf getEndGestureProbabilities(float minpos=0.);// -- DEPRECATED --
	Eigen::MatrixXf getEstimatedStatus();
	
    
    // Sets
	////////
	void setNumberOfParticles(int newNs);
	void setIcovSingleValue(float f);
	void setAdaptSpeed(std::vector<float> as);
	void setResamplingThreshold(int r);
    
	// Misc
	////////
	void saveTemplates(std::string filename);
    void loadTemplates(std::string filename);
	
    
    //    void testSetup(int ge);
    //    void testIt(int cur_dom_gest);
    //    int testGestureIdx;
    //    struct costTest{
    //        int idx;
    //        std::pair<int,int> transition;
    //        int tranPoint;
    //    };
    //    int testIdx;
    //    int realGest;
    //    costTest gest;
    //    std::vector<int> testScore;
    
    //in order to output particles
    std::vector<std::vector<float> > particlesPositions;
    
    
	// Segmentation variables
	std::vector<float> abs_weights;
	double probThresh;
    double probThreshMin;
    int currentGest;
    bool compa;
    float old_max;
    Eigen::VectorXf meanPVRScopy;
    Eigen::VectorXf rangePVRScopy;
    std::vector<float> origin;
    std::vector<float> *offset;
    bool new_gest;
    void setInitCoord(std::vector<float> s_origin);
    
    Eigen::VectorXf obs_eigen;
    Eigen::VectorXf vref;
    Eigen::VectorXf vrefmineigen;
    
};

#endif

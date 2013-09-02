/****************************************************************
 
 The GFPF library (GFPF.cpp, GFPF.hpp) has been created in 2010-2011 at Ircam Centre Pompidou by
 - Baptiste Caramiaux
 previously with Ircam Centre Pompidou and University Paris VI, since 2012 with Goldsmiths College, University of London
 - Nicola Montecchio
 previously with University of Padova, since 2012 with The Echo Nest
 The library is maintained by Baptiste Caramiaux at Goldsmiths College, University of London
 
 © Baptiste Caramiaux, Nicola Montecchio - STMS lab Ircam-CRNS-UPMC, University of Padova
 
 contact: b.caramiaux@gold.ac.uk
 ****************************************************************/


#ifndef GFPF_H
#define GFPF_H

#include <vector>
#include <Eigen/Core>
#include <boost/random.hpp>
#include <tr1/random>
#include <map>
//#include "executiontimer.h"
#include "filewriter.h"


#define BOOSTLIB 0
#define OPTIMISD 0 
#define VDSPOPTM 0

#define GESTLEARNT 8

// note:
// code is a bit dirty due to the multivar stuff (i.e. when multivar is true, use a different covariance
// matrix for each point, otherwise use the value icov_single); could be rewritten better with a class
// for multiple covariance inheriting from the base one (single value).

class gfpf
{
private:

	// algorithm variables
	Eigen::MatrixXf X;          // each row is a particle
	Eigen::VectorXi g;          // gesture index for each particle [g is ns x 1]
	Eigen::VectorXf w;          // weight of each particle [w is ns x 1]
    Eigen::VectorXf logW;       // non-normalized weights
    Eigen::VectorXf offS;     // translation offset
	Eigen::VectorXf sigt;       // vector of variances
	Eigen::VectorXf means;      // vector of means for particles initial spreading
	Eigen::VectorXf ranges;     // vector of ranges around the means for particles initial spreading
	std::vector<std::pair<Eigen::MatrixXf, Eigen::MatrixXf> > R_multi;  // gesture references (several examples)
	std::map<int,std::vector<std::vector<float> > > R_single;       // gesture references (1 example)
	float icov_single;          // inverse covariance (coeff. for the diagonal matrix)
	float nu;                   // degree of freedom for the t-distribution; if 0, use a gaussian
	float sp, sv, sr, ss;       // sigma values (actually, their square root)
	int resampling_threshold;   // resampling threshol
	int pdim;                   // number of state dimension
    int pdim_m1;
	int lrndGstr;               // number of learned gestures (starts at 0)
	bool multivar;              // -- DEPREC. --
	std::vector<int> gestureLengths;   // length of each reference gesture
	
    // random number generator
#if BOOSTLIB
	boost::mt19937 rng;
	boost::normal_distribution<float> normdist;
#else 
    std::tr1::mt19937 rng;
    std::tr1::normal_distribution<float> *normdist;
    std::tr1::uniform_real<float> *unifdist;
#endif
    
    // private functions
    void initweights();                         // initialize weights
    
    
	
public:
	// constructor of the gfpf instance
	gfpf(int ns, Eigen::VectorXf sigs, float icov, int resThresh, float nu = 0.);
    ~gfpf();
	void addTemplate();
	void fillTemplate(int id, std::vector<float> data);
	void clear();

	// spread particles
	void spreadParticles(Eigen::VectorXf meanPVRS, Eigen::VectorXf rangePVRS);
    
    // resample particles according to the proba distrib given by the weights
    void resampleAccordingToWeights();
	
    // inference
    void particleFilter(std::vector<float> obs);    // core algorithm
    void particleFilterOptim(std::vector<float> obs);
    
	void infer(std::vector<float> vect);    // -- DEPREC. -- 
    
    std::vector<float> inferGestureActivity();
    float inferTotalGestureActivity();
    std::vector<float> abs_weights;
    void writeGesturesToFile();

    
	// Gets
    Eigen::VectorXf getGestureConditionnalProbabilities();
    Eigen::VectorXf getGestureLikelihoods();
	Eigen::VectorXf getEndGestureProbabilities(float minpos=0.);
	Eigen::MatrixXf getEstimatedStatus();
    int getResamplingThreshold();
	int getNbOfParticles();
	int getNbOfTemplates();
	int getLengthOfTemplateByInd(int Ind);
    std::vector<std::vector<float> > getTemplateByInd(int Ind);
    
    
    // Sets
	void setIcovSingleValue(float f);
	void setAdaptSpeed(std::vector<float> as);
	void setResamplingThreshold(int r);
    
    //optims
    void setObsDimension(int s_d);
    int obs_dim;
    Eigen::VectorXf obs_eigen;
    Eigen::VectorXf vref;
    Eigen::VectorXf vrefmineigen;
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::normal_distribution<float> > *rndnorm;//(rng, *normdist);
    std::vector<float> particle_before_0;
    std::vector<float> particle_after_1;

    filewriter *g1,*g2,*g3,*g1y,*g2y,*g3y,*ug,*ugy,*tot;
    filewriter *w1,*w2,*w3,*p1,*p2,*p3;
    filewriter *ph1,*ph2,*ph3,*rs;
    filewriter *phase,*phase2,*phase3;
    filewriter *switcher;
    
    filewriter **wAry;
    filewriter **gAryx,**gAryy;
    
    filewriter **pAry;
    int pValArray[GESTLEARNT];
    
    filewriter **phaseAry;
    filewriter *curGest;
    int currentGest;
    bool compa;
    float old_max;
    Eigen::VectorXf meanPVRScopy;
    Eigen::VectorXf rangePVRScopy;
	
    std::vector<float> origin;
    std::vector<float> *offset;
    bool new_gest;
    void setInitCoord(std::vector<float> s_origin);
    
    
    void testSetup(int ge);
    void testIt(int cur_dom_gest);
    int testGestureIdx;
    struct costTest{
        int idx;
        std::pair<int,int> transition;
        int tranPoint;
    };
    int testIdx;
    int realGest;
    costTest gest;
    std::vector<int> testScore;
    filewriter *testWrite;

};

#endif

/* Optim Notes
    
    rndnorm no longer created each time in particle filter
    same with obs_eigen, vref and subtraction matrix. big speedup
    removed temp vars
*/

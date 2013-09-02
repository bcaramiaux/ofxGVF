#ifndef GFPF_H
#define GFPF_H

#include <vector>
#include <Eigen/Core>
#include <boost/random.hpp>
#include <map>

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
	int lrndGstr;               // number of learned gestures (starts at 0)
	bool multivar;              // -- DEPREC. --
	std::vector<int> gestureLengths;   // length of each reference gesture
	
    // random number generator
	boost::mt19937 rng;	
	boost::normal_distribution<float> normdist;
	
    
    // private functions
    void initweights();                         // initialize weights
    
	
public:
	// constructor of the gfpf instance
	gfpf(int ns, Eigen::VectorXf sigs, float icov, int resThresh, float nu = 0.);
	
	void addTemplate();
	void fillTemplate(int id, std::vector<float> data);
	void clear();

	// spread particles
	void spreadParticles(Eigen::VectorXf meanPVRS, Eigen::VectorXf rangePVRS);
    
    // resample particles according to the proba distrib given by the weights
    void resampleAccordingToWeights();
	
    // inference
    	void particleFilter(std::vector<float> obs);    // core algorithm
	void infer(std::vector<float> vect);    // -- DEPREC. -- 
    
    
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

	
};

#endif

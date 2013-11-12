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
#include <tr1/random>
#include <map>
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
    
    // private variables
	vector<vector<float> >  X;                  // each row is a particle
	vector<int>             g;                  // gesture index for each particle [g is ns x 1]
	vector<float>           w;                  // weight of each particle [w is ns x 1]
    vector<float>           offS;               // translation offset
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
    
	map<int,vector<vector<float> > > R_single;   // gesture references (1 example)

    
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
    
    


	
	
	
public:
	
    
	// constructor of the gvf instance
	GestureVariationFollower(int inputDim);     // use default parameter values
    GestureVariationFollower(int inputDim, int ns, vector<float> featVariances, float tolerance, int resamplingThreshold, float nu = 0.);
	
	// destructor
    ~GestureVariationFollower();
	
	// add template to the vocabulary
	void addTemplate();
	
	// fill tempalte given by id with data vector
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
	int     getNbOfParticles();
	int     getNbOfTemplates();
	int     getLengthOfTemplateByInd(int Ind);
    vector< vector<float> > getTemplateByInd(int Ind);
    vector< vector<float> > getX();
	vector<int>    getG();
	vector<float>  getW();
    vector<float>  getGestureProbabilities();
    vector<float>  getGestureConditionnalProbabilities(); // ----- DEPRECATED ------
	vector< vector<float> > getEstimatedStatus();
    vector<float>  getFeatureVariances();
	
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
	
    
    //    void testSetup(int ge);
    //    void testIt(int cur_dom_gest);
    //    int testGestureIdx;
    //    struct costTest{
    //        int idx;
    //        pair<int,int> transition;
    //        int tranPoint;
    //    };
    //    int testIdx;
    //    int realGest;
    //    costTest gest;
    //    vector<int> testScore;
    
    //in order to output particles
    vector<vector<float> > particlesPositions;
    
    
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
	
	
	
	
	// =====================
	//
	// matrices and vectors
    //
	//
    
    // init matrix by allocating memory
    void initMatf(vector< vector<float> > &T, int rows, int cols)
    {
        T.resize(rows);
        for (int n=0;n<rows;n++)
            T[n].resize(cols);
    }
    // init matrix and copy values from another matrix
    void setMatf(vector< vector<float> > &T, vector<vector<float> > &M)
    {
        int rows = M.size();
        int cols = M[0].size();
        T.resize(rows);
        for (int n=0;n<rows;n++){
            T[n].resize(cols);
            for (int m=0;m<cols;m++)
                T[n][m]=M[n][m];
        }
    }
    // init matrix by allocating memory and fill with float f
    void setMatf(vector< vector<float> > &T, float f, int rows, int cols)
    {
        T.resize(rows);
        for (int n=0;n<rows;n++){
            T[n].resize(cols);
            for (int m=0;m<cols;m++)
                T[n][m]=f;
        }
    }
    // set matrix filled with float f
    void setMatf(vector< vector<float> > &T, float f)
    {
        for (int n=0;n<T.size();n++)
            for (int m=0;m<T[n].size();m++)
                T[n][m]=f;
    }

    void printMatf(vector< vector< float> > &M){
        for (int k=0; k<M.size(); k++){
            cout << k << ": ";
            for (int l=0; l<M[0].size(); l++)
                cout << M[k][l] << " ";
            cout << endl;
        }
        cout << endl;
    }
	
    void initVeci(vector<int> &T, int rows)
    {
        T.resize(rows);
    }
    void setVeci(vector<int> &T, vector<int> &V)
    {
        int rows = V.size();
        T.resize(rows);
        for (int n=0;n<rows;n++)
            T[n]=V[n];
    }
    void initVecf(vector<float> &T, int rows)
    {
        T.resize(rows);
    }
    
    void setVecf(vector<float> &T, vector<float> &V)
    {
        int rows = V.size();
        T.resize(rows);
        for (int n=0;n<rows;n++)
            T[n]=V[n];
    }
    void setVecf(vector<float> &T, float f, int rows)
    {
        T.resize(rows);
        for (int n=0;n<rows;n++)
            T[n]=f;
    }
    void setVecf(vector<float> &T, float f)
    {
        for (int n=0;n<T.size();n++)
            T[n]=f;
    }
    
    // TODO(Baptiste) bugged, to be fixed
    vector< vector<float> > dotMatf(vector< vector<float> > &M1, vector< vector<float> > &M2)
    {
        assert(M1[0].size() == M2.size()); // columns in M1 == rows in M2
        vector< vector<float> > dot;
        initMatf(dot, M1.size(), M2[0].size()); // rows in M1 x cols in M2
        for (int i=0;i<M1.size();i++)
        {
            for (int j=0;j<M2[i].size();j++)
            {
                dot[i][j] = 0.0f;
                for(int k=0;k<M1.size();k++)
                {
                    dot[i][j] += M1[i][k] * M2[k][j]; // ??? is this right ???
                }
                
            }
        }
        return dot;
    }
    vector< vector<float> > multiplyMatf(vector< vector<float> > &M1, float v)
    {
        vector< vector<float> > multiply;
        initMatf(multiply, M1.size(), M1[0].size());
        for (int i=0;i<M1.size();i++)
        {
            for (int j=0;j<M1[i].size();j++)
            {
                multiply[i][j] = M1[i][j] * v;
            }
        }
        return multiply;
    }
    vector< vector<float> > multiplyMatf(vector< vector<float> > &M1, vector< vector<float> > &M2)
    {
        assert(M1[0].size() == M2.size()); // columns in M1 == rows in M2
        vector< vector<float> > multiply;
        initMatf(multiply, M1.size(), M2[0].size()); // rows in M1 x cols in M2
        for (int i=0;i<M1.size();i++)
        {
            for (int j=0;j<M2[i].size();j++)
            {
                multiply[i][j] = 0.0f;
                for(int k=0;k<M1[0].size();k++)
                {
                    multiply[i][j] += M1[i][k] * M2[k][j]; // ??? is this right ???
                }
                
            }
        }
        return multiply;
        
    
    }
    
	float getMeanVecf(vector<float> &T)
    {
        float tSum = 0.0f;
        for (int n=0;n<T.size();n++)
        {
            tSum += T[n];
        }
        return tSum / (float)T.size();
    }
    
    
};

#endif

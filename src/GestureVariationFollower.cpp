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


#include "GestureVariationFollower.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tr1/memory>
#include <unistd.h>



using namespace Eigen;
using namespace std;









// Constructor of the class GestureVariationFollower
// This creates an object that is able to learn gesture template,
// to recognize in realtime the live gesture and to estimate the
// live gesture variations according to the template (e.g. scale, speed, ...)
//
// typical use
//   GestureVariationFollower *myGVF;
//   myGVF = new GestureVariationFollower(NS, Sigs, Icov, ResThresh, Nu)
//
// ns is the number of particles
// Sigs is the variance for each varying feature that has to be estimated (speed, scale, twisting angle)
//
// Note about the current implementation: it involves geometric features: phase, speed, scale, angle of rotation
//    that are meant to be used for 2-dimensional input shapes. For general N-dimensional input, the class will
//    only consider phase, speed, scaling
GestureVariationFollower::GestureVariationFollower(int ns, VectorXf sigs, float icov, int resThresh, float nu)
{
    
    // State dimension depends on the number of noise variances
	pdim = static_cast<int>(sigs.size());   // state space dimension is given by the number of std dev
	pdim_m1 = pdim-1;
	X = MatrixXf(ns,pdim);          // Matrix of NS particles
	g = VectorXi(ns);               // Vector of gesture class
	w = VectorXf(ns);               // Weights
    logW = VectorXf(ns);
    offS = VectorXf(ns);            // translation offsets
    
	
	sigt = VectorXf(pdim);          // Fill variances
	for (int k=0; k<pdim; k++)
		sigt(k)=sqrt(sigs(k));
	
    resampling_threshold = resThresh;   // Set resampling threshold (usually NS/2)
	this->nu = nu;                      // Set Student's distribution parameter Nu
	lrndGstr=-1;                        // Set num. of learned gesture to -1
	icov_single = icov;                 // inverse of the global tolerance (variance)
	gestureLengths = vector<int>();     // Vector of gesture lengths
    
    logW.setConstant(0.0);              // log of weights equal to 0
    
#if !BOOSTLIB
    normdist = new std::tr1::normal_distribution<float>();
    unifdist = new std::tr1::uniform_real<float>();
    rndnorm  = new std::tr1::variate_generator<std::tr1::mt19937, std::tr1::normal_distribution<float> >(rng, *normdist);
#endif
    
    
    // Variables used for segmentation -- experimental (research in progress)
    // TODO(baptiste)
    abs_weights = vector<float>();      // absolute weights used for segmentation
    currentGest = 0;
    new_gest = false;
    offset = new std::vector<float>(2); // offset that has to be updated
    compa = false;
    old_max = 0;
    input_type = "shape";
    probThresh = 0.02*ns;               // thresholds on the absolute weights for segmentation
    probThreshMin = 0.1*ns;
    
}




// Destructor of the class
GestureVariationFollower::~GestureVariationFollower()
{
#if !BOOSTLIB
    if(normdist != NULL)
        delete (normdist);
    if(unifdist != NULL)
        delete (unifdist);
#endif
}



// Add a template into the vocabulary. This method does not add the data but allocate
// the memory and increases the number of learned gesture
void GestureVariationFollower::addTemplate()
{
	lrndGstr++;                                         // increment the num. of learned gesture
	R_single[lrndGstr] = vector<vector<float> >();      // allocate the memory for the gesture's data
    gestureLengths.push_back(0);                        // add an element (0) in the gesture lengths table
    abs_weights.resize(lrndGstr+1);
    
}



// Fill the template given by the integer 'id' by appending the current data vector 'data'
// This example fills the template 1 with the live gesture data (stored in liveGesture)
// for (int k=0; k<SizeLiveGesture; k++)
//    myGVF->fillTemplate(1, liveGesture[k]);
void GestureVariationFollower::fillTemplate(int id, vector<float> data)
{
	if (id<=lrndGstr)
	{
		R_single[id].push_back(data);
		gestureLengths[id]=gestureLengths[id]+1;
	}
}



// Clear the internal data (templates)
void GestureVariationFollower::clear()
{
	R_single.clear();
	gestureLengths.clear();
	lrndGstr=-1;
}



// Spread the particles by sampling values from intervals given my their means and ranges.
// Note that the current implemented distribution for sampling the particles is the uniform distribution
void GestureVariationFollower::spreadParticles(Eigen::VectorXf meanPVRS, Eigen::VectorXf rangePVRS)
{
    
	// we copy the initial means and ranges to be able to restart the algorithm
    meanPVRScopy  = meanPVRS;
    rangePVRScopy = rangePVRS;

    // USE BOOST FOR UNIFORM DISTRIBUTION!!!!
    // deprecated class should use uniform_real_distribution
#if BOOSTLIB
	boost::uniform_real<float> ur(0,1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
	
    
    int ns = static_cast<int>(X.rows());
	unsigned int ngestures = lrndGstr+1;
	
    // Spread particles using a uniform distribution
	for(int i = 0; i < pdim; i++)
		for(int n = 0; n < ns; n++)
			X(n,i) = (rnduni() - 0.5) * rangePVRS(i) + meanPVRS(i);
    
    // Weights are also uniformly spread
    initweights();
    logW.setConstant(0.0);
	
    // Spread uniformly the gesture class among the particles
	for(int n = 0; n < ns; n++)
		g(n) = n % ngestures;
    
    // offsets are set to 0
    offS.setConstant(0.0);
    
}



// Initialialize the weights of the particles. The initial values of the weights is a
// unifrom weight over the particles
void GestureVariationFollower::initweights()
{
    int ns = static_cast<int>(w.size());
    w.setConstant(1.0/ns);
}



// Performs the inference based on a given new observation
// This is the core algorithm: does one step of inference using particle filtering
// DEPRECATED
void GestureVariationFollower::particleFilter(vector<float> obs)
{
    
#if BOOSTLIB
    boost::variate_generator<boost::mt19937&, boost::normal_distribution<float> > rndnorm(rng, normdist);
#else
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::normal_distribution<float> > rndnorm(rng, *normdist);
#endif
    
    // Number of particles
    int ns = static_cast<int>(X.rows());
    
    // Change obs to VectorXf
    obs_eigen=VectorXf(obs.size());
    for (int k=0; k< obs.size(); k++)
        obs_eigen(k)=obs[k];
    
    // particles outside
    vector<float> particle_before_0;
    int numb_particle_before_0 = 0;
    vector<float> particle_after_1;
    int numb_particle_after_1 = 0;
    
    // MAIN LOOP: same process for EACH particle (row n in X)
	for(int n = 0; n < ns; n++)
    {
        // Move the particle
        // Position respects a first order dynamic: p = p + v/L
		X(n,0) = X(n,0) + rndnorm() * sigt(0) + X(n,1)/gestureLengths[g(n)];
        
		// Move the other state elements according a gaussian noise
        // sigt vector of variances
		for (int l=1;l<pdim;l++)
			X(n,l) = X(n,l) + rndnorm() * sigt(l);
		
		VectorXf x_n = X.row(n);
		if(x_n(0) < 0)
        {
            w(n) = 0;        // can't observe a particle outside (0,1) range [this behaviour could be changed]
            particle_before_0.push_back(n);
            numb_particle_before_0 += 1;
            logW(n) = -INFINITY;
        }
        else if(x_n(0) > 1)
        {
            w(n) = 0;
            particle_after_1.push_back(n);
            numb_particle_after_1 += 1;
            logW(n) = -INFINITY;
        }
		else
        {
			int pgi = g(n); // gesture index for the particle
			int frameindex = min((int)(gestureLengths[pgi]-1),(int)(floor(x_n(0) * gestureLengths[pgi])));
            
            //
            VectorXf vref(obs.size());
            for (int os=0; os<obs.size(); os++)
                vref(os) = R_single[pgi][frameindex][os];
            
            
            // If incoming data is 2-dimensional: we assume that it is drawn shape!
            if (obs.size()==2){
                // scaling
                vref *= x_n(2);
                // rotation
                float alpha = x_n(3);
                Matrix2f rotmat;
                rotmat << cos(alpha), -sin(alpha), sin(alpha), cos(alpha);
                vref = rotmat * vref;
            }
            // If incoming data is 3-dimensional
            else if (obs.size()==3){
                // scaling
                vref *= x_n(2);
            }
            // observation likelihood and update weights
            float dist;
            dist = (vref-obs_eigen).dot(vref-obs_eigen) * icov_single;
            
            if(nu == 0.)    // Gaussian distribution
            {
                w(n)   *= exp(-dist);
                logW(n) += -dist;
            }
            else            // Student's distribution
            {
                w(n)   *= pow(dist/nu + 1,-nu/2-1);    // dimension is 2 .. pay attention if editing
                logW(n) += (-nu/2-1)*log(dist/nu + 1);
            }
            
        }
    }
    // TODO: here we should compute the "absolute likelihood" as log(w) before normalization
    // this absolute likelihood could be used as a raw criterion for segmentation
    // USE BOOST FOR UNIFORM DISTRIBUTION!!!!
    
#if BOOSTLIB
    boost::uniform_real<float> ur(0,1);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif

	// normalization - resampling
	w /= w.sum();
	float neff = 1./w.dot(w);
	if(neff<resampling_threshold)
    {
        resampleAccordingToWeights();
        initweights();
        
    }
	
}



// Performs the inference based on a given new observation. This is the core algorithm: does
// one step of inference using particle filtering. It is the optimized version of the
// function ParticleFilter(). Note that the inference is possible only if some templates
// have been learned beforehand
//
// The inferring values are the weights of each particle that represents a possible gesture,
// plus a possible configuration of the features (value of speec, scale,...)
void GestureVariationFollower::particleFilterOptim(std::vector<float> obs)
{
 
    // Number of particles
    int ns = static_cast<int>(X.rows());
    
    // TODO(baptiste) must be changed by not using Eigen anymore
    VectorXf obs_eigen(obs.size());
    for(int i=0; i <obs.size(); i++)
    {
        if(!new_gest)
        {
            obs_eigen(i)=obs[i];
        } else{
            if(obs.size() == 2)
            {
                obs_eigen(i)=obs[i]-(*offset)[i];
            } else {
                obs_eigen(i)=obs[i];
            }
        }
    }
    
    // particles outside the beginning (phase=0) or end (phase=1) of a gesture
    // must have a weight equals to 0
    int numb_particle_before_0 = 0;
    int numb_particle_after_1 = 0;
    
    // zero abs weights
    for(int i = 0 ; i < abs_weights.size(); i++){
        abs_weights[i] = 0.0;
    }
    
    // clear any previous information about the particles' positions
    // (this is used for possible visualization but not in the inference)
    particlesPositions.clear();
    
    
    // MAIN LOOP: same process for EACH particle (row n in X)
    for(int n = ns-1; n >= 0; --n)
    {
        
        // Move the particle
        // Position respects a first order dynamic: p = p + v/L
		//X(n,0) = X(n,0) + (*rndnorm)() * sigt(0) + X(n,1)/gestureLengths[g(n)];
        X(n,0) += (*rndnorm)() * sigt(0) + X(n,1)/gestureLengths[g(n)];
        
		// Move the other state elements according a gaussian noise
        // sigt vector of variances
        for(int l = pdim_m1; l>=1 ; --l)
			X(n,l) += (*rndnorm)() * sigt(l);
		VectorXf x_n = X.row(n);
        
        // can't observe a particle outside (0,1) range [this behaviour could be changed]
		if(x_n(0) < 0)
        {
            w(n) = 0;
            particle_before_0.push_back(n);
            numb_particle_before_0 += 1;
            logW(n) = -INFINITY;
        }
        else if(x_n(0) > 1)
        {
            w(n) = 0;
            particle_after_1.push_back(n);
            numb_particle_after_1 += 1;
            logW(n) = -INFINITY;
        }
        // Can observe a particle inside (0,1) range
		else
        {
            // gesture index for the particle
            int pgi = g(n);
            
            // given the phase between 0 and 1 (first value of the particle x),
            // return the index of the corresponding gesture, given by g(n)
            int frameindex = min((int)(gestureLengths[pgi]-1),(int)(floor(x_n(0) * gestureLengths[pgi])));
            
            // given the index, return the gesture template value at this index
            VectorXf vref(obs.size());
            for (int os=0; os<obs.size(); os++)
                vref(os) = R_single[pgi][frameindex][os];
            
            // temp vector for the positions
            std::vector<float> temp;
        
            
            // If incoming data is 2-dimensional: we estimate phase, speed, scale, angle
            if (obs.size()==2){
                // sca1ing
                vref *= x_n(2);
                // rotation
                float alpha = x_n(3);
                Matrix2f rotmat;
                rotmat << cos(alpha), -sin(alpha), sin(alpha), cos(alpha);
                vref = rotmat * vref;

                // put the positions into vector
                // [used for visualization]
                std::vector<float> temp;
                temp.push_back(vref[0]);
                temp.push_back(vref[1]);
                particlesPositions.push_back(temp);
                
            }
            // If incoming data is N-dimensional
            else if (obs.size()!=2){
                // scaling
                vref *= x_n(2);
                
                // put the positions into vector
                // [used for visualization]
                std::vector<float> temp;
                for (int ndi=0; ndi<obs.size(); ndi++)
                    temp.push_back(vref[ndi]);
                particlesPositions.push_back(temp);
                
            }
            
            // compute distance between estimation given the current particle values
            // and the incoming observation
            vrefmineigen = vref-obs_eigen;
            
            // observation likelihood and update weights
            float dist = vrefmineigen.dot(vrefmineigen) * icov_single;
            if(nu == 0.)    // Gaussian distribution
            {
                w(n)   *= exp(-dist);
                logW(n) += -dist;
                abs_weights[g(n)] += exp(-dist);
            }
            else            // Student's distribution
            {
                w(n)   *= pow(dist/nu + 1,-nu/2-1);    // dimension is 2 .. pay attention if editing]
                logW(n) += (-nu/2-1)*log(dist/nu + 1);
            }
        }
    }
    
    
    // USE BOOST FOR UNIFORM DISTRIBUTION!!!!
#if BOOSTLIB
    boost::uniform_real<float> ur(0,1);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
    
    // normalize weights and compute criterion for degeneracy
	w /= w.sum();
	float neff = 1./w.dot(w);
    

    
    // Try segmentation from here...
    
    // do naive maximum value
    float maxSoFar = abs_weights[0];
    currentGest = 1;
    for(int i = 1; i< abs_weights.size();i++)
    {
        if(abs_weights[i] > maxSoFar)
        {
            maxSoFar = abs_weights[i];
            currentGest = i+1;
        }
    }
    
    if(maxSoFar > probThreshMin && !compa)
    {
        old_max = maxSoFar;
        compa = true;
    }
    
    if(maxSoFar < probThresh && compa)
    {
        spreadParticles(meanPVRScopy, rangePVRScopy);
        new_gest = true;
        (*offset)[0] = obs[0];
        (*offset)[1] = obs[1];
        compa = false;
    }
    
    // ... to here.
    
    
    // avoid degeneracy (no particles active, i.e. weights = 0) by resampling
    // around the active particles
	if(neff<resampling_threshold)
    {
        resampleAccordingToWeights();
        initweights();
    }
    
    particle_before_0.clear();
    particle_after_1.clear();
    
}



// Resampling function. The function resamples the particles based on the weights.
// Particles with negligeable weights will be respread near the particles with non-
// neglieable weigths (which means the most likely estimation).
// This steps is important to avoid degeneracy problem
void GestureVariationFollower::resampleAccordingToWeights()
{
#if BOOSTLIB
    boost::uniform_real<float> ur(0,1);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
    
    int ns = static_cast<int>(w.rows());
    
    MatrixXf oldX = X;
    VectorXi oldG = g;
    VectorXf oldLogW = logW;
    VectorXf c(ns);
    
    c(0) = 0;
    for(int i = 1; i < ns; i++)
        c(i) = c(i-1) + w(i);
    int i = 0;
    float u0 = rnduni()/ns;
    int free_pool = 0;
    for (int j = 0; j < ns; j++)
    {
        float uj = u0 + (j + 0.) / ns;
        
        while (uj > c(i) && i < ns - 1){
            i++;
        }
        
        if(j < ns - free_pool){
            X.row(j) = oldX.row(i);
            g(j) = oldG(i);
            logW(j) = oldLogW(i);
        }
    }
    
}


// Set the initial coordinates
void GestureVariationFollower::setInitCoord(std::vector<float> s_origin)
{
    origin = s_origin;
}



// Step function is the function called outside for inference. It
// has been originally created to be able to infer on a new observation or
// a set of observation.
// ---- DEPRECATED -----
void GestureVariationFollower::infer(vector<float> vect)
{
    particleFilterOptim(vect);
}








////////////////////////////////////////////////////////////////
//
// GET FUNCTIONS to ACCESS to INTERNAL VALUES
//
////////////////////////////////////////////////////////////////



// Return the number of particles
int GestureVariationFollower::getNbOfParticles()
{
	return static_cast<int>(w.size());
}

// Return the number of templates in the vocabulary
int GestureVariationFollower::getNbOfTemplates()
{
	return static_cast<int>(gestureLengths.size());
}

// Return the template given by its index in the vocabulary
vector<vector<float> > GestureVariationFollower::getTemplateByInd(int Ind)
{
	if (Ind < gestureLengths.size())
		return R_single[Ind];
	else
		return vector<vector<float> > ();
}

// Return the length of a specific template given by its index
// in the vocabulary
int GestureVariationFollower::getLengthOfTemplateByInd(int Ind)
{
	if (Ind < gestureLengths.size())
		return gestureLengths[Ind];
	else
		return -1;
}

// Return the resampling threshold used to avoid degeneracy problem
int GestureVariationFollower::getResamplingThreshold()
{
    return resampling_threshold;
}

// Return the standard deviation of the observation likelihood
float GestureVariationFollower::getObservationNoiseStd(){
    return sqrt(1/icov_single);
}

// Return the particle data (each row is a particle)
Eigen::MatrixXf GestureVariationFollower::getX()
{
    return X;
}

// Return the gesture index for each particle
Eigen::VectorXi GestureVariationFollower::getG()
{
    return g;
}

// Return particles' weights
Eigen::VectorXf GestureVariationFollower::getW()
{
    return w;
}


// Returns the probabilities of each gesture. This probability is conditionnal
// because it depends on the other gestures in the vocabulary:
// probability to be in gesture A knowing that we have gesture A, B, C, ... in the vocabulary
VectorXf GestureVariationFollower::getGestureConditionnalProbabilities()
{
	unsigned int ngestures = lrndGstr+1;
	int ns = static_cast<int>(X.rows());
	VectorXf gp(ngestures);
	gp.setConstant(0);
	for(int n = 0; n < ns; n++)
		gp(g(n)) += w(n);
    
	return gp;
}



// Returns likelihoods
// ----- DEPRECATED -------
VectorXf GestureVariationFollower::getGestureLikelihoods()
{
	unsigned int ngestures = lrndGstr+1;
    VectorXi numg(ngestures);
    for(int n = 0; n < ngestures; n++)
        numg(n)=0;
	int ns = static_cast<int>(X.rows());
	VectorXf gp(ngestures);
	gp.setConstant(0);
	for(int n = 0; n < ns; n++)
        if (logW(n) > -INFINITY)
        {
            gp(g(n))   += logW(n);
            numg(g(n)) += 1;
        }
    for(int n = 0; n < ngestures; n++)
    {
        if (numg(n) == 0)
            gp(n) = -INFINITY;
        else
            gp(n) = gp(n)/numg(n);
    }
    
    
	return gp;
}



// Returns the gesture probaiblities at the end of the gestures
// ----- DEPRECATED -------
VectorXf GestureVariationFollower::getEndGestureProbabilities(float minpos)
{
	
	unsigned int ngestures = lrndGstr+1;
	
	int ns = static_cast<int>(X.rows());
	
	VectorXf gp(ngestures);
	gp.setConstant(0);
	
	for(int n = 0; n < ns; n++)
		if(X(n,0) > minpos)
			gp(g(n)) += w(n);
	
	
	return gp;
	
}



// Returns the estimates features. It calls status to refer to the status of the state
// space which comprises the features to be adapted. If features are phase, speed, scale and angle,
// the function will return these estimateed features for each gesture, plus their probabilities.
// The returned matrix is nxm
//   rows correspond to the gestures in the vocabulary
//   cols correspond to the features (the last column is the [conditionnal] probability of each gesture)
// The output matrix is an Eigen matrix
MatrixXf GestureVariationFollower::getEstimatedStatus()
{
	
    // get the number of gestures in the vocabulary
	unsigned int ngestures = lrndGstr+1;
	
    // get the number of particles
	int ns = static_cast<int>(X.rows());
	
    // create the matrix (num of gestures x num of features) to be returned
    // and initialize
	MatrixXf es(ngestures,pdim+1);
	es.setConstant(0);
	
	// compute the estimated features by computing the expected values
    // sum ( feature values * weights)
	for(int n = 0; n < ns; n++)
	{
		int gi = g(n);
		es.block(gi,0,1,pdim) += X.row(n) * w(n);
		es(gi,pdim) += w(n);
    }
	
	for(int gi = 0; gi < ngestures; gi++)
	{
		es.block(gi,0,1,pdim) /= es(gi,pdim);
	}
    
	return es;
}





////////////////////////////////////////////////////////////////
//
// SET FUNCTIONS to UPDATE INTERNAL VALUES
//
////////////////////////////////////////////////////////////////

// Update the number of particles
void GestureVariationFollower::setNumberOfParticles(int newNs)
{
    particlesPositions.clear();
	X = MatrixXf(newNs,pdim);          // Matrix of NS particles
	g = VectorXi(newNs);               // Vector of gesture class
	w = VectorXf(newNs);               // Weights
    logW = VectorXf(newNs);
}

// Update the standard deviation of the observation distribution
// this value acts as a tolerance for the algorithm
// low value: less tolerant so more precise but can diverge
// high value: more tolerant so less precise but converge more easily
void GestureVariationFollower::setIcovSingleValue(float f)
{
	icov_single = f > 0 ? f : icov_single;
}

// Update the variance for each features which control their precision
// and speed of convergence
void GestureVariationFollower::setAdaptSpeed(vector<float> as)
{
	
	if (as.size() == pdim)
	{
		for (int k=0; k<pdim; k++)
			sigt(k)=sqrt(as[k]);
	}
	
}

// Update the resampling threshold used to avoid degeneracy problem
void GestureVariationFollower::setResamplingThreshold(int r)
{
    resampling_threshold = r;
}










////////////////////////////////////////////////////////////////
//
// MISC FUNCTIONS
//
////////////////////////////////////////////////////////////////




// Save function. This function is used by applications to save the
// vocabulary in a text file given by filename (filename is also the complete path + filename)
void GestureVariationFollower::saveTemplates(std::string filename)
{
    
    std::string directory = filename;
    
    std::ofstream file_write(directory.c_str());
    for(int i=0; i<R_single.size(); i++){
        file_write << "template " << i << " " << R_single[0][0].size() << endl;
        for(int j=0; j<R_single[i].size(); j++)
        {
            for(int k=0; k<R_single[i][j].size(); k++)
                file_write << R_single[i][j][k] << " ";
            file_write << endl;
        }
    }
    file_write.close();
    //values.clear();
    
}


// Load function. This function is used by applications to load a vocabulary
// given by filename (filename is also the complete path + filename)
void GestureVariationFollower::loadTemplates(std::string filename)
{
    
    clear();

    ifstream infile;
    stringstream doung;

    infile.open (filename.c_str(), ifstream::in);
    
    string line;
    vector<string> list;
    int cl=-1;
    while(!infile.eof())
    {
        cl++;
        infile >> line;
        //post("%i %s",cl,line.c_str());
        list.push_back(line);
    }
    
    int k=0;
    int template_starting_point = 1;
    int template_id=-1;
    int template_dim = 0;
    vector<float> vect_0_l;
    //post("list size %i",list.size());
    
    while (k < (list.size()-1) ){ // TODO to be changed if dim>2
        if (!strcmp(list[k].c_str(),"template"))
        {
            template_id = atoi(list[k+1].c_str());
            template_dim = atoi(list[k+2].c_str());
            k=k+3;
            //post("add template %i with size %i (k=%i)", template_id, template_dim,k);
            addTemplate();
            template_starting_point = 1;
        }
        
        if (template_dim<=0){
            //post("bug dim = -1");
        }
        else{          
            
            vector<float> vect(template_dim);
            if (template_starting_point==1)
            {
                // keep track of the first point
                for (int kk=0; kk<template_dim; kk++)
                {
                    vect[kk] = (float)atof(list[k+kk].c_str());
                }
                vect_0_l = vect;
                template_starting_point=0;
            }
            // store the incoming list as a vector of float
            for (int kk=0; kk<template_dim; kk++)
            {
                vect[kk] = (float)atof(list[k+kk].c_str());
                vect[kk] = vect[kk]-vect_0_l[kk];
            }
            //post("fill %i with %f %f",lrndGstr,vect[0],vect[1]);
            fillTemplate(lrndGstr,vect);
        }

        k+=template_dim;
        
    }
    
    infile.close();
    
}

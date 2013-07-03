/*
 *  gfpf.cpp
 *
 
 Realtime gesture recognition using particle filtering
 Required Eigen (header-only) Library, and Boost
 
 Copyright (C) 2013 Baptiste Caramiaux //TEST
 
 ...
 
 *
 */


#include "gfpf.h"
#include <iostream>
#include <tr1/memory>

/*  optimisations
 
    remove all pass by copy - replace with pass by ref
 
 
 */


using namespace Eigen;
using namespace std;

// Constructor of gfpf
//   arguments:
//      ns          number of particles
//      sigs        table of sigmas for each adapted parameters
//      icov        static std dev for observation likelihood
//      resThresh   threshold for resampling
//      nu          t-dist parameter
// ============================================================
gfpf::gfpf(int ns, VectorXf sigs, float icov, int resThresh, float nu)
{
    // State dimension depends on the number of noise variances
	pdim = sigs.size();
	pdim_m1 = pdim-1;
	X = MatrixXf(ns,pdim);          // Matrix of NS particles
	g = VectorXi(ns);               // Vector of gesture class
	w = VectorXf(ns);               // Weights
    logW = VectorXf(ns);
	
	sigt = VectorXf(pdim);          // Fill variances
	for (int k=0; k<pdim; k++)
		sigt(k)=sqrt(sigs(k));
	
    resampling_threshold = resThresh; // Set resampling threshold (usually NS/2)
	this->nu = nu;                  // Set Student's distribution parameter Nu
                                    // ^ init'd to 0
	lrndGstr=-1;                    // Set num. of learned gesture to -1
	icov_single = icov;             // inverse of the global tolerance (variance)
	gestureLengths = vector<int>(); // Vector of gesture lengths
	multivar = false; // -- DEPRECATED --
    logW.setConstant(0.0);
    
#if !BOOSTLIB
    normdist = new std::tr1::normal_distribution<float>();
    unifdist = new std::tr1::uniform_real<float>();
    rndnorm  = new std::tr1::variate_generator<std::tr1::mt19937, std::tr1::normal_distribution<float> >(rng, *normdist);
#endif
    
    // default resize to 2 min
    setObsDimension(2);
    
    abs_weights = vector<float>();
    
    g1 = new filewriter("g1");
    g2 = new filewriter("g2");
    tot = new filewriter("tot");
    ug = new filewriter("ug");

    
}

gfpf::~gfpf()
{
    
#if !BOOSTLIB
    if(normdist != NULL)
        delete (normdist);
    if(unifdist != NULL)
        delete (unifdist);
#endif
}


// addTemplate
//
// add a template to the database by allocating the memory
// needs to be called before the fillTemplate() method
// ============================================================
void gfpf::addTemplate()
{
	lrndGstr++;                                         // increment the num. of learned gesture
	R_single[lrndGstr] = vector<vector<float> >(); // allocate the memory for the gesture's data
	gestureLengths.push_back(0);                        // add an element (0) in the gesture lengths table
    abs_weights.resize(lrndGstr+1);

}

void gfpf::writeGesturesToFile()
{
    for(int i=0; i<R_single[0].size(); i++)
        g1->addValue(R_single[0][i][0]);
    for(int i=0; i<R_single[1].size(); i++)
        g2->addValue(R_single[1][i][0]);

    g1->writeFile();
    g2->writeFile();
    ug->writeFile();
    tot->writeFile();
    
}

// fillTemplate
//
// fill the template given by the integer 'id'
// with the current data vector 'data'
// ============================================================
void gfpf::fillTemplate(int id, vector<float> data)
{
	if (id<=lrndGstr)
	{
		R_single[id].push_back(data);
		gestureLengths[id]=gestureLengths[id]+1;
	}
}



// spreadParticles
//
// spread particles by sampling values from given intervals
// the current implemented distribution is uniform
//
//   arguments:
//      meanPVRS    mean values around which the particles are sampled
//      rangePVRS   range values defining how far from the means the particles can be spread
// ============================================================
void gfpf::spreadParticles(Eigen::VectorXf meanPVRS, Eigen::VectorXf rangePVRS)
{
	
    // USE BOOST FOR UNIFORM DISTRIBUTION!!!!
    // deprecated class should use uniform_real_distribution
#if BOOSTLIB
	boost::uniform_real<float> ur(0,1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else 
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
	int ns = X.rows();
	
	unsigned int ngestures = lrndGstr+1;
	
    // We keep track of the means/ranges by setting them as class attributes
	means  = meanPVRS;
	ranges = rangePVRS;
	
    // Spread particles using a uniform distribution
	for(int i = 0; i < pdim; i++)
		for(int n = 0; n < ns; n++)
			X(n,i) = (rnduni() - 0.5) * rangePVRS(i) + meanPVRS(i);
    
    // Weights are also uniformly spread
    initweights();
    logW.setConstant(0.0);
    //post("test logW to 0");
	
    // Spread uniformly the gesture class among the particles
	for(int n = 0; n < ns; n++)
		g(n) = n % ngestures;
}



// initWeights
//
// ============================================================
void gfpf::initweights()
{
    int ns = w.size();
    w.setConstant(1.0/ns);
    //post("%i %f %f %f", ns, w(0), w(1), w(2));
    //logW.setConstant(0.0);
}



// particleFilter
//
// Core algorithm: does one step of inference using particle filtering
//
//   arguments:
//      xy      current data vector
// ============================================================
void gfpf::particleFilter(vector<float> obs)
{
    
#if BOOSTLIB
    boost::variate_generator<boost::mt19937&, boost::normal_distribution<float> > rndnorm(rng, normdist);
#else 
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::normal_distribution<float> > rndnorm(rng, *normdist);
#endif
   
    
    // Number of particles
    int ns = X.rows();
    
    // Change obs to VectorXf
    VectorXf obs_eigen(obs.size());
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
    

    for (int n=0; n<particle_before_0.size(); n++)
    {
        // Spread particles using a uniform distribution
        for(int i = 0; i < pdim; i++)
                X(particle_before_0[n],i) = (rnduni() - 0.5) * ranges(i) + means(i);
        w(particle_before_0[n]) = 1.0/(ns);
        g(particle_before_0[n]) = n % (lrndGstr+1);
    }
    for (int n=0; n<particle_after_1.size(); n++)
    {
        // Spread particles using a uniform distribution
        for(int i = 0; i < pdim; i++)
            X(particle_after_1[n],i) = (rnduni() - 0.5) * ranges(i) + means(i);
        w(particle_after_1[n]) = 1.0/(ns);
        g(particle_after_1[n]) = n % (lrndGstr+1);
    }
    
	// normalization - resampling
	w /= w.sum();
	float neff = 1./w.dot(w);
	if(neff<resampling_threshold)
    {
        resampleAccordingToWeights();
        initweights();
    }
	
}

void gfpf::setObsDimension(int s_d)
{
    obs_dim = s_d;
    obs_eigen.resize(obs_dim);
    vref.resize(obs_dim);
    vrefmineigen.resize(obs_dim);
    
}
// test

void gfpf::particleFilterOptim(std::vector<float> obs)
{
    // Number of particles
    int ns = X.rows();
    ug->addValue(obs[0]);

    for(int k=obs_dim-1; k >= 0; --k)
        obs_eigen(k)=obs[k];
    
    // particles outside
    int numb_particle_before_0 = 0;
    int numb_particle_after_1 = 0;
    
    // zero abs weights
    for(int i = 0 ; i < abs_weights.size(); i++){
        abs_weights[i] = 0.0;
    }
    
    //executiontimer ex("loop");
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
		// if not eigen, could use SSE/ vec operations
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
            //make this local var common
            //vref(obs.size());
            for (int os=0; os<obs.size(); os++)
                vref(os) = R_single[pgi][frameindex][os];
            
            
            // If incoming data is 2-dimensional: we assume that it is drawn shape!
            if (obs.size()==2){
                // scal1ing
                vref *= x_n(2);
                // rotation
                float alpha = x_n(3);
                Matrix2f rotmat;
                rotmat << cos(alpha), -sin(alpha), sin(alpha), cos(alpha);
                // tvref = vref * rotmat;
                
                
            }
            // If incoming data is 3-dimensional
            else if (obs.size()==3){
                // scaling
                vref *= x_n(2);
            }
            vrefmineigen = vref-obs_eigen;
            
            
            // observation likelihood and update weights
            // vref-obs_eigen will be costly as special data type
            float dist = vrefmineigen.dot(vrefmineigen) * icov_single;
            if(nu == 0.)    // Gaussian distribution
            {
                w(n)   *= exp(-dist);
                logW(n) += -dist;
            }
            else            // Student's distribution
            {
                w(n)   *= pow(dist/nu + 1,-nu/2-1);    // dimension is 2 .. pay attention if editing]
                logW(n) += (-nu/2-1)*log(dist/nu + 1);
            }
        }
        abs_weights[g(n)] += w(n);


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
    
    
    for (int n=0; n<particle_before_0.size(); n++)
    {
        // Spread particles using a uniform distribution
        for(int i = 0; i < pdim; i++)
            X(particle_before_0[n],i) = (rnduni() - 0.5) * ranges(i) + means(i);
        w(particle_before_0[n]) = 1.0/(ns);
        g(particle_before_0[n]) = n % (lrndGstr+1);
    }
    for (int n=0; n<particle_after_1.size(); n++)
    {
        // Spread particles using a uniform distribution
        for(int i = 0; i < pdim; i++)
            X(particle_after_1[n],i) = (rnduni() - 0.5) * ranges(i) + means(i);
        w(particle_after_1[n]) = 1.0/(ns);
        g(particle_after_1[n]) = n % (lrndGstr+1);
    }
    
	// normalization - resampling
    // this is normalization. take absolute val here and output
    // set a threshold for 'this is gesture'
    
	w /= w.sum();
	float neff = 1./w.dot(w);
	if(neff<resampling_threshold)
    {
        resampleAccordingToWeights();
        initweights();
    }
    
    particle_before_0.clear();
    particle_after_1.clear();
    
    //ex.kill();
}

// inferGestureActivity
//
// use the unnormalized weights to determine if there
// is an active gesture
// ============================================================

std::vector<float> gfpf::inferGestureActivity()
{
    return abs_weights;
}

float gfpf::inferTotalGestureActivity()
{
    float total = 0.0;
    for(int i = 0 ; i < abs_weights.size(); i++)
        total += abs_weights[i];
    tot->addValue(total);
    return total;
}

// resampleAccordingToWeights
//
// ============================================================
void gfpf::resampleAccordingToWeights()
{
#if BOOSTLIB
    boost::uniform_real<float> ur(0,1);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif

    int ns = w.rows();
    
    MatrixXf oldX = X;
    VectorXi oldG = g;
    VectorXf oldLogW = logW;
    VectorXf c(ns);
    
    c(0) = 0;
    for(int i = 1; i < ns; i++)
        c(i) = c(i-1) + w(i);
    int i = 0;
    float u0 = rnduni()/ns;
    for (int j = 0; j < ns; j++)
    {
        float uj = u0 + (j + 0.) / ns;
        while (uj > c(i) && i < ns - 1)
            i++;
        X.row(j) = oldX.row(i);
        g(j) = oldG(i);
        logW(j) = oldLogW(i);
    }
}



// step
//
// run inference on the input dataset. Each row is a temporal observation
// for each row the incremental inference procedure is called
//
//   arguments:
//      xy      whole data matrix
// ============================================================
void gfpf::infer(vector<float> vect)
{
//	unsigned int nsteps = vect.size();
    //particleFilter(vect);
    particleFilterOptim(vect);
//	for(int i = 0; i < nsteps; i++)
//		particleFilter2D(xy.row(i));
}



// getGestureConditionnalProbabilities
//
// return values of each gesture's likelihood
// ============================================================
VectorXf gfpf::getGestureConditionnalProbabilities()
{
	unsigned int ngestures = lrndGstr+1;
	int ns = X.rows();
	VectorXf gp(ngestures);
	gp.setConstant(0);
	for(int n = 0; n < ns; n++)
		gp(g(n)) += w(n);
	return gp;
}



// getGestureLikelihoods
//
// return values of each gesture's likelihood
// ============================================================
VectorXf gfpf::getGestureLikelihoods()
{
	unsigned int ngestures = lrndGstr+1;
    VectorXi numg(ngestures);
    for(int n = 0; n < ngestures; n++)
        numg(n)=0;
	int ns = X.rows();
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



// getEndGestureProbabilities
//
// ============================================================
VectorXf gfpf::getEndGestureProbabilities(float minpos)
{
	
	unsigned int ngestures = lrndGstr+1;
	
	int ns = X.rows();
	
	VectorXf gp(ngestures);
	gp.setConstant(0);
	
	for(int n = 0; n < ns; n++)
		if(X(n,0) > minpos)
			gp(g(n)) += w(n);
	
	
	return gp;
	
}



// getEstimatedStatus()
//
// return values of estimated features
// ============================================================
MatrixXf gfpf::getEstimatedStatus()
{
	
	unsigned int ngestures = lrndGstr+1;
	
	//	post("getEstimatedStatus ngestures: %i (lrndGstr+1=%i)",ngestures,lrndGstr+1);
	
	int ns = X.rows();
	
	MatrixXf es(ngestures,pdim+1);     // PVRSW
	es.setConstant(0);
	
	
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



// setIcovSingleValue(...)
//
// return values of estimated features
// ============================================================
void gfpf::setIcovSingleValue(float f)
{
	icov_single = f > 0 ? f : icov_single;
}



// getNbOfParticles()
//
// ============================================================
int gfpf::getNbOfParticles()
{
	return w.size();
}



// getNbOfTemplates()
//
// ============================================================
int gfpf::getNbOfTemplates()
{
	return gestureLengths.size();
}



// getLengthOfTemplateByInd(...)
//
// ============================================================
int gfpf::getLengthOfTemplateByInd(int Ind)
{
	if (Ind < gestureLengths.size())
		return gestureLengths[Ind];
	else
		return -1;
}



// getTemplateByInd(...)
//
// ============================================================
vector<vector<float> > gfpf::getTemplateByInd(int Ind)
{
	if (Ind < gestureLengths.size())
		return R_single[Ind];
	else
		return vector<vector<float> > ();
}



// getResamplingThreshold()
//
// ============================================================
int gfpf::getResamplingThreshold()
{
    return resampling_threshold;
}



// setAdaptSpeed(...)
//
// ============================================================
void gfpf::setAdaptSpeed(vector<float> as)
{
	
	if (as.size() == pdim)
	{
		for (int k=0; k<pdim; k++)
			sigt(k)=sqrt(as[k]);
	}
	
}



// setResamplingThreshold(...)
//
// argument
//      r   resampling threshold (usually NS/2)
// ============================================================
void gfpf::setResamplingThreshold(int r)
{
    resampling_threshold = r;
}



// clear()
//
// Clear the data structure of the object gfpf
// ============================================================
void gfpf::clear()
{
	R_single.clear();
	gestureLengths.clear();
	lrndGstr=-1;
}



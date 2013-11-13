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

#include "ofxGVF.h"

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tr1/memory>
#include <unistd.h>

using namespace std;

// Constructor of the class ofxGVF
// This creates an object that is able to learn gesture template,
// to recognize in realtime the live gesture and to estimate the
// live gesture variations according to the template (e.g. scale, speed, ...)
//
// typical use
//   ofxGVF *myGVF;
//   myGVF = new ofxGVF(NS, Sigs, Icov, ResThresh, Nu)
//
// ns is the number of particles
// Sigs is the variance for each varying feature that has to be estimated (speed, scale, twisting angle)
//
// Note about the current implementation: it involves geometric features: phase, speed, scale, angle of rotation
//    that are meant to be used for 2-dimensional input shapes. For general N-dimensional input, the class will
//    only consider phase, speed, scaling
//ofxGVF::ofxGVF(int ns, VectorXf sigs, float icov, int resThresh, float nu)


ofxGVF::ofxGVF(){
    // nothing?
}

ofxGVF::ofxGVF(ofxGVFParameters parameters){
    setup(parameters);
}

void ofxGVF::setup(){
    
    // use defualt parameters
    // EXPERIMENTAL!!!
    
    ofxGVFParameters parameters;
    
    parameters.inputDimensions = 2;
    parameters.numberParticles = 2000;
    parameters.phaseVariance = 0.00001;
    parameters.speedVariance = 0.00001;
    parameters.scaleVariance = 0.00001;
    parameters.rotationVariance = 0.00001;
    parameters.tolerance = 0.2f;
    parameters.resamplingThreshold = 500;
    parameters.distribution = 0.0f;
    
    setup(parameters);
}

void ofxGVF::setup(ofxGVFParameters parameters){
    
    clear(); // just in case
    
    inputDim = parameters.inputDimensions;
    ns = parameters.numberParticles;
    
    if(inputDim > 2 && parameters.rotationVariance != 0.0){
        cout << "Warning rotation variance will not be considered for more than 2 input dimensions!" << endl;
        parameters.rotationVariance = 0.0f;
    }
    
    if(parameters.phaseVariance != 0.0f) featVariances.push_back(sqrt(parameters.phaseVariance));
    if(parameters.speedVariance != 0.0f) featVariances.push_back(sqrt(parameters.speedVariance));
    if(parameters.scaleVariance != 0.0f) featVariances.push_back(sqrt(parameters.scaleVariance));
    if(parameters.rotationVariance != 0.0f) featVariances.push_back(sqrt(parameters.rotationVariance));
    
    pdim = featVariances.size();
    
	initMat(X, ns, pdim);           // Matrix of NS particles
	initVec(g, ns);                 // Vector of gesture class
	initVec(w, ns);                 // Weights
    initMat(offS, ns, inputDim);    // Offsets
    
    resamplingThreshold = parameters.resamplingThreshold;   // Set resampling threshold (usually NS/2)
    tolerance = parameters.tolerance;                       // inverse of the global tolerance (variance)
    nu = parameters.distribution;                           // Set Student's distribution parameter Nu
    
    numTemplates=-1;                        // Set num. of learned gesture to -1
    gestureLengths = vector<int>();         // Vector of gesture lengths
    
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
    probThresh = 0.02*ns;               // thresholds on the absolute weights for segmentation
    probThreshMin = 0.1*ns;
}

// Destructor of the class
ofxGVF::~ofxGVF(){
#if !BOOSTLIB
    if(normdist != NULL)
        delete (normdist);
    if(unifdist != NULL)
        delete (unifdist);
#endif
    
    // should we free here other variables such X, ...??
    //TODO(baptiste)
    
    clear(); // not really necessary but it's polite ;)
    
}

// Add a template into the vocabulary. This method does not add the data but allocate
// the memory and increases the number of learned gesture
void ofxGVF::addTemplate(){
	numTemplates++;                                         // increment the num. of learned gesture
	R_single[numTemplates] = vector< vector<float> >();      // allocate the memory for the gesture's data
    gestureLengths.push_back(0);                        // add an element (0) in the gesture lengths table
    abs_weights.resize(numTemplates+1);
}

void ofxGVF::addTemplate(vector<float> & data){
	addTemplate();
    fillTemplate(getNumberOfTemplates(), data);
}

// Fill the template given by the integer 'id' by appending the current data vector 'data'
// This example fills the template 1 with the live gesture data (stored in liveGesture)
// for (int k=0; k<SizeLiveGesture; k++)
//    myGVF->fillTemplate(1, liveGesture[k]);
void ofxGVF::fillTemplate(int id, vector<float> & data){
	if (id <= numTemplates){
        
        // BAPTISTE: WHY ONLY DO THIS FOR 2D DATA????????
        if(data.size() == 2){
            
            // store initial point
            if(R_single[id].size() == 0){
                R_initial[id] = data;
            }
            
            // 'center' data
            for(int i = 0; i < data.size(); i++){
                data[i] -= R_initial[id][i];
            }
        }

		R_single[id].push_back(data);
		gestureLengths[id] = gestureLengths[id]+1;
	}
}

// clear template given by id
void ofxGVF::clearTemplate(int id){
    if (id <= numTemplates){
        R_single[id] = vector< vector<float> >();      // allocate the memory for the gesture's data
        gestureLengths[id] = 0;                // add an element (0) in the gesture lengths table
    }
}

// Clear the internal data (templates)
void ofxGVF::clear(){
    state = STATE_CLEAR;
	R_single.clear();
    R_initial.clear();
    O_initial.clear();
	gestureLengths.clear();
	numTemplates=-1;
}

// Spread the particles by sampling values from intervals given my their means and ranges.
// Note that the current implemented distribution for sampling the particles is the uniform distribution
void ofxGVF::spreadParticles(vector<float> & means, vector<float> & ranges){
    
    O_initial.clear(); // clear initial observation data (used to 'center' 2D obs)
    
	// we copy the initial means and ranges to be able to restart the algorithm
    meansCopy  = means;
    rangesCopy = ranges;

    // USE BOOST FOR UNIFORM DISTRIBUTION!!!!
    // deprecated class should use uniform_real_distribution
#if BOOSTLIB
	boost::uniform_real<float> ur(0,1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
	
	unsigned int ngestures = numTemplates+1;
	
    // Spread particles using a uniform distribution
	for(int i = 0; i < pdim; i++)
		for(int n = 0; n < ns; n++)
			X[n][i] = (rnduni() - 0.5) * ranges[i] + means[i];
    
    // Weights are also uniformly spread
    initweights();
    //    logW.setConstant(0.0);
	
    // Spread uniformly the gesture class among the particles
	for(int n = 0; n < ns; n++){
		g[n] = n % ngestures;
    
        // offsets are set to 0
        for (int k=0; k<inputDim; k++)
            offS[n][k]=0.0;
    }
    
}

// Initialialize the weights of the particles. The initial values of the weights is a
// unifrom weight over the particles
void ofxGVF::initweights(){
    for (int k=0; k<ns; k++)
        w[k]=1.0/ns;
}

float distance_weightedEuclidean(vector<float> x, vector<float> y, vector<float> w){
    int count = x.size();
    // the size must be > 0
    if (count<=0)
        return 0;
    
    float dist=0;
    for(int k=0;k<count;k++){
        dist+=w[k]*pow((x[k]-y[k]),2);
        //post(" k=%i | %f %f %f",k,x[k],y[k],w[k]);
    }
    return dist;
}

// Performs the inference based on a given new observation. This is the core algorithm: does
// one step of inference using particle filtering. It is the optimized version of the
// function ParticleFilter(). Note that the inference is possible only if some templates
// have been learned beforehand
//
// The inferring values are the weights of each particle that represents a possible gesture,
// plus a possible configuration of the features (value of speec, scale,...)
void ofxGVF::particleFilter(vector<float> & obs){
    
//    post("%f %f", obs[0], obs[1]);
    // BAPTISTE: WHY ONLY DO THIS FOR 2D DATA????????
        
        
    if(obs.size() == 2){
        
        if(O_initial.size() == 0){ // then it's a new gesture observation - cleared in spreadParticles
        
            // store initial obs data
            O_initial = obs;
        
        }else{
            
            // 'center' data
            for(int i = 0; i < obs.size(); i++){
                obs[i] -= O_initial[i];
            }
        }
    }
    
#if BOOSTLIB
	boost::uniform_real<float> ur(0,1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
    
    // particles outside the beginning (phase=0) or end (phase=1) of a gesture
    // must have a weight equals to 0
    int numParticlesPhaseLt0 = 0;
    int numParticlesPhaseGt1 = 0;
    
    // zero abs weights
    for(int i = 0 ; i < getNumberOfTemplates(); i++){
        abs_weights[i] = 0.0;
    }
    
    // clear any previous information about the particles' positions
    // (this is used for possible visualization but not in the inference)
    particlesPositions.clear();

    float sumw=0.0;

    // MAIN LOOP: same process for EACH particle (row n in X)
    for(int n = ns-1; n >= 0; --n)
    {

        // Move the particle
        // Position respects a first order dynamic: p = p + v/L
		//X(n,0) = X(n,0) + (*rndnorm)() * featVariances(0) + X(n,1)/gestureLengths[g(n)];
        X[n][0] += (*rndnorm)() * featVariances[0] + X[n][1]/gestureLengths[g[n]];

        //post("n=%i g(n)=%i length=%i | %f %f",n,g[n],gestureLengths[g[n]],(*rndnorm)() * featVariances[0], X[n][1]/gestureLengths[g[n]]);
		// Move the other state elements according a gaussian noise
        // featVariances vector of variances
        for(int l = pdim-1; l>=1 ; --l)
			X[n][l] += (*rndnorm)() * featVariances[l];
		vector<float> x_n = X[n];
        
        
        // can't observe a particle outside (0,1) range [this behaviour could be changed]
		if(x_n[0] < 0)
        {
            //experimental!
            w[n] = 1/ns;
            // Spread particles using a uniform distribution
            for(int i = 0; i < pdim; i++)
                x_n[i] = (rnduni() - 0.5) * rangesCopy[i] + meansCopy[i];
            g[n] = n % gestureLengths.size();
            particlesPhaseLt0.push_back(n);
            numParticlesPhaseLt0 += 1;
            
            for (int k=0;k<inputDim;k++)
                offS[n][k] = obs[k];
        }
        else if(x_n[0] > 1)
        {

            //experimental!
            w[n] = 1/ns;
            // Spread particles using a uniform distribution
            for(int i = 0; i < pdim; i++)
                x_n[i] = (rnduni() - 0.5) * rangesCopy[i] + meansCopy[i];
            g[n] = n % gestureLengths.size();
            particlesPhaseGt1.push_back(n);
            numParticlesPhaseGt1 += 1;
            for (int k=0;k<inputDim;k++)
                offS[n][k] = obs[k];
        }
        // Can observe a particle inside (0,1) range
	//	else
    //    {
            // gesture index for the particle
            int pgi = g[n];
            
            // given the phase between 0 and 1 (first value of the particle x),
            // return the index of the corresponding gesture, given by g(n)
            int frameindex = min((int)(gestureLengths[pgi]-1),(int)(floor(x_n[0] * gestureLengths[pgi])));
            
            // given the index, return the gesture template value at this index
            vector<float> vref(inputDim);
            setVec(vref,R_single[pgi][frameindex]);

            vector<float> vobs(inputDim);
            setVec(vobs,obs);
        
            // If incoming data is 2-dimensional: we estimate phase, speed, scale, angle
            if (inputDim == 2){
                
                // offset!
                //for (int k=0;k<inputDim;k++)
                //    vobs[k] -= offS[n][k];
                
                // sca1ing
                for (int k=0;k<inputDim;k++)
                    vref[k] *= x_n[2];
                
                // rotation
                float alpha = x_n[3];
                float tmp0=vref[0]; float tmp1=vref[1];
                vref[0] = cos(alpha)*tmp0 - sin(alpha)*tmp1;
                vref[1] = sin(alpha)*tmp0 + cos(alpha)*tmp1;

                // put the positions into vector
                // [used for visualization]
                std::vector<float> temp;
                temp.push_back(vref[0]);
                temp.push_back(vref[1]);
                particlesPositions.push_back(temp);
                
            }
            // If incoming data is N-dimensional
            else if (inputDim != 2){
                
                // sca1ing
                for (int k=0;k<inputDim;k++)
                    vref[k] *= x_n[2];
                
                // put the positions into vector
                // [used for visualization]
                std::vector<float> temp;
                for (int ndi=0; ndi<inputDim; ndi++)
                    temp.push_back(vref[ndi]);
                particlesPositions.push_back(temp);
                
            }
            
            // compute distance between estimation given the current particle values
            // and the incoming observation
            
            // define weights here on the dimension if needed
            vector<float> dimWeights(inputDim);
            for(int k=0;k<inputDim;k++) dimWeights[k]=1.0/inputDim;
            
            // observation likelihood and update weights
            float dist = distance_weightedEuclidean(vref,obs,dimWeights) * 1/(tolerance*tolerance);
            
            
            if(nu == 0.)    // Gaussian distribution
            {
                w[n]   *= exp(-dist);
                abs_weights[g[n]] += exp(-dist);
            }
            else            // Student's distribution
            {
                w[n]   *= pow(dist/nu + 1,-nu/2-1);    // dimension is 2 .. pay attention if editing]
            }

      //  }
        sumw+=w[n];
    }
    
    // normalize weights and compute criterion for degeneracy
    //	w /= w.sum();
    //	float neff = 1./w.dot(w);
    float dotProdw=0.0;
    for (int k=0;k<ns;k++){
        w[k]/=sumw;
        dotProdw+=w[k]*w[k];
    }
    float neff = 1./dotProdw;
    //cout << neff << endl;
    // Try segmentation from here...
    
    // do naive maximum value
    /*
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
        spreadParticles(meansCopy, rangesCopy);
        new_gest = true;
        (*offset)[0] = obs[0];
        (*offset)[1] = obs[1];
        compa = false;
    }*/
    
    // ... to here.
    
    
    // avoid degeneracy (no particles active, i.e. weights = 0) by resampling
    // around the active particles
	if(neff<resamplingThreshold)
    {
            //cout << "Resampling" << endl;
        resampleAccordingToWeights();
        initweights();
    }
    
    particlesPhaseLt0.clear();
    particlesPhaseGt1.clear();
    
}

// Resampling function. The function resamples the particles based on the weights.
// Particles with negligeable weights will be respread near the particles with non-
// neglieable weigths (which means the most likely estimation).
// This steps is important to avoid degeneracy problem
void ofxGVF::resampleAccordingToWeights()
{
#if BOOSTLIB
    boost::uniform_real<float> ur(0,1);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnduni(rng, ur);
#else
    std::tr1::uniform_real<float> ur(0,1);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<float> > rnduni(rng, ur);
#endif
    
    vector< vector<float> > oldX;
    setMat(oldX,X);
    vector<int> oldG;
    setVec(oldG, g);
    vector<float> c(ns);
    
    c[0] = 0;
    for(int i = 1; i < ns; i++)
        c[i] = c[i-1] + w[i];
    int i = 0;
    float u0 = rnduni()/ns;
    int free_pool = 0;
    for (int j = 0; j < ns; j++)
    {
        float uj = u0 + (j + 0.) / ns;
        
        while (uj > c[i] && i < ns - 1){
            i++;
        }
        
        if(j < ns - free_pool){
            for (int kk=0;kk<X[0].size();kk++)
                X[j][kk] = oldX[i][kk];
            g[j] = oldG[i];
            //            logW(j) = oldLogW(i);
        }
    }
    
}

// Set the initial coordinates
void ofxGVF::setInitCoord(std::vector<float> s_origin){
    origin = s_origin;
}

// Step function is the function called outside for inference. It
// has been originally created to be able to infer on a new observation or
// a set of observation.
void ofxGVF::infer(vector<float> & vect){
    particleFilter(vect);
}

////////////////////////////////////////////////////////////////
//
// GET FUNCTIONS to ACCESS to INTERNAL VALUES
//
////////////////////////////////////////////////////////////////


// Return the number of particles
int ofxGVF::getNumberOfParticles(){
	return ns;
}

// Return the number of templates in the vocabulary
int ofxGVF::getNumberOfTemplates(){
	return gestureLengths.size();
}

// Return the template given by its index in the vocabulary
vector< vector<float> >& ofxGVF::getTemplateByIndex(int index){
	if (index < gestureLengths.size())
		return R_single[index];
	else
		return EmptyTemplate;
}

// Return the length of a specific template given by its index
// in the vocabulary
int ofxGVF::getLengthOfTemplateByIndex(int index){
	if (index < gestureLengths.size())
		return gestureLengths[index];
	else
		return -1;
}

// Return the resampling threshold used to avoid degeneracy problem
int ofxGVF::getResamplingThreshold(){
    return resamplingThreshold;
}

// Return the standard deviation of the observation likelihood
float ofxGVF::getObservationNoiseStd(){
    return tolerance;
}

// Return the particle data (each row is a particle)
vector< vector<float> > ofxGVF::getX(){
    return X;
}

// Return the gesture index for each particle
vector<int> ofxGVF::getG(){
    return g;
}

// Return particles' weights
vector<float> ofxGVF::getW(){
    return w;
}

vector< vector<float> >& ofxGVF::getParticlesPositions(){
    return particlesPositions;
}

// Returns the probabilities of each gesture. This probability is conditionnal
// because it depends on the other gestures in the vocabulary:
// probability to be in gesture A knowing that we have gesture A, B, C, ... in the vocabulary
vector<float> ofxGVF::getGestureProbabilities()
{
	unsigned int ngestures = numTemplates+1;
    
	vector<float> gp(ngestures);
     setVec(gp, 0.0f);
	for(int n = 0; n < ns; n++)
		gp[g[n]] += w[n];
    
	return gp;
}
// ----- DEPRECATED ------
vector<float> ofxGVF::getGestureConditionnalProbabilities()
{
	unsigned int ngestures = numTemplates+1;

    vector<float> gp(ngestures);
    setVec(gp, 0.0f);
	for(int n = 0; n < ns; n++)
		gp[g[n]] += w[n];
    
	return gp;
}

// Returns the estimates features. It calls status to refer to the status of the state
// space which comprises the features to be adapted. If features are phase, speed, scale and angle,
// the function will return these estimateed features for each gesture, plus their probabilities.
// The returned matrix is nxm
//   rows correspond to the gestures in the vocabulary
//   cols correspond to the features (the last column is the [conditionnal] probability of each gesture)
// The output matrix is an Eigen matrix
vector< vector<float> > ofxGVF::getEstimatedStatus()
{
	
    // get the number of gestures in the vocabulary
	unsigned int ngestures = numTemplates+1;
	//cout << "getEstimatedStatus():: ngestures= "<< numTemplates+1<< endl;
    
    vector< vector<float> > es;
    setMat(es, 0.0f, ngestures, pdim+1);   // rows are gestures, cols are features + probabilities
	//printMatf(es);
    
	// compute the estimated features by computing the expected values
    // sum ( feature values * weights)
	for(int n = 0; n < ns; n++)
	{
        int gi = g[n];
        for(int m=0; m<pdim; m++)
            es[gi][m] += X[n][m] * w[n];

		es[gi][pdim] += w[n];
    }
	
	for(int gi = 0; gi < ngestures; gi++)
	{
        for(int m=0; m<pdim; m++)
            es[gi][m] /= es[gi][pdim];
		//es.block(gi,0,1,pdim) /= es(gi,pdim);
	}
    
	return es;
}

vector<float> ofxGVF::getFeatureVariances(){
    return featVariances;
}

// Returns the index of the currently recognized gesture
int ofxGVF::getMostProbableGestureIndex(){
    vector< vector< float> > M = getEstimatedStatus();
    float maxprob=0.0;
    int   indexMaxprob=0;
    for (int k=0; k<M.size(); k++){
        if (M[k][M[0].size() - 1] > maxprob){
            maxprob = M[k][M[0].size() - 1];
            indexMaxprob = k;
        }
    }
    return indexMaxprob;
}



////////////////////////////////////////////////////////////////
//
// SET FUNCTIONS to UPDATE INTERNAL VALUES
//
////////////////////////////////////////////////////////////////

// Update the number of particles
void ofxGVF::setNumberOfParticles(int newNs){
    particlesPositions.clear();
    initMat(X,newNs,pdim);          // Matrix of NS particles
    initVec(g,newNs);               // Vector of gesture class
    initVec(w,newNs);               // Weights
    //    logW = VectorXf(newNs);
}

// Update the standard deviation of the observation distribution
// this value acts as a tolerance for the algorithm
// low value: less tolerant so more precise but can diverge
// high value: more tolerant so less precise but converge more easily
void ofxGVF::setToleranceValue(float f){
	tolerance = f > 0 ? f : tolerance;
}

// Update the variance for each features which control their precision
// and speed of convergence
void ofxGVF::setAdaptSpeed(vector<float> as){
	
	if (as.size() == pdim){
		for (int k=0; k<pdim; k++)
			featVariances[k]=sqrt(as[k]);
	}
	
}

// Update the resampling threshold used to avoid degeneracy problem
void ofxGVF::setResamplingThreshold(int r){
    resamplingThreshold = r;
}


////////////////////////////////////////////////////////////////
//
// MISC FUNCTIONS
//
////////////////////////////////////////////////////////////////

// Save function. This function is used by applications to save the
// vocabulary in a text file given by filename (filename is also the complete path + filename)
void ofxGVF::saveTemplates(std::string filename){
    
    std::string directory = filename;
    
    std::ofstream file_write(directory.c_str());
    for(int i=0; i<R_single.size(); i++){
        file_write << "template " << i << " " << inputDim << endl;
        for(int j=0; j<R_single[i].size(); j++)
        {
            for(int k=0; k<inputDim; k++)
                file_write << R_single[i][j][k] << " ";
            file_write << endl;
        }
    }
    file_write.close();
    //values.clear();
    
}


// Load function. This function is used by applications to load a vocabulary
// given by filename (filename is also the complete path + filename)
void ofxGVF::loadTemplates(std::string filename){
    
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
    float* vect_0_l;
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
                    vect_0_l[kk] = vect[kk];
                }
                template_starting_point=0;
            }
            // store the incoming list as a vector of float
            for (int kk=0; kk<template_dim; kk++)
            {
                vect[kk] = (float)atof(list[k+kk].c_str());
                vect[kk] = vect[kk]-vect_0_l[kk];
            }
            //post("fill %i with %f %f",numTemplates,vect[0],vect[1]);
            fillTemplate(numTemplates,vect);
        }

        k+=template_dim;
        
    }
    
    infile.close();
    
}
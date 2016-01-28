/**
 * Gesture Variation Follower class allows for early gesture recognition and variation tracking
 *
 * @details Original algorithm designed and implemented in 2011 at Ircam Centre Pompidou
 * by Baptiste Caramiaux and Nicola Montecchio. The library has been created and is maintained by Baptiste Caramiaux
 *
 * Copyright (C) 2015 Baptiste Caramiaux, Nicola Montecchio
 * STMS lab Ircam-CRNS-UPMC, University of Padova, Goldsmiths College University of London
 *
 * The library is under the GNU Lesser General Public License (LGPL v3)
 */


#ifndef _H_GVF
#define _H_GVF

#include "GVFUtils.h"
#include "GVFGesture.h"
#include <random>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>


using namespace std;

class GVF
{
    
public:
    
    enum GVFState{
        STATE_CLEAR = 0,
        STATE_LEARNING,
        STATE_FOLLOWING
    };
    
    
#pragma mark - Constructors
    
    /**
     * GVF default constructor
     * @details use default configuration and parameters, can be changed using accessors
     */
    GVF();
    
    /**
     * GVF default constructor
     * @param configuration structure (nb. dimensions etc), use default parameters
     */
    GVF(GVFConfig _config);
    
    /**
     * GVF default constructor
     * @param configuration structure (nb. dimensions etc)
     * @param parameters structure
     */
    GVF(GVFConfig _config, GVFParameters _parameters);
    
    /**
     * GVF default destructor
     */
    ~GVF();
    
#pragma mark - Gesture templates

    /**
     * Start a gesture either to be recorded or followed
     */
    void startGesture();
    
    /**
     * Add an observation to a gesture template
     * @details
     * @param vector of features
     */
    void addObservation(vector<float> data);
    
    /**
     * Add gesture template to the vocabulary
     *
     * @details a gesture template is a GVFGesture object
     * @param the gesture template to be recorded
     */
    void addGestureTemplate(GVFGesture & gestureTemplate);
    
    /**
     * Replace a specific gesture template by another
     *
     * @param the gesture template to be used
     * @param the gesture index (as integer) to be replaced
     */
    void replaceGestureTemplate(GVFGesture & gestureTemplate, int index);
    
    /**
     * Remove a specific template
     *
     * @param the gesture index (as integer) to be removed
     */
    void removeGestureTemplate(int index);
    
    /**
     * Remove every recorded gesture template
     */
    void removeAllGestureTemplates();
    
    /**
     * Get a specific gesture template a gesture template by another
     *
     * @param index of the template to be returned
     * @return the template
     */
    GVFGesture & getGestureTemplate(int index);
    
    /**
     * Get every recorded gesture template
     *
     * @return the vecotr of gesture templates
     */
    vector<GVFGesture> & getAllGestureTemplates();
    
    /**
     * Get number of gesture templates in the vocabulary
     * @return the number of templates
     */
    int getNumberOfGestureTemplates();
    
    /**
     * Get gesture classes
     */
    vector<int> getGestureClasses();
    
    
#pragma mark - Recognition and tracking

    /**
     *
     */
    GVFState setState(GVFState _state, vector<int> indexes = vector<int>());
    
    /**
     *
     */
    GVFState getState();
    
    /**
     * Compute the estimated gesture and its potential variations
     *
     * @details infers the probability that the current observation belongs to
     * one of the recorded gesture template and track the variations of this gesture
     * according to each template
     *
     * @param vector of the observation data at current time
     * @return the estimated probabilities and variaitons relative to each template
     */
    GVFOutcomes & update(vector<float> & observation);
    
    /**
     * Define a subset of gesture templates on which to perform the recognition
     * and variation tracking
     *
     * @details By default every recorded gesture template is considered
     * @param set of gesture template index to consider
     */
    void setActiveGestures(vector<int> activeGestureIds);
    
    /**
     * Restart GVF
     * @details re-sample particles at the origin (i.e. initial prior)
     */
    void restart();
    
    /**
     * Clear GVF
     * @details delete templates
     */
    void clear();
    
    void translate(bool translateFlag);
    
    void segmentation(bool segmentationFlag);
    
    
//#pragma mark - Output accessors (deprecated, use GVFOutcomes)
//    
//    /**
//     * Get gesture probabilities
//     * @return vector of probabilities
//     */
//    vector<float> & getGestureProbabilities();
//    
//    /**
//     *
//     */
//    int getMostProbableGestureIndex();
    
#pragma mark - [ Accessors ]
    
//    void setParameters(GVFParameters parameters);
//    GVFParameters getParameters();

#pragma mark > Parameters
    /**
     * Set tolerance between observation and estimation
     * @details tolerance depends on the range of the data
     * typially tolerance = (data range)/3.0;
     * @param tolerance value
     */
    void setTolerance(float tolerance);
    
    /**
     * Get the obervation tolerance value
     * @details see setTolerance(float tolerance)
     * @return the current toleranc value
     */
    float getTolerance();
    
    /**
     * Set number of particles used in estimation
     * @details default valye is 1000, note that the computational
     * cost directly depends on the number of particles
     * @param new number of particles
     */
    void setNumberOfParticles(int numberOfParticles);
    
    /**
     * Get the current number of particles
     * @return the current number of particles
     */
    int getNumberOfParticles();
    
    /**
     * Number of prediciton steps
     * @details it is possible to leave GVF to perform few steps of prediction
     * ahead which can be useful to estimate more fastly the variations. Default value is 1
     * which means no prediction ahead
     * @param the number of prediction steps
     */
    void setNumberOfPredictionSteps(int predictionSteps);
    
    /**
     * Get the current number of prediction steps
     * @return current number of prediciton steps
     */
    int getNumberOfPredictionSteps();

    /**
     * Set resampling threshold
     * @details resampling threshold is the minimum number of active particles
     * before resampling all the particles by the estimated posterior distribution.
     * in other words, it re-targets particles around the best current estimates
     * @param the minimum number of particles (default is (number of particles)/2)
     */
    void setResamplingThreshold(int resamplingThreshold);
    
    /**
     * Get the current resampling threshold
     * @return resampling threshold
     */
    int getResamplingThreshold();
    
#pragma mark > Alignment
    void setAlignmentVariance(float alignmentVariance);
    float getAlignmentVariance();
    
#pragma mark > Dynamics
    void setDynamicsVariance(float dynVariance);
    void setDynamicsVariance(float dynVariance, int dim);
    void setDynamicsVariance(vector<float> dynVariance);
    vector<float> getDynamicsVariance();
    
#pragma mark > Scalings
    void setScalingsVariance(float scaleVariance);
    void setScalingsVariance(float scaleVariance, int dim);
    void setScalingsVariance(vector<float> scaleVariance);
    vector<float> getScalingsVariance();

#pragma mark > Rotations
    void setRotationsVariance(float rotationsVariance);
    void setRotationsVariance(float rotationsVariance, int dim);
    void setRotationsVariance(vector<float> rotationsVariance);
    vector<float> getRotationsVariance();
    
    
#pragma mark > Others
    
    /**
     * Get particle values
     * @return vector of list of estimated particles
     */
    const vector<vector<float> > & getParticlesPositions();

    /**
     *
     */
    void setSpreadDynamics(float min, float max);

    /**
     *
     */
    void setSpreadScalings(float min, float max);
    
    /**
     *
     */
    void setSpreadRotations(float min, float max);
    
//    vector<float> getEstimatedProbabilities();
//    vector<float> getEstimatedLikelihoods();
//    vector<float> getWeights();
//    vector<float> getPrior();
    
    
#pragma mark - Import/Export templates
    
    void saveTemplates(string filename);
    void loadTemplates(string filename);
    
    
#pragma mark - openFrameworks convetion: setup functions
    
    void setup(); // use default config and parameters
    void setup(GVFConfig _config); // default parameters
    void setup(GVFConfig _config, GVFParameters _parameters);
    
//#pragma mark - Deprecated functions, to be removed in the next version
//    
//    /**
//     * Perform inference, replaced by update()
//     */
//    void infer(vector<float> obs);
//    GVFOutcomes getOutcomes();
//
//
//    
//#pragma mark DEPRECATED
    
private:
    
    // private variables
    GVFConfig        config;
    GVFParameters    parameters;
    GVFOutcomes      outcomes;
    GVFState         state;
    GVFGesture       theGesture;
    
    
    vector<float> dimWeights;           // TOOD: to be put in parameters?
    vector<float> maxRange;
    vector<float> minRange;
    int     dynamicsDim;                // dynamics state dimension
    int     scalingsDim;                // scalings state dimension
    int     rotationsDim;               // rotation state dimension
    float   globalNormalizationFactor;          // flagged if normalization
    int     mostProbableIndex;                  // cached most probable index
    int     learningGesture;
    
    vector<int>             classes;            // gesture index for each particle [ns x 1]
    vector<float >          alignment;          // alignment index (between 0 and 1) [ns x 1]
    vector<vector<float> >  dynamics;           // dynamics estimation [ns x 2]
    vector<vector<float> >  scalings;           // scalings estimation [ns x D]
    vector<vector<float> >  rotations;          // rotations estimation [ns x A]
    vector<float>           weights;            // weight of each particle [ns x 1]
    vector<float>           prior;              // prior of each particle [ns x 1]
    vector<float>           posterior;          // poserior of each particle [ns x 1]
    vector<float>           likelihood;         // likelihood of each particle [ns x 1]
    
    // estimations
    vector<float>           estimatedGesture;           // ..
    vector<float>           estimatedAlignment;         // ..
    vector<vector<float> >  estimatedDynamics;          // ..
    vector<vector<float> >  estimatedScalings;          // ..
    vector<vector<float> >  estimatedRotations;         // ..
    vector<float>           estimatedProbabilities;     // ..
    vector<float>           estimatedLikelihoods;       // ..
    vector<float>           absoluteLikelihoods;        // ..
    
    // velocity and acceleration estimations
    vector<float>           currentVelAcc;
    vector<vector<float>>   kalmanDynMatrix;
    vector<vector<float>>   kalmanMapMatrix;
    
    
    bool tolerancesetmanually;
    
    vector<GVFGesture>   gestureTemplates;           // ..
    vector<vector<float> >  offsets;                    // translation offset
    
    vector<int> activeGestures;
    

    
    // random number generator
    std::random_device                      rd;
    std::mt19937                            normgen;
    std::normal_distribution<float>         *rndnorm;
    std::default_random_engine              unifgen;
    std::uniform_real_distribution<float>   *rndunif;
    
//    // ONLY for LOGS
//    vector<vector<float> >  vecRef;
//    vector<float>           vecObs;
//    vector<float>           stateNoiseDist;
    
    vector<float> gestureProbabilities;
    vector< vector<float> > particles;
    //    vector<float> particlesPositions;
    
//    vector<float>& getGestureTemplateSample(int gestureIndex, float cursor);

    
#pragma mark - Private methods for model mechanics
    
    void initPrior();
    void initNoiseParameters();
    void updateLikelihood(vector<float> obs, int n);
    void updatePrior(int n);
    void updatePosterior(int n);
    void resampleAccordingToWeights(vector<float> obs);
    void estimates();       // update estimated outcome
    void train();
    
    
};


#endif
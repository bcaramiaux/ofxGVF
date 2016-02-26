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
    
    /** 
     * GVF possible states
     */
    enum GVFState
    {
        STATE_CLEAR = 0,    /**< STATE_CLEAR: clear the GVF and be in standby */
        STATE_LEARNING,     /**< STATE_LEARNING: recording mode, input gestures are added to the templates */
        STATE_FOLLOWING,    /**< STATE_FOLLOWING: tracking mode, input gestures are classifed and their variations tracked (need the GVF to be trained) */
        STATE_BYPASS        /**< STATE_BYPASS: by pass GVF but does not erase templates or training */
    };
    
    
#pragma mark - Constructors
    
    /**
     * GVF default constructor
     * @details use default configuration and parameters, can be changed using accessors
     */
    GVF();
    
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
     * @param data vector of features
     */
    void addObservation(vector<float> data);
    
    /**
     * Add gesture template to the vocabulary
     *
     * @details a gesture template is a GVFGesture object and can be added directly to the vocabulqry or
     * recorded gesture templates by using this method
     * @param gestureTemplate the gesture template to be recorded
     */
    void addGestureTemplate(GVFGesture & gestureTemplate);
    
    /**
     * Replace a specific gesture template by another
     *
     * @param gestureTemplate the gesture template to be used
     * @param index the gesture index (as integer) to be replaced
     */
    void replaceGestureTemplate(GVFGesture & gestureTemplate, int index);
    
    /**
     * Remove a specific template
     *
     * @param index the gesture index (as integer) to be removed
     */
    void removeGestureTemplate(int index);
    
    /**
     * Remove every recorded gesture template
     */
    void removeAllGestureTemplates();
    
    /**
     * Get a specific gesture template a gesture template by another
     *
     * @param index the index of the template to be returned
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
     * Set the state of GVF
     * @param _state the state to be given to GVF, it is a GVFState
     * @param indexes an optional argument providing a list of gesture index. 
     * In learning mode the index of the gesture being recorded can be given as an argument
     * since the type is vector<int>, it should be something like '{3}'. In following mode, the list of indexes
     * is the list of active gestures to be considered in the recognition/tracking.
     */
    void setState(GVFState _state, vector<int> indexes = vector<int>());
    
    /**
     * Return the current state of GVF
     * @return GVFState the current state
     */
    GVFState getState();
    
    /**
     * Compute the estimated gesture and its potential variations
     *
     * @details infers the probability that the current observation belongs to
     * one of the recorded gesture template and track the variations of this gesture
     * according to each template
     *
     * @param observation vector of the observation data at current time
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

    /**
     * Translate data according to the first point
     * @details substract each gesture feature by the first point of the gesture
     * @param boolean to activate or deactivate translation
     */
    void translate(bool translateFlag);

    /**
     * Segment gestures within a continuous gesture stream
     * @details if segmentation is true, the method will segment a continuous gesture into a sequence
     * of gestures. In other words no need to call the method startGesture(), it is done automatically
     * @param segmentationFlag boolean to activate or deactivate segmentation
     */
    void segmentation(bool segmentationFlag);
    
#pragma mark - [ Accessors ]
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
    
    void setDistribution(float _distribution);
    
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
    void setPredictionSteps(int predictionSteps);
    
    /**
     * Get the current number of prediction steps
     * @return current number of prediciton steps
     */
    int getPredictionSteps();

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
    
#pragma mark > Dynamics
    /**
     * Change variance of adaptation in dynamics
     * @details if dynamics adaptation variance is high the method will adapt faster to 
     * fast changes in dynamics. Dynamics is 2-dimensional: the first dimension is the speed
     * The second dimension is the acceleration.
     *
     * Typically the variance is the average amount the speed or acceleration can change from
     * one sample to another. As an example, if the relative estimated speed can change from 1.1 to 1.2 
     * from one sample to another, the variance should allow a change of 0.1 in speed. So the variance 
     * should be set to 0.1*0.1 = 0.01
     *
     * @param dynVariance dynamics variance value
     * @param dim optional dimension of the dynamics for which the change of variance is applied (default value is 1)
     */
    void setDynamicsVariance(float dynVariance, int dim = -1);
    
    /**
     * Change variance of adaptation in dynamics
     * @details See setDynamicsVariance(float dynVariance, int dim) for more details
     * @param dynVariance vector of dynamics variances, each vector index is the variance to be applied to
     * each dynamics dimension (consequently the vector should be 2-dimensional).
     */
    void setDynamicsVariance(vector<float> dynVariance);
    
    /**
     * Get dynamics variances
     * @return the vector of variances (the returned vector is 2-dimensional)
     */
    vector<float> getDynamicsVariance();
    
#pragma mark > Scalings
    /**
     * Change variance of adaptation in scalings
     * @details if scalings adaptation variance is high the method will adapt faster to
     * fast changes in relative sizes. There is one scaling variance for each dimension
     * of the input gesture. If the gesture is 2-dimensional, the scalings variances will 
     * also be 2-dimensional.
     *
     * Typically the variance is the average amount the size can change from
     * one sample to another. As an example, if the relative estimated size changes from 1.1 to 1.15
     * from one sample to another, the variance should allow a change of 0.05 in size. So the variance
     * should be set to 0.05*0.05 = 0.0025
     *
     * @param scalings variance value
     * @param dimension of the scalings for which the change of variance is applied
     */
    void setScalingsVariance(float scaleVariance, int dim = -1);
    
    /**
     * Change variance of adaptation in dynamics
     * @details See setScalingsVariance(float scaleVariance, int dim) for more details
     * @param vector of scalings variances, each vector index is the variance to be applied to
     * each scaling dimension.
     * @param vector of variances (should be the size of the template gestures dimension)
     */
    void setScalingsVariance(vector<float> scaleVariance);
    
    /**
     * Get scalings variances
     * @return the vector of variances
     */
    vector<float> getScalingsVariance();

#pragma mark > Rotations
    /**
     * Change variance of adaptation in orientation
     * @details if rotation adaptation variance is high the method will adapt faster to
     * fast changes in relative orientation. If the gesture is 2-dimensional, there is 
     * one variance value since the rotation can be defined by only one angle of rotation. If
     * the gesture is 3-dimensional, there are 3 variance values since the rotation in 3-d is
     * defined by 3 rotation angles. For any other dimension, the rotation is not defined.
     *
     * The variance is the average amount the orientation can change from one sample to another. 
     * As an example, if the relative orientation in rad changes from 0.1 to 0.2 from one observation
     * to another, the variance should allow a change of 0.1 in rotation angle. So the variance
     * should be set to 0.1*0.1 = 0.01
     *
     * @param rotationsVariance rotation variance value
     * @param dim optional dimension of the rotation for which the change of variance is applied
     */
    void setRotationsVariance(float rotationsVariance, int dim = -1);
    
    /**
     * Change variance of adaptation in orientation
     * @details See setRotationsVariance(float rotationsVariance, int dim) for more details
     * @param vector of rotation variances, each vector index is the variance to be applied to
     * each rotation angle (1 or 3)
     * @param vector of variances (should be 1 if the the template gestures are 2-dim or 3 if 
     * they are 3-dim)
     */
    void setRotationsVariance(vector<float> rotationsVariance);
    
    /**
     * Get rotation variances
     * @return the vector of variances
     */
    vector<float> getRotationsVariance();
    
    
#pragma mark > Others
    
    /**
     * Get particle values
     * @return vector of list of estimated particles
     */
    const vector<vector<float> > & getParticlesPositions();

    /**
     * Set the interval on which the dynamics values should be spread at the beginning (before adaptation)
     * @details this interval can be used to concentrate the potential dynamics value on a narrow interval,
     * typically around 1 (the default value), for instance between -0.05 and 0.05, or to allow at the very 
     * beginning, high changes in dynamics by spreading, for instance between 0.0 and 2.0
     * @param min lower value of the inital values for dynamics
     * @param max higher value of the inital values for dynamics
     * @param dim the dimension on which the change of initial interval should be applied (optional)
     */
    void setSpreadDynamics(float min, float max, int dim = -1);

    /**
     * Set the interval on which the scalings values should be spread at the beginning (before adaptation)
     * @details this interval can be used to concentrate the potential scalings value on a narrow interval,
     * typically around 1.0 (the default value), for instance between 0.95 and 1.05, or to allow at the very 
     * beginning high changes in dynamics by spreading, for instance, between 0.0 and 2.0
     * @param min lower value of the inital values for scalings
     * @param max higher value of the inital values for scalings
     * @param dim the dimension on which the change of initial interval should be applied (optional)
     */
    void setSpreadScalings(float min, float max, int dim = -1);
    
    /**
     * Set the interval on which the angle of rotation values should be spread at the beginning (before adaptation)
     * @details this interval can be used to concentrate the potential angle values on a narrow interval,
     * typically around 0.0 (the default value), for instance between -0.05 and 0.05, or to allow at the very
     * beginning, high changes in orientation by spreading, for instance, between -0.5 and 0.5
     * @param min lower value of the inital values for angle of rotation
     * @param max higher value of the inital values for angle of rotation
     * @param dim the dimension on which the change of initial interval should be applied (optional)
     */
    void setSpreadRotations(float min, float max, int dim = -1);
    
#pragma mark - Import/Export templates
    /**
     * Export template data in a filename
     * @param filename file name as a string
     */
    void saveTemplates(string filename);

    /**
     * Import template data in a filename
     * @details needs to respect a given format provided by saveTemplates()
     * @param file name as a string
     */
    void loadTemplates(string filename);

protected:
    
    GVFConfig        config;        // Structure storing the configuration of GVF (in GVFUtils.h)
    GVFParameters    parameters;    // Structure storing the parameters of GVF (in GVFUtils.h)
    GVFOutcomes      outcomes;      // Structure storing the outputs of GVF (in GVFUtils.h)
    GVFState         state;         // State (defined above)
    GVFGesture       theGesture;    // GVFGesture object to handle incoming data in learning and following modes
    
    vector<GVFGesture>   gestureTemplates; // vector storing the gesture templates recorded when using the methods addObservation(vector<float> data) or addGestureTemplate(GVFGesture & gestureTemplate)
    
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

    bool tolerancesetmanually;
    

    vector<vector<float> >  offsets;                    // translation offset
    
    vector<int> activeGestures;

    vector<float> gestureProbabilities;
    vector< vector<float> > particles;

private:

    // random number generator
    std::random_device                      rd;
    std::mt19937                            normgen;
    std::normal_distribution<float>         *rndnorm;
    std::default_random_engine              unifgen;
    std::uniform_real_distribution<float>   *rndunif;

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
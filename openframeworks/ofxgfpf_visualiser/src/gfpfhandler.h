//
//  gfpfhandler.h
//  ofGfpfVisualiser
//
//  Created by Thomas Rushmore on 21/06/2013.
//  Modified by Igor Correa
//  An instance of this class should be used to access the gfpf algorithim.
//  It provides methods to feed data to the particle filter and get the results from it.
//  It also provides methods to change the main gfpf parameters.
//  For other useful functionalities, refer to the descriptions of the methods below.

#ifndef __graphicsExample__gfpfhandler__
#define __graphicsExample__gfpfhandler__


#include <iostream>
#include "ofMain.h"

#include "gfpf.h"
#include <Eigen/LU> // To use MatrixXf on overloaded gfpf_data method
#include "gfpfGesture.h"
using namespace Eigen;

// enumaration defining the 3 states the application can be
// STATE_CLEAR: no gestures learnt
// STATE_LEARNING: learning new gesture or waiting for the user to perform it
// STATE_FOLLOWING: following the current gesture or waiting for the user to perform it
enum {STATE_CLEAR, STATE_LEARNING, STATE_FOLLOWING};

// struct to hold information on how a learnt template is being recognised.
typedef struct {
    float likelihoods;
    float probability;
    float phase;
    float speed;
    float scale;
    float rotation;
}  recognitionInfo;

class gfpfhandler{
public:
    gfpfhandler();
    ~gfpfhandler();
    
    // increases the count of references learnt and change the state to STATE_LEARNING
    int  gfpf_learn();
    
    // spread particles and change the state to STATE_FOLLOWING
    void gfpf_follow();
    
    // clear all templates and change the state to STATE_CLEAR
    void gfpf_clear();
    
    // feeds data to gfpf based on the current state
    // argc is the number of dimentions on the input and argv is the input array
    void gfpf_data(int argc, float *argv);

    // treats the 2 dimentional point p and calls gfpf_data(int argc, float *argv);
    void gfpf_data(ofPoint p);
    
    void gfpf_restart();
    
    // methods used to change gfpf parameters
    void gfpf_std(float smoothingCoeficient);
    void gfpf_rt(int resamplingThreshold);
    void gfpf_adaptspeed (std::vector<float> varianceCoeficients); // the variance coefficients can only be changed all at once.

    void setNumberOfParticles(int newNs);
    
    // returns a string containing information about the current recognition
    // (probabilities, phase, speed and scale for each gesture)
    string gfpf_get_status();
    
    // amount of templates
    int getTemplateCount();
    
    // current state (STATE_CLEAR, STATE_LEARNING or STATE_FOLLOWING)
    int get_state();

    // vector containing one recognitionInfo struct for each template
    // these structs will contain real time information on how each template is being recognised
    std::vector<recognitionInfo> recogInfo;
    std::vector<recognitionInfo> getRecogInfo();
    recognitionInfo getRecogInfoOfMostProbable();
    
    // gets all points based on a template index
    std::vector<std::vector<float> > get_template_data(int index);
    
    // creates a gfpfGesture object for each template and, using the scale provided, calls draw
    void drawTemplates(float scale);
    
    // by retrieving information directly from gfpf, draws circles representing each particle
    void printParticleInfo(gfpfGesture currentGesture);

    // adds a new template gesture. Here is where the position of the miniature templates are calculated
    void addTemplateGesture(ofPoint initialPoint, ofColor templateColor);
    
    gfpfGesture getTemplateGesture(int index);
    
    // returns a gfpfGesture representing the estimated gesture being currently recognised
    gfpfGesture getRecognisedGestureRepresentation();

    // returns the index of the template that is, most probably,
    // the one the user is trying to perform
    int getIndexMostProbable();

    Eigen::VectorXf getVref();
    
private:

    // will hold empty versions of the templates,
    // but when getTemplateGesture is called the data is retrieved and a prober version of the gesture is returned
    std::vector<gfpfGesture> templates;

    // reference to the gfpf object
    gfpf *gf;
    
    // variance coefficients
    float sigPosition, sigSpeed, sigScale, sigRotation;
    
    float smoothingCoef;
    
    int rt, ns; // resampling threshold and amount of particles;
    
    int rp, pdim;
    Eigen::VectorXf mpvrs;
	Eigen::VectorXf rpvrs;
    int Nspg, Rtpg;
    int state;
	int lastreferencelearned;
    std::map<int,std::vector<std::pair<float,float> > > *refmap;
    int restarted_l;
    int restarted_d;
    std::pair<float,float> xy0_l;
    std::pair<float,float> xy0_d;
    std::vector<float> vect_0_l;
    std::vector<float> vect_0_d;

};
#endif /* defined(__graphicsExample__gfpfhandler__) */

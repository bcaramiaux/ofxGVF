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


#ifndef _H_OFXGVF
#define _H_OFXGVF

#include "GVF.h"
#include <random>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>


using namespace std;



class ofxGVF : public GVF
{
    
public:
    
    /**
     * Similar to constructor GVF() - for openFrameworks
     */
    void setup()
    {
        clear(); // just in case
        
        config.inputDimensions   = 2;
        config.translate         = true;
        config.segmentation      = false;

        learningGesture = -1;
        
        // default parameters
        parameters.numberParticles       = 1000;
        parameters.tolerance             = 0.2f;
        parameters.resamplingThreshold   = 250;
        parameters.distribution          = 0.0f;
        parameters.alignmentVariance     = sqrt(0.000001f);
        parameters.dynamicsVariance      = vector<float>(1,sqrt(0.001f));
        parameters.scalingsVariance      = vector<float>(1,sqrt(0.00001f));
        parameters.rotationsVariance     = vector<float>(1,sqrt(0.0f));
        parameters.predictionSteps       = 1;
        parameters.dimWeights            = vector<float>(1,sqrt(1.0f));
        
        // default spreading
        parameters.alignmentSpreadingCenter = 0.0;
        parameters.alignmentSpreadingRange  = 0.2;
        
        parameters.dynamicsSpreadingCenter = 1.0;
        parameters.dynamicsSpreadingRange  = 0.3;
        
        parameters.scalingsSpreadingCenter = 1.0;
        parameters.scalingsSpreadingRange  = 0.3;
        
        parameters.rotationsSpreadingCenter = 0.0;
        parameters.rotationsSpreadingRange  = 0.0;
        
        tolerancesetmanually = false;
        
    }
    
};


#endif
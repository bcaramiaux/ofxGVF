#pragma once

//addons
#include "ofMain.h"
#include "ofxKinectFeatures.h"
#include "ofxGVF.h"
#include "ofxOpenNI.h"
#include "ofxXmlSettings.h"

#define FRAMERATE_MILLIS 33

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit();
    
    //Device
    ofxOpenNI kinect;
    bool hadUsers;
    void checkKinectReset();
    
    //XML
    ofxXmlSettings xml;
    
    //Feature extractor
    ofxKinectFeatures featExtractor;
    
    //GVF
    float velocityThreshold;
    ofxGVF gvfRight, gvfLeft;
    ofxGVFGesture       currentGestureLeft, currentGestureRight;
    ofxGVFConfig        config;
    ofxGVFParameters    parameters;
    ofxGVFOutcomes      outcomes;
    bool performingLearningRight, performingLearningLeft, performingFollowingRight, performingFollowingLeft;
    
};

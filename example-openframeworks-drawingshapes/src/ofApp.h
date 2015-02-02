#pragma once

#include "ofMain.h"
#include "ofxGVF.h"
#include "ofxUI.h"
#include <math.h>

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void setupGVFGui();
        ofxUISuperCanvas *GVFGui;
        void guiEvent(ofxUIEventArgs &e);
    
        ofxGVF gvf;
        ofxGVFGesture       currentGesture;
        ofxGVFConfig        config;
        ofxGVFParameters    parameters;
        ofxGVFOutcomes      outcomes;
    
        float initX, initY;

    float inputX, inputY;
    bool performingLearning;
    bool performingFollowing;
	
	string templateFile;
};

#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"

#include "GestureVariationFollower.h"
#include "gvfhandler.h"

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
    
    void saveGestures();
    void loadGestures();
    float counter;
    
private:
    
    void initColors();
    ofColor generateRandomColor();
    
    // gui stuff
    void initializeGui();
    
    ofxPanel gui;
    
    ofParameter<int> numParticles;
	ofParameter<int> resampleThreshold;
	ofParameter<float> smoothingCoefficient;
	ofParameter<float> sigPosition;
    ofParameter<float> sigRotation;
    ofParameter<float> sigScale;
	ofParameter<float> sigSpeed;
    
    ofxButton save;
    ofxButton load;
    
    ofxLabel label;
    
    void numParticlesChanged(int & numParticles);
    void resampleThresholdChanged(int & resampleThreshold);
    void smoothingCoefficientChanged(float & smoothingCoefficient);
    void varianceCoefficentsChanged(float & coefficent);
    
    int scrW, scrH;
    
    ofPoint templates_area = ofPoint(200, 90);
    
    //    ofRectangle drawArea = ofRectangle(ofGetWindowWidth()/2 - side_of_drawing_area/2,
    //                             templates_area.y + side_of_drawing_area,
    //                             side_of_drawing_area,
    //                             side_of_drawing_area);
    //
    
    ofRectangle drawArea = ofRectangle(ofPoint(0, 0), ofGetWindowWidth(), ofGetWindowHeight());
    
    ofRectangle testRect = ofRectangle(900, 400, 330, 330);
    
    gvfhandler gvfh;
    gvfGesture currentGesture = gvfGesture(drawArea);
    
    bool isMouseDrawing = false;
    bool rotate = false;
    
    bool displayParticles = true;
    bool displayCurrentGesture = true;
    bool displayEstimatedGesture = true;
    bool displayTemplate = true;
    
    float zDist = 10;
    ofEasyCam cam; // add mouse controls for camera movement
    float viewYRotation;
    float viewXRotation;
    
    std::vector<ofColor> colors;
    
};


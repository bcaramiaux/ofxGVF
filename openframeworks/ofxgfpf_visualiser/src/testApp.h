#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "gfpf.h"

//#include "gfpfGesture.h"
#include "gfpfhandler.h"
#include "ofxLeapMotion.h"


using namespace Eigen;

class testApp : public ofBaseApp{
	
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
    void guiEvent(ofxUIEventArgs &e);
    
    void saveGestures(ofFileDialogResult dialogResult);
    void loadGestures(ofFileDialogResult dialogResult);
    float 	counter;
    
private:
    
    void initColors();
    ofColor generateRandomColor();
    
    void initializeGui();
    
    int scrW, scrH;
    
    ofPoint templates_area = ofPoint(200, 90);
    
    //    ofRectangle drawArea = ofRectangle(ofGetWindowWidth()/2 - side_of_drawing_area/2,
    //                             templates_area.y + side_of_drawing_area,
    //                             side_of_drawing_area,
    //                             side_of_drawing_area);
    //
    
    ofRectangle drawArea = ofRectangle(ofPoint(0, 0), ofGetWindowWidth(), ofGetWindowHeight());
    
    ofRectangle testRect = ofRectangle(900, 400, 330, 330);
    
    gfpfhandler gfpfh;
    gfpfGesture currentGesture = gfpfGesture(drawArea);
    
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
    
    ofRectangle guiArea;
    ofxUICanvas *settingsGui;
    ofxUINumberDialer *nsNumDialer;
    ofxUINumberDialer *rtNumDialer;
    ofxUINumberDialer *soNumDialer;
    ofxUINumberDialer *sigPosND, *sigSpeedND, *sigScaleND, *sigRotND;
    
    //leap
    ofxLeapMotion leap;
	vector <ofxLeapMotionSimpleHand> simpleHands;
	vector <int> fingersFound;
};


#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxGVF.h"

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    void guiEvent(ofxUIEventArgs &e);
    void initGui();
    void initColors();
    void displayParticlesOnGesture(GVFGesture currentGesture);
    
private:
    std::vector<float> theSample;
    ofxUISuperCanvas *settingsGui;
    
    // parameters
    ofxUINumberDialer * theNumberOfParticlesDialer;
    ofxUINumberDialer * theToleranceDialer;
    ofxUINumberDialer * theScalingVarianceDialer;
    
    ofxGVF * mygvf;
    GVFGesture gesture;
    
    ofRectangle drawArea;
    ofxUIRectangle guiArea;
    std::vector<ofColor> colors;
    
    float scrW, scrH;
    bool isMouseDrawing;
    bool displayParticles;
    bool displayTemplate;
    bool displayEstimatedGesture;
    bool displayCurrentGesture;
    
    void drawGesture(GVFGesture gesture);
    void drawGesture(float x, float y);
    void drawGesture(float x, float y, float w, float h);

};

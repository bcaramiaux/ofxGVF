#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofEnableSmoothing();
    ofSetCircleResolution(60);
    
    ofSetWindowTitle("openframeworks gvf visualiser");
    ofSetWindowShape(1024, 768);
    
    ofSetFrameRate(60); // if vertical sync is off, we can go a bit fast... this caps the framerate at 60fps.
    
    ofPoint wSize = ofGetWindowSize();
    scrW = (float)wSize.x;
    scrH = (float)wSize.y;
//    printf("w: %f h: %f\n", scrW, scrH);
    
    drawArea = ofRectangle(ofPoint(0, 0), ofGetWindowWidth(), ofGetWindowHeight());
    
    //    currentGesture.setDrawArea(drawArea);
    
    isMouseDrawing = false;
    
    int viewYRotation = 0;
    int viewXRotation = 0;
    
    initGui();
    initColors();
    
    mygvf = new ofxGVF();
    
    displayCurrentGesture = true;
    displayParticles = true;
    
}

//--------------------------------------------------------------
void ofApp::update()
{
    draw();
    if (isMouseDrawing)
    {
        if (mygvf->getState()==ofxGVF::STATE_FOLLOWING)
        {
            if (gesture.getNumberOfTemplates()>0)
            {
                mygvf->update(gesture.getLastObservation());
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    float templatesScale = 0.5f;
    ofBackgroundGradient(ofColor(2), ofColor(40), OF_GRADIENT_CIRCULAR);
    ofPushMatrix();
    
    string state_string;
    state_string.append("GESTURE VARIATION FOLLOWER (GVF)\n\n'l' to learn a new template\n'c' to clear\n"
                        "\n"
                        //"numbers 1 to 4 to toggle visual feedback "
                        //"(1 - particles; 2 - template; 3 - estimated gesture; 4 - current gesture)"
                        "\nSTATE_LEARINING [");
    
    
    // draw the current templates on a small scale
    //    gvfh.drawTemplates(templatesScale);
    
    if(mygvf->getState() == ofxGVF::STATE_FOLLOWING)
    {
        state_string.append(" ]\nSTATE_FOLLOWING [X]\nSTATE_CLEAR     [ ]");
        
        if(displayParticles)
            displayParticlesOnGesture(gesture);
        
        if (isMouseDrawing)
        {
//            gesture.draw();
            drawGesture(gesture);
            
            //            if(displayCurrentGesture)
            //                gesture.draw();
            //            if(displayParticles)
            //            displayParticlesOnGesture(gesture);
        }
    }
    else if(mygvf->getState() == ofxGVF::STATE_LEARNING)
    {
        state_string.append("X]\nSTATE_FOLLOWING [ ]\nSTATE_CLEAR     [ ]");
        if (isMouseDrawing) drawGesture(gesture); //gesture.draw();
    }
    else
    {
        state_string.append(" ]\nSTATE_FOLLOWING [ ]\nSTATE_CLEAR     [X]");
    }
    
    
    //ofDisableAlphaBlending();
    
    ofPopMatrix();
    ofSetColor(198);
    ofDrawBitmapString(state_string.c_str(), 30, 25);
    ofDrawBitmapString("FPS " + ofToString(ofGetFrameRate(), 0), ofGetWidth() - 200, 25);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if (key == 'l' || key == 'L')
    {
        if(mygvf->getState() != ofxGVF::STATE_LEARNING && !isMouseDrawing)
        {
            mygvf->setState(ofxGVF::STATE_LEARNING);
        }
    }
    else if(key == 'c' || key == 'C')
    {
        mygvf->clear();
        initColors();
    }
    else if (key == 'r' || key == 'R')
    {
        
    }
    else if (key == 'f' || key == 'F')
    {
        ofToggleFullscreen();
    }
    else if (key == '1')
    {
        displayParticles = !displayParticles;
    }
    else if (key == '2')
    {
        displayTemplate = !displayTemplate;
    }
    else if (key == '3')
    {
        displayEstimatedGesture = !displayEstimatedGesture;
    }
    else if (key == '4')
    {
        displayCurrentGesture = !displayCurrentGesture;
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    theSample.clear();
    // if a gesture has already been starded, a new point is added to it
    if(isMouseDrawing)
    {
        //        vector<float> obs(2,0.0);
        theSample.push_back(x); ///scrW;
        theSample.push_back(y); ///scrH;
        gesture.addObservation(theSample);
        //        gesture.addObservation(obs);
        //        if (mygvf->getState()==ofxGVF::STATE_FOLLOWING)
        //        {
        //            mygvf->update(gesture.getLastObservation());
        ////            std::cout << mygvf->getOutcomes().estimations[0].alignment << std::endl;
        //        }
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if (!isMouseDrawing)
    {
        theSample.clear();
        gesture.clear();
    }
    isMouseDrawing = true;
    if (mygvf->getState()==ofxGVF::STATE_FOLLOWING)
        mygvf->restart();
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    if(isMouseDrawing)
    {
        isMouseDrawing = false;
    }
    
    if (mygvf->getState() == ofxGVF::STATE_LEARNING)
    {
        mygvf->addGestureTemplate(gesture);
        mygvf->setState(ofxGVF::STATE_FOLLOWING);
        std::cout << "switch to following" << std::endl;
    }
    else if (mygvf->getState() == ofxGVF::STATE_FOLLOWING)
    {
        mygvf->restart();
    }
    gesture.clear();
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

void ofApp::initGui()
{
    float guiWidth = 200;
    float guiHeight = 300;
    guiArea = ofxUIRectangle(30, 175, guiWidth, guiHeight);
    settingsGui = new ofxUISuperCanvas("SUPER", guiArea);
    ofColor c(1,100,1,100);
    settingsGui->setColorBack(c);
    
    //    mode = new ofxUILabel("Standby", OFX_UI_FONT_MEDIUM);
    
    // the initial parameters values set here are not connected to the initial parameters values on the gvfhandler constructor
    // those are the real initial values. The values here will only take effect once the user changes them
    // (this is inconsistent and should be fixed)
    settingsGui->addSpacer(guiWidth - 2, 2);
    settingsGui->addWidgetDown(new ofxUILabel("PARAMETERS", OFX_UI_FONT_MEDIUM));
    settingsGui->addSpacer(guiWidth - 2, 2);
    //    settingsGui->addWidgetDown(mode);
    //    settingsGui->addSpacer(guiWidth - 2, 2);
    //    settingsGui->addWidgetDown(new ofxUILabel("Precision", OFX_UI_FONT_SMALL));
    theNumberOfParticlesDialer = new ofxUINumberDialer(10, 10000, 2000, 0, "precision", OFX_UI_FONT_SMALL);
    settingsGui->addWidgetDown(theNumberOfParticlesDialer);
    settingsGui->addSpacer(guiWidth - 2, 2);
    
    //    settingsGui->addWidgetDown(new ofxUILabel("Resampling", OFX_UI_FONT_SMALL));
    //    rtNumDialer = new ofxUINumberDialer(100, 10000, 1000, 0, "resampling", OFX_UI_FONT_SMALL);
    //    settingsGui->addWidgetDown(rtNumDialer);
    //    //    settingsGui->addSpacer(guiWidth - 2, 2);
    //
    //    settingsGui->addWidgetDown(new ofxUILabel("Tolerance", OFX_UI_FONT_SMALL));
    theToleranceDialer = new ofxUINumberDialer(0.01, 2, 0.12, 2, "tolerance", OFX_UI_FONT_SMALL);
    settingsGui->addWidgetDown(theToleranceDialer);
            settingsGui->addSpacer(guiWidth - 2, 2);
    //
    //    //    settingsGui->addWidgetDown(new ofxUILabel("", OFX_UI_FONT_SMALL));
    //    //    settingsGui->addWidgetDown(new ofxUILabel("Variance coefficients", OFX_UI_FONT_SMALL));
    //    //
    //    //    settingsGui->addWidgetDown(new ofxUILabel("Position", OFX_UI_FONT_SMALL));
    //    //    sigPosND = new ofxUINumberDialer(0.000001, 0.1, 0.0001, 6, "position", OFX_UI_FONT_SMALL);
    //    //    settingsGui->addWidgetDown(sigPosND);
    //
    //    settingsGui->addWidgetDown(new ofxUILabel("Speed adaptation", OFX_UI_FONT_SMALL));
    //    sigSpeedND = new ofxUINumberDialer(0.000001, 0.1, 0.01, 6, "speed adaptation", OFX_UI_FONT_SMALL);
    //    settingsGui->addWidgetDown(sigSpeedND);
    //
        theScalingVarianceDialer = new ofxUINumberDialer(0.000001, 0.1, 0.0001, 6, "scale adaptation", OFX_UI_FONT_SMALL);
        settingsGui->addWidgetDown(theScalingVarianceDialer);
    settingsGui->addSpacer(guiWidth - 2, 2);

    float red = 100;
    settingsGui->addMinimalSlider("GREEN", 0.0, 255.0, &red);
//    settingsGui->addWidgetDown(theScalingVarianceDialer);
    settingsGui->addSpacer(guiWidth - 2, 2);
    //
    //    settingsGui->addWidgetDown(new ofxUILabel("Rotation adaptation", OFX_UI_FONT_SMALL));
    //    sigRotND = new ofxUINumberDialer(0.000001, 0.1, 0.000001, 6, "rotation adaptation", OFX_UI_FONT_SMALL);
    //    settingsGui->addWidgetDown(sigRotND);
    //
    //    //    settingsGui->addWidgetDown(new ofxUILabel("", OFX_UI_FONT_SMALL));
    //    //    settingsGui->addLabelButton("Save gesture(s)", false);
    //    //    settingsGui->addLabelButton("Load gesture(s)", false);
    //
    //    //    settingsGui->addWidgetDown(new ofxUILabel("Sound", OFX_UI_FONT_SMALL));
    //    //    settingsGui->addToggle("activate", false);
    //
    ofAddListener(settingsGui->newGUIEvent, this, &ofApp::guiEvent);
    
    
}

void ofApp::guiEvent(ofxUIEventArgs &e)
{
    string name = e.widget->getName();
    
    if(name == "precision")
    {
        mygvf->setNumberOfParticles(theNumberOfParticlesDialer->getValue());
    }
    else if(name == "tolerance")
    {
        mygvf->setNumberOfParticles(theToleranceDialer->getValue());
    }
    else if(name == "scale adaptation")
    {
        mygvf->setScalingsVariance(theScalingVarianceDialer->getValue());
    }
//
//    // if save or load is requested,
//    // the appropriate dialog is shown and the task is carried out
//    else if(name == "Save gesture(s)")
//    {
//        ofxUILabelButton *button = (ofxUILabelButton*) e.widget;
//        if(button->getValue() && gvfh.getTemplateCount() > 0)
//        {
//            ofFileDialogResult dialogResult = ofSystemSaveDialog("my gestures.xml", "Save gestures");
//            if(dialogResult.bSuccess)
//            {
//                saveGestures(dialogResult);
//            }
//        }
//    }
//    else if(name == "Load gesture(s)")
//    {
//        ofxUILabelButton *button = (ofxUILabelButton*) e.widget;
//        if(button->getValue())
//        {
//            ofFileDialogResult dialogResult = ofSystemLoadDialog("Select the xml file containing gesture data");
//            if(dialogResult.bSuccess)
//            {
//                loadGestures(dialogResult);
//            }
//            
//        }
//    }
//    else if(name == "activate")
//    {
//        ofxUILabelButton *button = (ofxUILabelButton*) e.widget;
//        if(button->getValue())
//        {
//            cout << button->getValue() << endl;
//            
//            
//        }
//    }
}


void ofApp::initColors()
{
    colors.clear();
    colors.push_back(ofColor::white);
    colors.push_back(ofColor::gray);
    colors.push_back(ofColor::blue);
    colors.push_back(ofColor::cyan);
    colors.push_back(ofColor::olive);
    colors.push_back(ofColor::gold);
    colors.push_back(ofColor::magenta);
    colors.push_back(ofColor::violet);
}

void ofApp::displayParticlesOnGesture(GVFGesture currentGesture)
{
    const std::vector<std::vector<float> > pp = mygvf->getParticlesPositions();
    int ppSize = pp.size();
    float scale = 1;
    if(ppSize > 0)
    {
        vector<float> weights(ppSize,1.0/ppSize);// = mygvf->getW();
        float weightAverage = 0.0;
        
        for (int k=0;k<weights.size();k++)
        {
            weightAverage += weights[k]/((float)weights.size());
        }
        
        for(int i = 0; i < ppSize; i++)
        {
            ofPoint initialPoint;
            initialPoint.x = currentGesture.getInitialObservation()[0];
            initialPoint.y = currentGesture.getInitialObservation()[1];
            
            int length = mygvf->getGestureTemplate(pp[i][2]).getTemplate().size();
            int index  = pp[i][0]*mygvf->getGestureTemplate(pp[i][2]).getTemplate().size();
            if (index>=length-1) index=length-1;
            float scale = pp[i][1];
            
            vector<float> initial = currentGesture.getInitialObservation();
            vector<float> obs = mygvf->getGestureTemplate(pp[i][2]).getTemplate()[index];
            
            // each particle position is retrieved
            ofPoint point(obs[0],obs[1]);
            
            // and then scaled and translated in order to be drawn
            float x = point.x * scale + initialPoint.x;
            float y = point.y * scale + initialPoint.y;
            float radius = weights[i]/weightAverage;
            
            ofColor c(255,0,0); //,weights[i]/weightAverage);
            ofSetColor(c);
            ofDrawCircle(x, y, radius * 2.0);
        }
    }
}


// Draws the Normalized Mesh
void ofApp::drawGesture(GVFGesture gesture){
    
    float x = 0.0f;
    float y = 0.0f;
    float w = 1.0f;
    float h = 1.0f;
    
    vector< vector<ofMesh> > representationsNormal;
    
//    if(representationsNormal.size() > 0 && type == GEOMETRIC){
//        x = observationRangeMin[0];
//        y = observationRangeMin[1];
//        w = observationRangeMax[0] - observationRangeMin[0];
//        h = observationRangeMax[1] - observationRangeMin[1];
//    }
//    
//    if(representationsNormal.size() > 0 && type == TEMPORAL){
//        if(representationsNormal[0].size() > 0){
//            w = representationsNormal[0][0].getNumVertices();
//            for(int d = 0; d < inputDimensions; d++){
//                if(observationRangeMax[d] > h) h = observationRangeMax[d];
//            }
//        }
//    }
    
    drawGesture(x, y, w, h);
    
}

void ofApp::drawGesture(float x, float y){
    
//    float w = observationRangeMax[0] - observationRangeMin[0];
//    float h = observationRangeMax[1] - observationRangeMin[1];
    
//    drawGesture(x, y, w, h);
    
}

void ofApp::drawGesture(float x, float y, float w, float h){
    
    ofPushMatrix();
    
//    if(representationsNormal.size() > 0 && type == GEOMETRIC){
//        
//        float scaleX = observationRangeMax[0] - observationRangeMin[0];
//        float scaleY = observationRangeMax[1] - observationRangeMin[1];
//        //float scaleZ = observationRangeMax[2] - observationRangeMin[2];
//        
//        ofTranslate(x, y);
//        ofScale(w / scaleX, h / scaleY);
//        ofTranslate(-observationRangeMin[0], -observationRangeMin[1]);
//        
//        ofPushMatrix();
//        
//        ofScale(scaleX, scaleY);
//        
//        representationsNormal[0][0].draw();
//        
//        ofPopMatrix();
//        
//        ofNoFill();
//        ofSetColor(255, 0, 0);
//        ofPoint min = ofPoint(observationRangeMin[0], observationRangeMin[1], 0);
//        ofPoint max = ofPoint(observationRangeMax[0], observationRangeMax[1], 0);
//        ofRect(ofRectangle(min, max));
//        ofSetColor(255, 255, 255);
//    }
//    
//    if(representationsNormal.size() > 0 && representationsNormal[0].size() > 0 && type == TEMPORAL){
//        
//        float scaleM = -INFINITY;
//        float maxY = -INFINITY;
//        
//        for(int d = 0; d < inputDimensions; d++){
//            if(observationRangeMax[d] - observationRangeMin[d] > scaleM){
//                scaleM = observationRangeMax[d] - observationRangeMin[d];
//            }
//            if(observationRangeMax[d] > maxY){
//                maxY = observationRangeMax[d];
//            }
//        }
//        
//        ofTranslate(x, y);
//        ofScale(w / representationsNormal[0][0].getNumVertices(), h / maxY);
//        
//        for(int d = 0; d < inputDimensions; d++){
//            
//            ofPushMatrix();
//            ofScale(1.0f, observationRangeMax[d] - observationRangeMin[d]);
//            
//            representationsNormal[0][d].draw();
//            
//            ofPopMatrix();
//            
//        }
//        
//        ofNoFill();
//        ofSetColor(255, 0, 0);
//        ofPoint min = ofPoint(0, 0, 0);
//        ofPoint max = ofPoint(representationsNormal[0][0].getNumVertices(), maxY, 0);
//        ofRect(ofRectangle(min, max));
//        ofSetColor(255, 255, 255);
//    }
    
    ofPopMatrix();
    
}

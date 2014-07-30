#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetLogLevel(OF_LOG_VERBOSE);

    
    // CONFIGURATION of the GVF
    config.inputDimensions = 2;
    config.translate       = true;
    config.segmentation    = false;
    
    // PARAMETERS are set by default

    // CREATE the corresponding GVF
    gvf.setup(config);
    
    ofBackground(0, 0, 0);
    performingFollowing = false;
    
}

//--------------------------------------------------------------
void ofApp::update(){
    //if(gvf.getState() == ofxGVF::STATE_FOLLOWING) gvf.infer(currentGesture.getLastRawObservation());
        switch(gvf.getState()){
            case ofxGVF::STATE_LEARNING:
            {
                if (performingLearning)
                    currentGesture.addObservationRaw(ofPoint(inputX, inputY, 0));
                break;
            }
            case ofxGVF::STATE_FOLLOWING:
            {
                if (performingFollowing){
                    currentGesture.addObservationRaw(ofPoint(inputX, inputY, 0));
                    gvf.infer(currentGesture.getLastRawObservation());
                }
                break;
            }
    
            default:
                // nothing
                break;
        }

}

//--------------------------------------------------------------
void ofApp::draw(){

    ofPushMatrix();
    currentGesture.draw();
    ofPopMatrix();
    
    
    for(int i = 0; i < gvf.getNumberOfGestureTemplates(); i++){
        
        ofxGVFGesture & gestureTemplate = gvf.getGestureTemplate(i);
        
        ofPushMatrix();
        gestureTemplate.draw(i * 100.0f, ofGetHeight() - 100.0f, 100.0f, 100.0f);
        ofPopMatrix();
        
    }
    
    
    vector< vector<float> > pp = gvf.getParticlesPositions();
    int ppSize = pp.size();
    float scale = 1;
    
    if(ppSize > 0 && currentGesture.getNumberOfTemplates() > 0){
        // as the colors show, the vector returned by getG()
        // does not seem to be in synch with the information returned by particlesPositions
        vector<int> gestureIndex = gvf.getG();
        vector<float> weights = gvf.getW();
        
        ofFill();
        
        float weightAverage = getMeanVec(weights);
        
        ofPoint offset = ofPoint(currentGesture.getTemplateNormal()[0][0] - pp[0][0], currentGesture.getTemplateNormal()[0][1] - pp[0][1]);
        
        for(int i = 0; i < ppSize; i++){
            
            // each particle position is retrieved
            ofPoint point(pp[i][0], pp[i][1]);
            
            // and then scaled and translated in order to be drawn
            //float x = ((point.x)) * (currentGesture.getMaxRange()[0] - currentGesture.getMinRange()[0]);
            //float y = ((point.y)) * (currentGesture.getMaxRange()[1] - currentGesture.getMinRange()[1]);
            float x = point.x;
            float y = point.y;
            
            // the weight of the particle is normalised
            // and then used as the radius of the circle representing the particle
            float radius = weights[i]/weightAverage;
            ofColor c = ofColor(127, 0, 0);
            
            c.setBrightness(198);
            ofSetColor(c);
            ofPushMatrix();
            ofTranslate(currentGesture.getInitialObservationRaw()[0], currentGesture.getInitialObservationRaw()[1]);
            //ofCircle(x, y, radius);
            ofCircle(x, y, 1); // MATT something wrong with radius above
            ofPopMatrix();
            
        }
    }
    
    ofSetColor(255, 255, 255);
    
    
    ostringstream os;
    os << "GESTURE VARIATION FOLLOWER 2D Example " << endl;
    os << "FPS: " << ofGetFrameRate() << endl;
    os << "GVFState: " << gvf.getStateAsString() << " ('l': learning, 'f': following, 'c': Clear)" << endl;
    os << "Gesture Recognized: " << gvf.getMostProbableGestureIndex()+1 << endl;
    
    
    float phase = 0.0f;
    float speed = 0.0f;
    float size = 0.0f;
    float angle = 0.0f;
    

    // if performing gesture in following mode, display estimated variations
    if (performingFollowing)
    {
        // get outcomes: estimations of how the gesture is performed
        outcomes = gvf.getOutcomes();
        
          if (outcomes.most_probable >= 0){
            phase = outcomes.estimations[outcomes.most_probable].phase;
            speed = outcomes.estimations[outcomes.most_probable].speed;
              size = outcomes.estimations[outcomes.most_probable].scale[0];
                          angle = outcomes.estimations[outcomes.most_probable].rotation[0];
        }
    }
  
    os << "Cursor: " << phase << " | Speed: " << speed << " | Size: " << size << " | Angle: " << angle << endl;
    
    ofDrawBitmapString(os.str(), 20, 20);
    
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key){
        case 'L':
        case 'l':
            gvf.setState(ofxGVF::STATE_LEARNING);
            break;
        case 'f':
            gvf.setState(ofxGVF::STATE_FOLLOWING);
            break;
        case 'c':
            gvf.setState(ofxGVF::STATE_CLEAR);
            break;
        case 's':
            gvf.saveTemplates("/Users/caramiaux/test.txt");
            break;
        case 'g':
            currentGesture.setType(ofxGVFGesture::GEOMETRIC);
            break;
        case 't':
            currentGesture.setType(ofxGVFGesture::TEMPORAL);
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
    inputX = x;
    inputY = y;
//    switch(gvf.getState()){
//        case ofxGVF::STATE_LEARNING:
//        {
//            currentGesture.addObservationRaw(ofPoint(x, y, 0));
//            break;
//        }
//        case ofxGVF::STATE_FOLLOWING:
//        {
//            currentGesture.addObservationRaw(ofPoint(x, y, 0));
//            gvf.infer(currentGesture.getLastRawObservation());
//            break;
//        }
//            
//        default:
//            // nothing
//            break;
//    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    switch(gvf.getState()){
        case ofxGVF::STATE_LEARNING:
        {
            currentGesture.clear();
            currentGesture.setAutoAdjustRanges(false);
            currentGesture.setMin(0.0f, 0.0f);
            currentGesture.setMax(ofGetWidth(), ofGetHeight());
            currentGesture.addObservationRaw(ofPoint(x, y, 0));
            
            inputX = x; inputY = y;
            
            performingFollowing = false;
            performingLearning = true;
            
            break;
        }
        case ofxGVF::STATE_FOLLOWING:
        {
            gvf.spreadParticles();
            currentGesture.clear();
            currentGesture.setAutoAdjustRanges(false);
            currentGesture.setMin(0.0f, 0.0f);
            currentGesture.setMax(ofGetWidth(), ofGetHeight());
            currentGesture.addObservationRaw(ofPoint(x, y, 0));
            
            inputX = x; inputY = y;
            
            performingLearning = false;
            performingFollowing = true;
            
            break;
        }
            
        default:
            // nothing
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
    performingLearning = false;
    performingFollowing = false;
    
    switch(gvf.getState()){
        case ofxGVF::STATE_LEARNING:
        {
            gvf.addGestureTemplate(currentGesture);
            currentGesture.clear();
            break;
        }
        case ofxGVF::STATE_FOLLOWING:
        {
            currentGesture.clear();
            gvf.spreadParticles();
            break;
        }
            
        default:
            // nothing
            break;
    }
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

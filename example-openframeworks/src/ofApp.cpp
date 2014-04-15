#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetLogLevel(OF_LOG_VERBOSE);
//    ofSetVerticalSync(true);
//    ofSetFrameRate(60);
    
    ofxGVFParameters parameters;
    
    parameters.inputDimensions = 2;
    parameters.numberParticles = 2000;
    parameters.tolerance = 0.2f;
    parameters.resamplingThreshold = 500;
    parameters.distribution = 0.0f;
    
    ofxGVFVarianceCoefficents coefficents;
    
    coefficents.phaseVariance = 0.00001;
    coefficents.speedVariance = 0.00001;
    coefficents.scaleVariance = 0.00001;
    coefficents.rotationVariance = 0.00001;
    
    gvf.setup(parameters, coefficents);
//    gvf.setup();
    
    ofBackground(0, 0, 0);
    
}

//--------------------------------------------------------------
void ofApp::update(){
//    if(gvf.getState() == ofxGVF::STATE_FOLLOWING) gvf.infer(currentGesture.getLastRawObservation());
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofPushMatrix();
    currentGesture.draw();
    ofPopMatrix();
    
    for(int i = 0; i < gvf.getNumGestureTemplates(); i++){
        
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
            float x = ((point.x)) * (currentGesture.getMaxRange()[0] - currentGesture.getMinRange()[0]);
            float y = ((point.y)) * (currentGesture.getMaxRange()[1] - currentGesture.getMinRange()[1]);
            
            // the weight of the particle is normalised
            // and then used as the radius of the circle representing the particle
            float radius = weights[i]/weightAverage;
            ofColor c = ofColor(127, 0, 0);
            
            c.setBrightness(198);
            ofSetColor(c);
            ofPushMatrix();
            ofTranslate(currentGesture.getInitialObservationRaw()[0], currentGesture.getInitialObservationRaw()[1]);
            ofCircle(x, y, radius);
            ofPopMatrix();
            //cout << x << " " << y << " " << radius << endl;
            
        }
    }
    
    ofSetColor(255, 255, 255);
    
    ostringstream os;
    os << "FPS: " << ofGetFrameRate() << endl;
    os << "GVFState: " << gvf.getStateAsString() << endl;
    os << "Most probable index: " << gvf.getMostProbableGestureIndex() << endl;
    
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
    switch(gvf.getState()){
        case ofxGVF::STATE_LEARNING:
        {
            currentGesture.addObservationRaw(ofPoint(x, y, 0));
            break;
        }
        case ofxGVF::STATE_FOLLOWING:
        {
            currentGesture.addObservationRaw(ofPoint(x, y, 0));
            gvf.infer(currentGesture.getLastRawObservation());
            break;
        }
            
        default:
            // nothing
            break;
    }
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
            break;
        }
            
        default:
            // nothing
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
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

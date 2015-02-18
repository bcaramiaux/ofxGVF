#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofBackground(0,0,0);
    ofSetWindowTitle("openNiConductor");
    
    //Kinect
    kinect.setup();
    kinect.addDepthGenerator();
    kinect.addImageGenerator();
    kinect.setRegister(false);
    kinect.setMirror(true);
    kinect.addUserGenerator();
    kinect.setMaxNumUsers(3);
    kinect.setSkeletonProfile(XN_SKEL_PROFILE_ALL);
    kinect.start();
    hadUsers = false;
    
    // CONFIGURATION of the GVF
    velocityThreshold = 30.0;
    config.inputDimensions = 3;
    config.translate       = true;
    //config.segmentation    = false;
    
    // PARAMETERS are set by default
    
    // CREATE the corresponding GVF
    gvfRight.setup(config);
    gvfLeft.setup(config);
    
    ofBackground(0, 0, 0);
    performingLearningRight = performingLearningLeft = performingFollowingRight = performingFollowingLeft = false;
    //performingFollowing = false;
    
    /*
     templateFile = ofToDataPath("templates.txt");
     if( ofFile::doesFileExist(templateFile) ) {
     gvf.loadTemplates(templateFile);
     }
     */
}

//--------------------------------------------------------------
void ofApp::update(){
    kinect.update();
    checkKinectReset();
    if (kinect.getNumTrackedUsers()) {
        ofxOpenNIUser user = kinect.getTrackedUser(0);
        
        //The following "if" statement is a hard-coded alternative for if(kinect.getUserGenerator().IsNewDataAvailable()), which doesn't work properly in ofxOpenNI
        if (user.getJoint((Joint)0).getWorldPosition() != ofPoint(0,0,0) &&
            (!featExtractor.skeletonExists(0) ||
             user.getJoint((Joint)0).getWorldPosition() != featExtractor.getSkeleton(0)->getPosition(0) )) {
                map<int, ofPoint> joints;
                for (int j = 0; j < user.getNumJoints(); j++) {
                    joints[j] = user.getJoint((Joint)j).getWorldPosition();
                }
                featExtractor.updateSkeleton(0, joints);
            }
    }
    
    if (featExtractor.skeletonExists(0)) {
        //Right hand
        //Gesture being performed when hand moves at higher speed than velocityThreshold
        if (featExtractor.getSkeleton(0)->getVelocityMean(JOINT_RIGHT_HAND) > velocityThreshold) {
            if (gvfRight.getState() == ofxGVF::STATE_LEARNING){
                if (!performingLearningRight) { //init
                    currentGestureRight.setAutoAdjustRanges(false);
                    currentGestureRight.setNumberDimensions(3);
                    currentGestureRight.setMin(-1500.0f, -1500.0f, 0.0f);
                    currentGestureRight.setMax(1500.0f, 1500.0f, 4000.0f);
                }
                performingLearningRight = true;
                performingFollowingRight = false;
            } else if (gvfRight.getState() == ofxGVF::STATE_FOLLOWING){
                if (!performingFollowingRight) { //init
                    currentGestureRight.setAutoAdjustRanges(false);
                    currentGestureRight.setNumberDimensions(3);
                    currentGestureRight.setMin(-1500.0f, -1500.0f, 0.0f);
                    currentGestureRight.setMax(1500.0f, 1500.0f, 4000.0f);
                }
                performingLearningRight = false;
                performingFollowingRight = true;
            }
        } else { //hand "not moving"
            if (performingLearningRight) {
                gvfRight.addGestureTemplate(currentGestureRight);
                currentGestureRight.clear();
            } else if (performingFollowingRight){
                currentGestureRight.clear();
                gvfRight.spreadParticles();
            }
            performingFollowingRight = false;
            performingLearningRight = false;
        }
        
        switch(gvfRight.getState()){
            case ofxGVF::STATE_LEARNING:
            {
                if (performingLearningRight){
                    //currentGestureRight.addObservationRaw(featExtractor.getRelativePositionToTorso(JOINT_RIGHT_HAND));
                    ofPoint observation = featExtractor.getSkeleton(0)->getPositionFiltered(JOINT_RIGHT_HAND);
                    observation.y = -observation.y;
                    currentGestureRight.addObservationRaw(featExtractor.getSkeleton(0)->getPositionFiltered(JOINT_RIGHT_HAND));
                }
                break;
            }
            case ofxGVF::STATE_FOLLOWING:
            {
                if (performingFollowingRight){
                    ofPoint observation = featExtractor.getSkeleton(0)->getPositionFiltered(JOINT_RIGHT_HAND);
                    observation.y = -observation.y;
                    currentGestureRight.addObservationRaw(featExtractor.getSkeleton(0)->getPositionFiltered(JOINT_RIGHT_HAND));
                    gvfRight.infer(currentGestureRight.getLastRawObservation());
                }
                break;
            }
                
            default:
                // nothing
                break;
        }
        
        
        //Left hand
        //Gesture being performed when hand moves at higher speed than velocityThreshold
        if (featExtractor.getSkeleton(0)->getVelocityMean(JOINT_LEFT_HAND) > velocityThreshold) {
            if (gvfLeft.getState() == ofxGVF::STATE_LEARNING){
                if (!performingLearningLeft) { //init
                    currentGestureLeft.setAutoAdjustRanges(false);
                    currentGestureLeft.setNumberDimensions(3);
                    currentGestureLeft.setMin(-1500.0f, -1500.0f, 0.0f);
                    currentGestureLeft.setMax(1500.0f, 1500.0f, 4000.0f);
                }
                performingLearningLeft = true;
                performingFollowingLeft = false;
            } else if (gvfLeft.getState() == ofxGVF::STATE_FOLLOWING){
                if (!performingFollowingLeft) { //init
                    currentGestureLeft.setAutoAdjustRanges(false);
                    currentGestureLeft.setNumberDimensions(3);
                    currentGestureLeft.setMin(-1500.0f, -1500.0f, 0.0f);
                    currentGestureLeft.setMax(1500.0f, 1500.0f, 4000.0f);
                }
                performingLearningLeft = false;
                performingFollowingLeft = true;
            }
        } else { //hand "not moving"
            if (performingLearningLeft) {
                gvfLeft.addGestureTemplate(currentGestureLeft);
                currentGestureLeft.clear();
            } else if (performingFollowingLeft){
                currentGestureLeft.clear();
                gvfLeft.spreadParticles();
            }
            performingFollowingLeft = false;
            performingLearningLeft = false;
        }
        
        switch(gvfLeft.getState()){
            case ofxGVF::STATE_LEARNING:
            {
                if (performingLearningLeft){
                    //currentGestureRight.addObservationRaw(featExtractor.getRelativePositionToTorso(JOINT_RIGHT_HAND));
                    ofPoint observation = featExtractor.getSkeleton(0)->getPositionFiltered(JOINT_LEFT_HAND);
                    observation.y = -observation.y;
                    currentGestureLeft.addObservationRaw(featExtractor.getSkeleton(0)->getPositionFiltered(JOINT_LEFT_HAND));
                }
                break;
            }
            case ofxGVF::STATE_FOLLOWING:
            {
                if (performingFollowingLeft){
                    ofPoint observation = featExtractor.getSkeleton(0)->getPositionFiltered(JOINT_LEFT_HAND);
                    observation.y = -observation.y;
                    currentGestureLeft.addObservationRaw(featExtractor.getSkeleton(0)->getPositionFiltered(JOINT_LEFT_HAND));
                    gvfLeft.infer(currentGestureLeft.getLastRawObservation());
                }
                break;
            }
                
            default:
                // nothing
                break;
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    kinect.drawImage(0, 200, 680, 510);
    kinect.drawSkeletons(0, 200, 680, 510);
    
    //Templates RH
    for(int i = 0; i < gvfRight.getNumberOfGestureTemplates(); i++){
        ofxGVFGesture & gestureTemplate = gvfRight.getGestureTemplate(i);
        //gestureTemplate.draw(i * 80.0f, ofGetHeight() - 160.0f, 80.0f, 80.0f);
        gestureTemplate.draw(ofGetWidth() - 120, 20+ i*100, 100.0f, 100.0f);
    }
    
    //Templates LH
    for(int i = 0; i < gvfLeft.getNumberOfGestureTemplates(); i++){
        ofxGVFGesture & gestureTemplate = gvfLeft.getGestureTemplate(i);
        //gestureTemplate.draw(i * 80.0f, ofGetHeight() - 80.0f, 80.0f, 80.0f);
        gestureTemplate.draw(ofGetWidth() - 220, 20 + i*100, 100.0f, 100.0f);
    }
    
    
    //RH current gesture
    ofMesh gestureRightMesh;
    gestureRightMesh.setMode(OF_PRIMITIVE_LINE_STRIP);
    if (currentGestureRight.getRepresentationsRaw().size()) {
        ofMesh rawMesh = currentGestureRight.getRepresentationsRaw()[0][0];
        for (int i = 0; i < rawMesh.getNumVertices(); i++) {
            ofPoint vertex = rawMesh.getVertex(i);
            gestureRightMesh.addVertex(ofPoint(kinect.worldToProjective(vertex).x, kinect.worldToProjective(vertex).y, 0.0));
        }
    }
    
    ofPushMatrix();
    ofTranslate(0, 200);
    ofSetColor(ofColor::red);
    gestureRightMesh.draw();
    ofPopMatrix();
    
    //LH current gesture
    ofMesh gestureLeftMesh;
    gestureLeftMesh.setMode(OF_PRIMITIVE_LINE_STRIP);
    if (currentGestureLeft.getRepresentationsRaw().size()) {
        ofMesh rawMesh = currentGestureLeft.getRepresentationsRaw()[0][0];
        for (int i = 0; i < rawMesh.getNumVertices(); i++) {
            ofPoint vertex = rawMesh.getVertex(i);
            gestureLeftMesh.addVertex(ofPoint(kinect.worldToProjective(vertex).x, kinect.worldToProjective(vertex).y, 0.0));
        }
    }
    
    ofPushMatrix();
    ofTranslate(0, 200);
    ofSetColor(ofColor::blueSteel);
    gestureLeftMesh.draw();
    ofPopMatrix();
    
    //RH particles
    vector< vector<float> > pp = gvfRight.getParticlesPositions();
    int ppSize = pp.size();
    float scale = 1;
    
    if(performingFollowingRight && ppSize > 0 && currentGestureRight.getNumberOfTemplates() > 0){
        // as the colors show, the vector returned by getG()
        // does not seem to be in synch with the information returned by particlesPositions
        vector<int> gestureIndex = gvfRight.getG();
        vector<float> weights = gvfRight.getW();
        ofFill();
        float weightAverage = getMeanVec(weights);
        ofPoint offset = ofPoint(currentGestureRight.getTemplateNormal()[0][0] - pp[0][0], currentGestureRight.getTemplateNormal()[0][1] - pp[0][1]);
        
        for(int i = 0; i < ppSize; i++){
            // each particle position is retrieved
            ofPoint point(pp[i][0], pp[i][1], pp[i][3]);
            
            point.x += currentGestureRight.getInitialObservation()[0];
            point.y += currentGestureRight.getInitialObservation()[1];
            point.z += currentGestureRight.getInitialObservation()[2];
            
            float x = kinect.worldToProjective(point).x;
            float y = kinect.worldToProjective(point).y;
            
            // the weight of the particle is normalised
            // and then used as the radius of the circle representing the particle
            float radius = weights[i]/weightAverage;
            ofColor c = ofColor(255, 160, 122, 120);
            c.setBrightness(198);
            ofSetColor(c);
            ofPushMatrix();
            ofTranslate(0, 200);
            ofCircle(x, y, radius);
            ofPopMatrix();
        }
    }
    
    //LH particles
    pp = gvfLeft.getParticlesPositions();
    ppSize = pp.size();
    
    if(performingFollowingLeft && ppSize > 0 && currentGestureLeft.getNumberOfTemplates() > 0){
        // as the colors show, the vector returned by getG()
        // does not seem to be in synch with the information returned by particlesPositions
        vector<int> gestureIndex = gvfLeft.getG();
        vector<float> weights = gvfLeft.getW();
        ofFill();
        float weightAverage = getMeanVec(weights);
        ofPoint offset = ofPoint(currentGestureLeft.getTemplateNormal()[0][0] - pp[0][0], currentGestureLeft.getTemplateNormal()[0][1] - pp[0][1]);
        
        for(int i = 0; i < ppSize; i++){
            // each particle position is retrieved
            ofPoint point(pp[i][0], pp[i][1], pp[i][3]);
            
            point.x += currentGestureLeft.getInitialObservation()[0];
            point.y += currentGestureLeft.getInitialObservation()[1];
            point.z += currentGestureLeft.getInitialObservation()[2];
            
            float x = kinect.worldToProjective(point).x;
            float y = kinect.worldToProjective(point).y;
            
            // the weight of the particle is normalised
            // and then used as the radius of the circle representing the particle
            float radius = weights[i]/weightAverage;
            ofColor c = ofColor(32,178,170, 120);
            c.setBrightness(198);
            ofSetColor(c);
            ofPushMatrix();
            ofTranslate(0, 200);
            ofCircle(x, y, radius);
            ofPopMatrix();
        }
    }
    
    ofSetColor(0, 255, 0);
    ofNoFill();
    
    //RH performed gesture
    if (performingFollowingRight) {
        ofRect(ofGetWidth() - 120, 20+ gvfRight.getMostProbableGestureIndex() * 100, 100.0f, 100.0f);
    }
    
    //LH performed gesture
    if (performingFollowingLeft) {
        ofRect(ofGetWidth() - 220, 20+ gvfLeft.getMostProbableGestureIndex() * 100, 100.0f, 100.0f);
    }
    
    ofSetColor(255, 255, 255);
    
    ostringstream os;
    os << "GESTURE VARIATION FOLLOWER Kinect Example " << endl;
    os << "FPS: " << ofGetFrameRate() << endl;
    os << "GVFState: " << gvfRight.getStateAsString() << " ('l': learning, 'f': following, 'c': Clear)" << endl << endl;
    os << "RIGHT HAND" << endl;
    os << "Gesture Recognized: " << gvfRight.getMostProbableGestureIndex()+1 << endl;
    
    float phase = 0.0f;
    float speed = 0.0f;
    float size = 0.0f;
    float angle = 0.0f;
    
    // if performing gesture in following mode, display estimated variations
    if (performingFollowingRight)
    {
        // get outcomes: estimations of how the gesture is performed
        outcomes = gvfRight.getOutcomes();
        
        if (outcomes.most_probable >= 0){
            phase = outcomes.estimations[outcomes.most_probable].phase;
            speed = outcomes.estimations[outcomes.most_probable].speed;
            size = outcomes.estimations[outcomes.most_probable].scale[0];
            angle = outcomes.estimations[outcomes.most_probable].rotation[0];
        }
    }
    
    os << "Cursor: " << phase << " | Speed: " << speed << " | Size: " << size << " | Angle: " << angle << endl;
    
    os << "LEFT HAND" << endl;
    os << "Gesture Recognized: " << gvfRight.getMostProbableGestureIndex()+1 << endl;
    
    phase = 0.0f;
    speed = 0.0f;
    size = 0.0f;
    angle = 0.0f;
    
    // if performing gesture in following mode, display estimated variations
    if (performingFollowingLeft)
    {
        // get outcomes: estimations of how the gesture is performed
        outcomes = gvfLeft.getOutcomes();
        
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
            gvfRight.setState(ofxGVF::STATE_LEARNING);
            gvfLeft.setState(ofxGVF::STATE_LEARNING);
            break;
        case 'f':
            gvfRight.setState(ofxGVF::STATE_FOLLOWING);
            gvfLeft.setState(ofxGVF::STATE_FOLLOWING);
            break;
        case 'c':
            gvfRight.setState(ofxGVF::STATE_CLEAR);
            gvfLeft.setState(ofxGVF::STATE_CLEAR);
            break;
        case 's':
            //gvf.saveTemplates(templateFile);
            break;
        case 'g':
            currentGestureRight.setType(ofxGVFGesture::GEOMETRIC);
            currentGestureLeft.setType(ofxGVFGesture::GEOMETRIC);
            break;
        case 't':
            currentGestureRight.setType(ofxGVFGesture::TEMPORAL);
            currentGestureLeft.setType(ofxGVFGesture::GEOMETRIC);
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
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
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

//--------------------------------------------------------------
void ofApp::exit(){
    kinect.stop();
}

//--------------------------------------------------------------
void ofApp::checkKinectReset(){
    if (kinect.getNumTrackedUsers()) {
        hadUsers = true;
    } else if (!kinect.getNumTrackedUsers() && hadUsers){
        hadUsers = false;
        kinect.setPaused(true);
        kinect.removeUserGenerator();
        kinect.addUserGenerator();
        kinect.setPaused(false);
    }
}

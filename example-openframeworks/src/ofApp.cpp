#include "ofApp.h"

string testString;

//--------------------------------------------------------------
void ofApp::setup(){

	ofSetCircleResolution(50);
    
	ofSetWindowTitle("openframeworks gvf visualiser");
    ofSetWindowShape(1024, 768);
    
	ofSetFrameRate(60); // if vertical sync is off, we can go a bit fast... this caps the framerate at 60fps.
    
    ofPoint wSize = ofGetWindowSize();
    scrW = wSize.x;
    scrH = wSize.y;
    printf("w: %d h: %d\n", scrW, scrH);
    
    drawArea = ofRectangle(ofPoint(0, 0), ofGetWindowWidth(), ofGetWindowHeight());
    
    currentGesture.setDrawArea(drawArea);
    
    isMouseDrawing = false;
    
    viewYRotation = 0;
    viewXRotation = 0;
    
    initializeGui();
    initColors();
    
}

//--------------------------------------------------------------
void ofApp::update(){

    // if the user is performing a gesture,
    // feed the last point on the gesture to the gvf handler
    // (depending on the speed the user is performing the gesture,
    // the same point might be fed several times)
    if(isMouseDrawing)
    {
        gvfh.gvf_data(currentGesture.getLastPointAdded());
  }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    float templatesScale = 0.5f;
    ofBackgroundGradient(ofColor(2), ofColor(40), OF_GRADIENT_CIRCULAR);
    ofPushMatrix();
    
    gui.draw();
    
    if(rotate)
    {
        // rotate gesture related graphics around the center of the screen
        ofTranslate(scrW/2, scrH/2);
        ofRotateY(viewYRotation);
        ofRotateX(viewXRotation);
        ofTranslate(-scrW/2, -scrH/2);
    }
    
    // draw the current templates on a small scale
    gvfh.drawTemplates(templatesScale);
    
    if(gvfh.get_state() != STATE_FOLLOWING && isMouseDrawing)
        currentGesture.draw();
    else if(displayCurrentGesture)
        currentGesture.draw();
    
    ofDisableAlphaBlending();

    // string used to comunicate to the user of possible commands and of the current state of the application
    string state_string;
    state_string.append("'l' to learn a new template\n'c' to clear\n"
                        "numbers 1 to 4 to toggle visual feedback "
                        "(1 - particles; 2 - template; 3 - estimated gesture; 4 - current gesture)"
                        "\nSTATE_LEARINING [");
    
    int state = gvfh.get_state();
    if(state == STATE_FOLLOWING){
        state_string.append(" ]\nSTATE_FOLLOWING [X]\nSTATE_CLEAR     [ ]");
        
        if(displayParticles)
            gvfh.printParticleInfo(currentGesture);

        // temp will have the partial representation of how gvf is recognising the gesture being performed
        gvfGesture temp = gvfh.getRecognisedGestureRepresentation();
        if(temp.isValid)
        {
            // the estimated gesture will be drawn on the same area
            // as the gesture being performed and starting on the same point
            ofRectangle da = currentGesture.getDrawArea();
            ofPoint p = currentGesture.getInitialOfPoint();
            
            if(displayEstimatedGesture)
            {
                temp.setAppearance(ofColor(0,255,0), 5, 255, 180, 1);
                temp.draw(templatesScale);
                temp.setDrawArea(da);
                temp.setInitialPoint(p + 1);
                temp.centraliseDrawing = false;
                temp.draw();
            }
            
            if(displayTemplate)
            {
                // draw the original template for the gesture being recognised
                gvfGesture g = gvfh.getTemplateGesture(gvfh.getIndexMostProbable());
                g.setDrawArea(da);
                g.setInitialPoint(p + 1);
                g.centraliseDrawing = false;

                // the template's opacity is determined by how probable the recognition is
                g.setAppearance(g.getColor(),
                                1.5, 255, 50,
                                ofLerp(1/gvfh.getTemplateCount(),
                                1,
                                gvfh.mygvf->getMostProbableProbability()));
               g.draw();
            }
        }
        
    }else if (state == STATE_LEARNING)
        state_string.append("X]\nSTATE_FOLLOWING [ ]\nSTATE_CLEAR     [ ]");
    else
        state_string.append(" ]\nSTATE_FOLLOWING [ ]\nSTATE_CLEAR     [X]");
    
    ofPopMatrix();
    
    ofSetColor(198);
    
    ofDrawBitmapString(state_string.c_str(), 30, 25);

    // show the current frame rate
    ofDrawBitmapString("FPS " + ofToString(ofGetFrameRate(), 0), ofGetWidth() - 200, 25);
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	if (key == 'l' || key == 'L'){
        // get ready to learn a new gesture
        // (will not start to learn a new gesture
        // if the user is in the middle of a gesture
        // or if the state is already STATE_LEARNING)
		if(gvfh.get_state() != STATE_LEARNING && !isMouseDrawing)
        {
            gvfh.gvf_learn();
        }
	}
    else if(key == 'c' || key == 'C')
    {
        gvfh.gvf_clear();
        initColors();
    }
    else if (key == 'r' || key == 'R')
    {
        rotate = !rotate;
        viewYRotation = 0;
        viewXRotation = 0;
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
void ofApp::mouseMoved(int x, int y ){
    // if rotating, the mouse will determing how many degrees
    if(rotate)
    {
        viewYRotation = ofMap(x, 0, ofGetWidth(), -90, 90);
        viewXRotation = ofMap(y, 0, ofGetHeight(), 90, -90);
    }
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
    // if a gesture has already been starded, a new point is added to it
    if(isMouseDrawing)
    {
        currentGesture.addNonNormalisedPoint(x, y);
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

    // a new gesture will not start if the user clicks on the UI area or outside the gesture area
    if(currentGesture.isPointInGestureArea(x, y) && !currentGesture.isPointInArea(gui.getShape(), x, y))
    {
        // the current gesture is initialised with its initial point
        currentGesture.initialiseNonNormalised(x, y);
        
        // here the point is already normalised
        ofPoint initialPoint = currentGesture.getInitialOfPoint();
        
        if(gvfh.get_state() == STATE_LEARNING)
        {
            gvfh.addTemplateGesture(initialPoint, generateRandomColor());
        }

        // first point is always 0.5 (the gesture's initialPoint property can be used to translate the gesture)
        currentGesture.addPoint(ofPoint(0.5, 0.5));

        isMouseDrawing = true;   

    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    if(isMouseDrawing)
    {
        isMouseDrawing = false;
    }
    
    // if a gesture has just been learnt, automatically switches the state to following
    if(gvfh.get_state() == STATE_LEARNING)
        gvfh.gvf_follow();
    
    if(gvfh.get_state() == STATE_FOLLOWING)
        gvfh.gvf_restart();
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    ofPoint wSize = ofGetWindowSize();
    scrW = wSize.x;
    scrH = wSize.y;
    printf("w: %d h: %d\n", scrW, scrH);
    
    // resets the current gesture's draw area with the new size
    drawArea = ofRectangle(ofPoint(0, 0), ofGetWindowWidth(), ofGetWindowHeight());

    currentGesture.setDrawArea(drawArea);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

//--------------------------------------------------------------
void ofApp::initializeGui(){
    
    // the initial parameters values set here are not connected to the initial parameters values on the gvfhandler constructor
    // those are the real initial values. The values here will only take effect once the user changes them
    // (this is inconsistent and should be fixed)
    
    gui.setup("ofxGVF Settings");
    
    gui.add(numParticles.set("Number of particles", 2000, 10, 10000));
    gui.add(resampleThreshold.set("Resampling threshold", 1000, 100, 10000));
    gui.add(smoothingCoefficient.set("Smoothing coefficient", 0.2, 0.01, 2.0));
    
    gui.add(label.setup("Variance coefficients", ""));
    gui.add(sigPosition.set("Position", 0.0001, 0.000001, 0.1));
    gui.add(sigSpeed.set("Speed", 0.01, 0.000001, 0.1));
    gui.add(sigScale.set("Scale", 0.0001, 0.000001, 0.1));
    gui.add(sigRotation.set("Rotation", 0.0001, 0.000001, 0.1));
    
    gui.add(save.setup("save gestures"));
    gui.add(load.setup("load gestures"));
    
    numParticles.addListener(this, &ofApp::numParticlesChanged);
    resampleThreshold.addListener(this, &ofApp::resampleThresholdChanged);
    smoothingCoefficient.addListener(this, &ofApp::smoothingCoefficientChanged);

    sigPosition.addListener(this, &ofApp::varianceCoefficentsChanged);
    sigSpeed.addListener(this, &ofApp::varianceCoefficentsChanged);
    sigScale.addListener(this, &ofApp::varianceCoefficentsChanged);
    sigRotation.addListener(this, &ofApp::varianceCoefficentsChanged);
    
	save.addListener(this, &ofApp::saveGestures);
    load.addListener(this, &ofApp::loadGestures);
    
    gui.setShape(ofRectangle(30, 110, 250, 100));
    gui.setPosition(30, 110);
    gui.setWidthElements(250);
    
}

//--------------------------------------------------------------
void ofApp::numParticlesChanged(int & numParticles){
	//cout << numParticles << endl;
    gvfh.setNumberOfParticles(numParticles);
}

void ofApp::resampleThresholdChanged(int & resampleThreshold){
	//cout << resampleThreshold << endl;
    gvfh.gvf_rt(resampleThreshold);
}

void ofApp::smoothingCoefficientChanged(float & smoothingCoefficient){
	//cout << smoothingCoefficient << endl;
    gvfh.gvf_std(smoothingCoefficient);
}

void ofApp::varianceCoefficentsChanged(float & coefficent){
	//cout << coefficent << endl;
    // just get all the coefficients and ignore the passed value
    std::vector<float> sigs;
    sigs.push_back(sigPosition.get());
    sigs.push_back(sigSpeed.get());
    sigs.push_back(sigScale.get());
    sigs.push_back(sigRotation.get());
    gvfh.gvf_adaptspeed(sigs);
}

//--------------------------------------------------------------
void ofApp::loadGestures(){
    
    ofFileDialogResult dialogResult = ofSystemLoadDialog("Select the xml file containing gesture data");
    if(!dialogResult.bSuccess) return;
    
    ofxXmlSettings file;
    if(!file.loadFile(dialogResult.filePath))
        return;
    
    gvfh.gvf_clear();
    initColors();
    
    int gestureCount = file.getNumTags("GESTURE");
    if(gestureCount < 1)
        return;
    
    cout << gestureCount << " gestures." << endl;
    
    for(int i = 0; i < gestureCount; i++)
    {
        file.pushTag("GESTURE", i);

        ofPoint p;
        p.x = file.getValue("INIT_POINT:X", (double)-1);
        p.y = file.getValue("INIT_POINT:Y", (double)-1);
        
        file.pushTag("POINTS");
            int pointCount = file.getNumTags("PT");
            if(pointCount < 1)
                return;

            gvfh.gvf_learn();        
            gvfh.addTemplateGesture(p, generateRandomColor());
        
        for(int j = 0; j < pointCount; j++)
        {
            p.x = file.getValue("PT:X", (double)-1, j);
            p.y = file.getValue("PT:Y", (double)-1, j);
            gvfh.gvf_data(p);
        }
        file.popTag();
        file.popTag();
        gvfh.gvf_follow();
    }
}

//--------------------------------------------------------------
void ofApp::saveGestures(){
    
    ofFileDialogResult dialogResult = ofSystemSaveDialog("my gestures.xml", "Save gestures");
    if(!dialogResult.bSuccess) return;
    
    ofxXmlSettings file;
    cout << dialogResult.filePath << endl;
    cout << dialogResult.fileName << endl;
    int templateCount = gvfh.getTemplateCount();
    for(int i = 0; i < templateCount; i++)
    {
        gvfGesture g = gvfh.getTemplateGesture(i);
        ofPoint initialPoint = g.getInitialOfPoint();
        int currentGesture = file.addTag("GESTURE");
        file.pushTag("GESTURE", currentGesture);
        file.addTag("INIT_POINT");
        file.pushTag("INIT_POINT");
        file.addValue("X", initialPoint.x);
        file.addValue("Y", initialPoint.y);
        file.popTag();
        file.addTag("POINTS");
        file.pushTag("POINTS");
        std::vector<std::vector<float> > allPoints = g.getData();
        int pointCount = allPoints.size();
        for(int j = 0; j < pointCount; j++)
        {
            int tagNum = file.addTag("PT");
            file.setValue("PT:X", allPoints[j][0], tagNum);
            file.setValue("PT:Y", allPoints[j][1], tagNum);
        }
        file.popTag();
        file.popTag();
    }
    file.saveFile(dialogResult.filePath);
}

//--------------------------------------------------------------
void ofApp::initColors(){
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

//--------------------------------------------------------------
ofColor ofApp::generateRandomColor(){
    ofColor c;
    
    if(colors.size() == 0)
        initColors();
    
    int colorsRemaining = colors.size();
    
    int index = ofRandom(0, colorsRemaining - 1);
    cout << index << endl;
    c = colors[index];
    colors.erase(colors.begin() + index);
    return c;
}
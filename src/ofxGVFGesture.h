//
//  ofxGVFGesture.h
//  ofxGVF-Example
//
//  Created by gameover on 10/12/13.
//
//

#ifndef _H_OFXGVF_GESTURE
#define _H_OFXGVF_GESTURE

#include "ofxGVFTypes.h"
#include <assert.h>


/* Macros for min/max. */
#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif /* MIN */
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif	/* MAX */



#if OPENFRAMEWORKS
#include "ofMain.h"
#endif

class ofxGVFGesture{
    
public:
    
    enum ofxGVFGestureType{
        GEOMETRIC,
        TEMPORAL
    };
    
    ofxGVFGesture(){
        inputDimensions = 2; // default to 2D
        type = GEOMETRIC; // default to a geometric shape
    
//        bAutoAdjustNormalRange = true;
//        bIsRangeMinSet = false;
//        bIsRangeMaxSet = false;

        setAutoAdjustRanges(true);

        
        //added
        templatesRaw    = vector<vector<vector<float > > >();
        templatesNormal = vector<vector<vector<float > > >();
        
        clear();
    }
    
    ofxGVFGesture(int _inputdim){
        inputDimensions = _inputdim;
        if (inputDimensions == 2 || inputDimensions == 3)
            type = GEOMETRIC;
        else
            type = TEMPORAL;
        
        //        bAutoAdjustNormalRange = true;
        //        bIsRangeMinSet = false;
        //        bIsRangeMaxSet = false;
        
        setAutoAdjustRanges(true);
        
        //added
        templatesRaw    = vector<vector<vector<float > > >();
        templatesNormal = vector<vector<vector<float > > >();
        
        clear();
    }

    ~ofxGVFGesture(){
        clear();
    }
    
    void setName(string name){
        this->name = name;
    }
    
    string getName(){
        return name;
    }
    
    void setType(ofxGVFGestureType type){
        this->type = type;
        normalise();
    }
    
    ofxGVFGestureType getType(){
        return type;
    }
    
    void setNumberDimensions(int dimensions){
        assert(dimensions > 0);
        inputDimensions = dimensions;
    }
    
    void setAutoAdjustRanges(bool b){
        if(b) bIsRangeMinSet = bIsRangeMaxSet = false;
        bAutoAdjustNormalRange = b;
    }
    
#if OPENFRAMEWORKS
    
    void setMax(ofPoint max){
        assert(inputDimensions <= 2);
        vector<float> r;
        if(inputDimensions == 2){
            r.resize(2);
            r[0] = max.x; r[1] = max.y;
        }
        if(inputDimensions == 3){
            r.resize(3);
            r[0] = max.x; r[1] = max.y; r[2] = max.z;
        }
        setMaxRange(r);
    }
    
    void setMin(ofPoint min){
        assert(inputDimensions <= 2);
        vector<float> r;
        if(inputDimensions == 2){
            r.resize(2);
            r[0] = min.x; r[1] = min.y;
        }
        if(inputDimensions == 3){
            r.resize(3);
            r[0] = min.x; r[1] = min.y; r[2] = min.z;
        }
        setMinRange(r);
    }
    
#endif
    
    void setMax(float x, float y){
        assert(inputDimensions == 2);
        vector<float> r(2);
        r[0] = x; r[1] = y;
        setMaxRange(r);
    }
    
    void setMin(float x, float y){
        assert(inputDimensions == 2);
        vector<float> r(2);
        r[0] = x; r[1] = y;
        setMinRange(r);
    }
    
    void setMax(float x, float y, float z){
        assert(inputDimensions == 3);
        vector<float> r(3);
        r[0] = x; r[1] = y; r[2] = z;
        setMaxRange(r);
    }
    
    void setMin(float x, float y, float z){
        assert(inputDimensions == 3);
        vector<float> r(3);
        r[0] = x; r[1] = y; r[2] = z;
        setMinRange(r);
    }
    
    void setMaxRange(vector<float> observationRangeMax){
        this->observationRangeMax = observationRangeMax;
        bIsRangeMaxSet = true;
        normalise();
    }
    
    void setMinRange(vector<float> observationRangeMin){
        this->observationRangeMin = observationRangeMin;
        bIsRangeMinSet = true;
        normalise();
    }
    
    vector<float>& getMaxRange(){
        return observationRangeMax;
    }
    
    vector<float>& getMinRange(){
        return observationRangeMin;
    }
    
    void autoAdjustMinMax(vector<float> & observation){
        if(observationRangeMax.size()  < inputDimensions){
            observationRangeMax.assign(inputDimensions, -INFINITY);
            observationRangeMin.assign(inputDimensions,  INFINITY);
        }
        for(int i = 0; i < inputDimensions; i++){
            observationRangeMax[i] = MAX(observationRangeMax[i], observation[i]);
            observationRangeMin[i] = MIN(observationRangeMin[i], observation[i]);
        }
    }
    
#if OPENFRAMEWORKS
    void addObservationRaw(ofPoint observation, int templateIndex = 0){
        assert(inputDimensions <= 3);
        vector<float> obs(inputDimensions);
        for(int i = 0; i < inputDimensions; i++){
            obs[i] = observation[i];
        }
        addObservationRaw(obs, templateIndex);
    }
#endif
    
    void addObservation(vector<float> observation, int templateIndex = 0){
        //post("size %i",observation.size());
        if (observation.size()!=inputDimensions)
            inputDimensions=observation.size();
        addObservationRaw(observation);
    }
    
    
    void addObservationRaw(vector<float> observation, int templateIndex = 0){
        
        //post("%i %i",templateIndex,templateInitialRaw.size());
        
        
        // check we have a valid templateIndex and correct number of input dimensions
        assert(templateIndex <= templatesRaw.size());
        assert(observation.size() == inputDimensions);
        //assert(bAutoAdjustNormalRange || (bIsRangeMaxSet && bIsRangeMinSet));
        
        // if the template index is same as the number of temlates make a new template
        if(templateIndex == templatesRaw.size()){ // make a new template
            
            // reserve space in raw and normal template storage
            templatesRaw.resize(templatesRaw.size() + 1);
            templatesNormal.resize(templatesNormal.size() + 1);
            
        }

        if(templatesRaw[templateIndex].size() == 0)
            templateInitialRaw = templateInitialNormal = observation;

        for(int j = 0; j < observation.size(); j++)
            observation[j] = observation[j] - templateInitialRaw[j];

        //            cout << "THEN " << observation.size() << " | " << observation[0] << " " << observation[1] << " " << templateInitialRaw[0] << " " << templateInitialRaw[1] << endl;
        
        // store the raw observation
        templatesRaw[templateIndex].push_back(observation);
        
        //post("SIZE RAW %i",templatesRaw[0].size());
        
        // if set let's auto size the range for normalising
        if(bAutoAdjustNormalRange) autoAdjustMinMax(observation);
        
        normalise();
         
        
    }
    

    
    void normalise(){
        

#if OPENFRAMEWORKS
        // reserve space for raw and normal meshes
        representationsNormal.resize(templatesRaw.size() + 1);
        representationsRaw.resize(templatesRaw.size() + 1);
#endif 
        
        templatesNormal.resize(templatesRaw.size());
        
        for(int t = 0; t < templatesRaw.size(); t++){
#if OPENFRAMEWORKS         
            if(type == GEOMETRIC){
                
                // for GEOMETRIC representations let's use a single mesh with n-Dimensions
                
                representationsNormal[t].resize(1);
                representationsNormal[t][0].setMode(OF_PRIMITIVE_LINE_STRIP);
                representationsRaw[t].resize(1);
                representationsRaw[t][0].setMode(OF_PRIMITIVE_LINE_STRIP);
                
            }else{
                
                // for TEMPORAL representations let's use a mesh for EACH of the n-Dimensions
                
                representationsNormal[t].resize(inputDimensions);
                representationsRaw[t].resize(inputDimensions);
                for(int i = 0; i < inputDimensions; i++){
                    representationsNormal[t][i].setMode(OF_PRIMITIVE_LINE_STRIP);
                    representationsRaw[t][i].setMode(OF_PRIMITIVE_LINE_STRIP);
                }
            }
      
            for(int m = 0; m < representationsNormal[t].size(); m++){
                representationsNormal[t][m].clear();
                representationsRaw[t][m].clear();
            }
#endif  
            
            templatesNormal[t].resize(templatesRaw[t].size());
            
            for(int o = 0; o < templatesRaw[t].size(); o++){
#if OPENFRAMEWORKS
                ofPoint pN;
                ofPoint pR;
#endif
                templatesNormal[t][o].resize(inputDimensions);
                
                for(int d = 0; d < inputDimensions; d++){
                    
                    //cout << d << " " << templatesNormal[t][o][d] << " " << templatesRaw[t][o][d] / ABS(observationRangeMax[d] - observationRangeMin[d]) << endl;
                    
                    templatesNormal[t][o][d] = templatesRaw[t][o][d] / (observationRangeMax[d] - observationRangeMin[d]);
                    templateInitialNormal[d] = templateInitialRaw[d] / (observationRangeMax[d] - observationRangeMin[d]);
                    
#if OPENFRAMEWORKS
                    // Normal Representation
                    if(type == GEOMETRIC){
                        
                        pN[d] = templatesNormal[t][o][d] + templateInitialNormal[d];
                        
                    }else{
                        
                        pN.x = o;
                        pN.y = templatesNormal[t][o][d] + templateInitialNormal[d];
                        
                        representationsNormal[t][d].addVertex(pN);
                        representationsNormal[t][d].addColor(ofColor(255, 255, 255));
                        
                    }
                    
                    // Raw Representation
                    if(type == GEOMETRIC){
                        
                        pR[d] = templatesRaw[t][o][d] + templateInitialRaw[d];
                        
                    }else{
                        
                        pR.x = o;
                        pR.y = templatesRaw[t][o][d] + templateInitialRaw[d];
                        
                        representationsRaw[t][d].addVertex(pR);
                        representationsRaw[t][d].addColor(ofColor(255, 255, 255));
                        
                    }
#endif
                    
                }

#if OPENFRAMEWORKS
                if(type == GEOMETRIC){
                    
                    representationsNormal[t][0].addVertex(pN);
                    representationsNormal[t][0].addColor(ofColor(255, 255, 255));
                    
                    representationsRaw[t][0].addVertex(pR);
                    representationsRaw[t][0].addColor(ofColor(255, 255, 255));
                    
                }
#endif
            }
        }
        
        bIsRangeMinSet = bIsRangeMaxSet = true;
        
    }

    void setTemplate(vector< vector<float> > & observations, int templateIndex = 0){
        for(int i = 0; i < observations.size(); i++){
            addObservation(observations[i], templateIndex);
        }
    }

    void setTemplateRaw(vector< vector<float> > & observations, int templateIndex = 0){
        for(int i = 0; i < observations.size(); i++){
            addObservationRaw(observations[i], templateIndex);
        }
    }

    
    vector< vector<float> > & getTemplate(int templateIndex = 0){
        assert(templateIndex < templatesRaw.size());
        return templatesRaw[templateIndex];
    }
    
    vector< vector<float> > & getTemplateRaw(int templateIndex = 0){
        assert(templateIndex < templatesRaw.size());
        return templatesRaw[templateIndex];
    }

    vector< vector<float> > & getTemplateNormal(int templateIndex = 0){
        assert(templateIndex < templatesNormal.size());
        return templatesNormal[templateIndex];
    }
    
    int getNumberOfTemplates(){
        return templatesRaw.size();
    }
    
    int getNumberDimensions(){
        return inputDimensions;
    }
    
    int getTemplateLength(int templateIndex = 0){
        return templatesRaw[templateIndex].size();
    }

    vector<float>& getLastObservation(int templateIndex = 0){
        return templatesRaw[templateIndex][templatesRaw[templateIndex].size() - 1];
    }
    
    vector<float>& getLastRawObservation(int templateIndex = 0){
        return templatesRaw[templateIndex][templatesRaw[templateIndex].size() - 1];
    }
    
    vector<float>& getLastNormalObservation(int templateIndex = 0){
        return templatesNormal[templateIndex][templatesNormal[templateIndex].size() - 1];
    }
   
    vector< vector< vector<float> > >& getTemplates(){
        return templatesRaw;
    }
    
    vector< vector< vector<float> > >& getTemplatesRaw(){
        return templatesRaw;
    }
    
    vector< vector< vector<float> > >& getTemplatesNormal(){
        return templatesNormal;
    }
    
    vector< vector< ofMesh> >& getRepresentationsRaw() {
        return representationsRaw;
    }
    
    vector<float>& getInitialObservation(){
        return templateInitialRaw;
    }
    
    vector<float>& getInitialObservationRaw(){
        return templateInitialRaw;
    }
    
    vector<float>& getInitialObservationNormal(){
        return templateInitialNormal;
    }
    
    void deleteTemplate(int templateIndex = 0){
        assert(templateIndex < templatesRaw.size());
        templatesRaw[templateIndex].clear();
        templatesNormal[templateIndex].clear();
#if OPENFRAMEWORKS
        representationsNormal[templateIndex].clear();
        representationsRaw[templateIndex].clear();
#endif
    }
    
    void clear(){
        templatesRaw.clear();
        templatesNormal.clear();
#if OPENFRAMEWORKS
        representationsNormal.clear();
        representationsRaw.clear();
#endif
        bIsRangeMinSet = false;
        bIsRangeMaxSet = false;
        observationRangeMax.assign(inputDimensions, -INFINITY);
        observationRangeMin.assign(inputDimensions,  INFINITY);
    }

#if OPENFRAMEWORKS

    // Draws the Normalized Mesh
    void draw(){
        
        float x = 0.0f;
        float y = 0.0f;
        float w = 1.0f;
        float h = 1.0f;
        
        if(representationsNormal.size() > 0 && type == GEOMETRIC){
            x = observationRangeMin[0];
            y = observationRangeMin[1];
            w = observationRangeMax[0] - observationRangeMin[0];
            h = observationRangeMax[1] - observationRangeMin[1];
        }
        
        if(representationsNormal.size() > 0 && type == TEMPORAL){
            if(representationsNormal[0].size() > 0){
                w = representationsNormal[0][0].getNumVertices();
                for(int d = 0; d < inputDimensions; d++){
                    if(observationRangeMax[d] > h) h = observationRangeMax[d];
                }
            }
        }

        draw(x, y, w, h);
        
    }

    void draw(float x, float y){
        
        float w = observationRangeMax[0] - observationRangeMin[0];
        float h = observationRangeMax[1] - observationRangeMin[1];
        
        draw(x, y, w, h);
        
    }

    void draw(float x, float y, float w, float h){

        ofPushMatrix();

        if(representationsNormal.size() > 0 && type == GEOMETRIC){
            
            float scaleX = observationRangeMax[0] - observationRangeMin[0];
            float scaleY = observationRangeMax[1] - observationRangeMin[1];
            //float scaleZ = observationRangeMax[2] - observationRangeMin[2];
            
            ofTranslate(x, y);
            ofScale(w / scaleX, h / scaleY);
            ofTranslate(-observationRangeMin[0], -observationRangeMin[1]);
            
            ofPushMatrix();
            
            ofScale(scaleX, scaleY);
            
            representationsNormal[0][0].draw();

            ofPopMatrix();
            
            ofNoFill();
            ofSetColor(255, 0, 0);
            ofPoint min = ofPoint(observationRangeMin[0], observationRangeMin[1], 0);
            ofPoint max = ofPoint(observationRangeMax[0], observationRangeMax[1], 0);
            ofRect(ofRectangle(min, max));
            ofSetColor(255, 255, 255);
        }
        
        if(representationsNormal.size() > 0 && representationsNormal[0].size() > 0 && type == TEMPORAL){
            
            float scaleM = -INFINITY;
            float maxY = -INFINITY;
            
            for(int d = 0; d < inputDimensions; d++){
                if(observationRangeMax[d] - observationRangeMin[d] > scaleM){
                    scaleM = observationRangeMax[d] - observationRangeMin[d];
                }
                if(observationRangeMax[d] > maxY){
                    maxY = observationRangeMax[d];
                }
            }
            
            ofTranslate(x, y);
            ofScale(w / representationsNormal[0][0].getNumVertices(), h / maxY);
            
            for(int d = 0; d < inputDimensions; d++){
                
                ofPushMatrix();
                ofScale(1.0f, observationRangeMax[d] - observationRangeMin[d]);
            
                representationsNormal[0][d].draw();
            
                ofPopMatrix();
                
            }

            ofNoFill();
            ofSetColor(255, 0, 0);
            ofPoint min = ofPoint(0, 0, 0);
            ofPoint max = ofPoint(representationsNormal[0][0].getNumVertices(), maxY, 0);
            ofRect(ofRectangle(min, max));
            ofSetColor(255, 255, 255);
        }

        ofPopMatrix();
        
    }

#endif
    
    
protected:

    string name;
    int inputDimensions;
    ofxGVFGestureType type;
    vector<float> observationRangeMax;
    vector<float> observationRangeMin;
    bool bAutoAdjustNormalRange;
    bool bIsRangeMaxSet;
    bool bIsRangeMinSet;
//    int id;
    
    vector<float> templateInitialRaw;
    vector<float> templateInitialNormal;
    
    vector< vector< vector<float> > > templatesRaw;
    vector< vector< vector<float> > > templatesNormal;
    
#if OPENFRAMEWORKS
    vector< vector<ofMesh> > representationsNormal;
    vector< vector<ofMesh> > representationsRaw;
#endif
    
};

#endif

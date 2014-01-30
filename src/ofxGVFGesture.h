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

#ifdef OPENFRAMEWORKS
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
        bAutoAdjustNormalRange = true;
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
    
#ifdef OPENFRAMEWORKS
    
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
    }
    
    void setMinRange(vector<float> observationRangeMin){
        this->observationRangeMin = observationRangeMin;
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
    
    void normalise(){
        
#ifdef OPENFRAMEWORKS
        
        // reserve space for raw and normal meshes
        representationsRaw.resize(templatesRaw.size() + 1);
        representationsNormal.resize(templatesRaw.size() + 1);
        
        templatesNormal.resize(templatesRaw.size());
        
        for(int t = 0; t < templatesRaw.size(); t++){
            
            if(type == GEOMETRIC){
                
                // for GEOMETRIC representations let's use a single mesh with n-Dimensions
                
                representationsRaw[t].resize(1);
                representationsRaw[t][0].setMode(OF_PRIMITIVE_LINE_STRIP);
                
                representationsNormal[t].resize(1);
                representationsNormal[t][0].setMode(OF_PRIMITIVE_LINE_STRIP);
                
            }else{
                
                // for TEMPORAL representations let's use a mesh for EACH of the n-Dimensions
                
                representationsRaw[t].resize(inputDimensions);
                for(int i = 0; i < inputDimensions; i++){
                    representationsRaw[t][i].setMode(OF_PRIMITIVE_LINE_STRIP);
                }
                
                representationsNormal[t].resize(inputDimensions);
                for(int i = 0; i < inputDimensions; i++){
                    representationsNormal[t][i].setMode(OF_PRIMITIVE_LINE_STRIP);
                }
            }
#endif
            
            for(int m = 0; m < representationsNormal[t].size(); m++){
                representationsNormal[t][m].clear();
            }

            templatesNormal[t].resize(templatesRaw[t].size());
            
            for(int o = 0; o < templatesRaw[t].size(); o++){
                
                ofPoint pN;
                
                templatesNormal[t][o].resize(inputDimensions);
                
                for(int d = 0; d < inputDimensions; d++){
                    cout << templatesNormal[t][o][d] << " " << templatesRaw[t][o][d] / ABS(observationRangeMax[d] - observationRangeMin[d]) << endl;
                    templatesNormal[t][o][d] = templatesRaw[t][o][d] / (observationRangeMax[d] - observationRangeMin[d]);
#ifdef OPENFRAMEWORKS
                    if(type == GEOMETRIC){
                        
                        pN[d] = templatesNormal[t][o][d];
                        
                    }else{
                        
                        pN.x = o;
                        pN.y = templatesNormal[t][o][d];
                        
                        representationsNormal[t][d].addVertex(pN);
                        representationsNormal[t][d].addColor(ofColor(255, 255, 255));
                        
                    }
                    
                }
                
                if(type == GEOMETRIC){
                    
                    representationsNormal[t][0].addVertex(pN);
                    representationsNormal[t][0].addColor(ofColor(255, 255, 255));
                }
#else
                }
#endif
            }
        }
    
    bIsRangeMinSet = bIsRangeMaxSet = true;
    
    }
    
#ifdef OPENFRAMEWORKS
    void addObservationRaw(ofPoint observation, int templateIndex = 0){
        assert(inputDimensions <= 3);
        vector<float> obs(inputDimensions);
        for(int i = 0; i < inputDimensions; i++){
            obs[i] = observation[i];
        }
        addObservationRaw(obs, templateIndex);
    }
#endif
    
    void addObservationRaw(vector<float> observation, int templateIndex = 0){
        
        // check we have a valid templateIndex and correct number of input dimensions
        assert(templateIndex <= templatesRaw.size());
        assert(observation.size() == inputDimensions);
        
        // if the template index is same as the number of temlates make a new template
        if(templateIndex == templatesRaw.size()){ // make a new template
            
            // reserve space in raw and normal template storage
            templatesRaw.resize(templatesRaw.size() + 1);
            templatesNormal.resize(templatesNormal.size() + 1);
            
//#ifdef OPENFRAMEWORKS
//            
//            // reserve space for raw and normal meshes
//            representationsRaw.resize(templatesRaw.size() + 1);
//            representationsNormal.resize(templatesRaw.size() + 1);
//
//            if(type == GEOMETRIC){
//                
//                // for GEOMETRIC representations let's use a single mesh with n-Dimensions
//                
//                representationsRaw[templateIndex].resize(1);
//                representationsRaw[templateIndex][0].setMode(OF_PRIMITIVE_LINE_STRIP);
//                
//                representationsNormal[templateIndex].resize(1);
//                representationsNormal[templateIndex][0].setMode(OF_PRIMITIVE_LINE_STRIP);
//                
//            }else{
//                
//                // for TEMPORAL representations let's use a mesh for EACH of the n-Dimensions
//                
//                representationsRaw[templateIndex].resize(inputDimensions);
//                for(int i = 0; i < inputDimensions; i++){
//                    representationsRaw[templateIndex][i].setMode(OF_PRIMITIVE_LINE_STRIP);
//                }
//                
//                representationsNormal[templateIndex].resize(inputDimensions);
//                for(int i = 0; i < inputDimensions; i++){
//                    representationsNormal[templateIndex][i].setMode(OF_PRIMITIVE_LINE_STRIP);
//                }
//            }
//#endif
        }
        
        // store the raw observation
        templatesRaw[templateIndex].push_back(observation);
        
        // if set let's auto size the range for normalising
        if(bAutoAdjustNormalRange) autoAdjustMinMax(observation);
        
        normalise();
        
//        vector<float> observationNormal(inputDimensions);
//        
//        // check to see if ranges have been set manually
//        if(bIsRangeMinSet && bIsRangeMaxSet){
//            
//            // normalise the raw value
//            
//            // calculate the normal on the fly
//            // NOTE: this will have errors if the
//            // normal range changes...so call
//            // normalise() at the end of learning
//            // to re-normalise all the values
//            for(int d = 0; d < inputDimensions; d++){
//                observationNormal[d] = observation[d] / (observationRangeMax[d] - observationRangeMin[d]);
//            }
//            
//            // store the normalised observation
//            templatesNormal[templateIndex].push_back(observationNormal);
//        }
 
//#ifdef OPENFRAMEWORKS
//        if(type == GEOMETRIC){
//            
//            ofPoint pR, pN;
//            
//            for(int i = 0; i < inputDimensions; i++){
//                pR[i] = observation[i];
//                if(bIsRangeMinSet && bIsRangeMaxSet) pN[i] = observationNormal[i];
//            }
//            
//            representationsRaw[templateIndex][0].addVertex(pR);
//            representationsRaw[templateIndex][0].addColor(ofColor(255, 255, 255));
//            
//            if(bIsRangeMinSet && bIsRangeMaxSet){
//                representationsNormal[templateIndex][0].addVertex(pN);
//                representationsNormal[templateIndex][0].addColor(ofColor(255, 255, 255));
//            }
//            
//        }else{
//            
//            for(int d = 0; d < inputDimensions; d++){
//                
//                ofPoint pR;
//                
//                pR.x = templatesRaw[templateIndex].size();
//                pR.y = observation[d];
//                
//                representationsRaw[templateIndex][d].addVertex(pR);
//                representationsRaw[templateIndex][d].addColor(ofColor(255, 255, 255));
//                
//                if(bIsRangeMinSet && bIsRangeMaxSet){
//                    ofPoint pN;
//                    pN.x = templatesRaw[templateIndex].size();
//                    pN.y = observationNormal[d];
//                    representationsNormal[templateIndex][d].addVertex(pN);
//                    representationsNormal[templateIndex][d].addColor(ofColor(255, 255, 255));
//                }
//            }
//        }
//#endif
        
    }
    
    void setTemplateRaw(vector< vector<float> > & observations, int templateIndex = 0){
        for(int i = 0; i < observations.size(); i++){
            addObservationRaw(observations[i], templateIndex);
        }
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
    
    vector< vector< vector<float> > > & getTemplatesRaw(){
        return templatesRaw;
    }
    
    vector< vector< vector<float> > > & getTemplatesNormal(){
        return templatesNormal;
    }
    
    void deleteTemplate(int templateIndex = 0){
        assert(templateIndex < templatesRaw.size());
        templatesRaw[templateIndex].clear();
        templatesNormal[templateIndex].clear();
#ifdef OPENFRAMEWORKS
        representationsRaw[templateIndex].clear();
        representationsNormal[templateIndex].clear();
#endif
    }
    
    void clear(){
        templatesRaw.clear();
        templatesNormal.clear();
#ifdef OPENFRAMEWORKS
        representationsRaw.clear();
        representationsNormal.clear();
#endif
        bIsRangeMinSet = false;
        bIsRangeMaxSet = false;
        observationRangeMax.assign(inputDimensions, -INFINITY);
        observationRangeMin.assign(inputDimensions,  INFINITY);
    }

#ifdef OPENFRAMEWORKS

    void draw(){
        
        float x = 0.0f;
        float y = 0.0f;
        
        float w = observationRangeMax[0] - observationRangeMin[0];
        float h = observationRangeMax[1] - observationRangeMin[1];

        draw(x, y, w, h);
        
    }

    void draw(float x, float y){
        
        float w = observationRangeMax[0] - observationRangeMin[0];
        float h = observationRangeMax[1] - observationRangeMin[1];
        
        draw(x, y, w, h);
        
    }

    void draw(float x, float y, float w, float h){
        
        float scaleX = observationRangeMax[0] - observationRangeMin[0];
        float scaleY = observationRangeMax[1] - observationRangeMin[1];
        //float scaleZ = observationRangeMax[2] - observationRangeMin[2];

        ofPushMatrix();

        ofTranslate(x, y);
        ofScale(w / scaleX, h / scaleY);

        if(representationsRaw.size() > 0 && type == GEOMETRIC){
            
            ofPushMatrix();
            if(bIsRangeMinSet && bIsRangeMaxSet){
                ofScale(scaleX, scaleY);
                representationsNormal[0][0].draw();
            }else{
                representationsRaw[0][0].draw();
            }
            ofPopMatrix();
            
            ofNoFill();
            ofSetColor(255, 0, 0);
            ofPoint min = ofPoint(observationRangeMin[0], observationRangeMin[1], 0);
            ofPoint max = ofPoint(observationRangeMax[0], observationRangeMax[1], 0);
            ofRect(ofRectangle(min, max));
            ofSetColor(255, 255, 255);
        }
        if(representationsRaw.size() > 0 && type == TEMPORAL){
            for(int d = 0; d < inputDimensions; d++){
                ofPushMatrix();
                if(bIsRangeMinSet && bIsRangeMaxSet){
                    ofScale(1.0f, scaleY);
                    representationsNormal[0][d].draw();
                }else{
                    representationsRaw[0][d].draw();
                }
                ofPopMatrix();
                
            }
            ofNoFill();
            ofSetColor(255, 0, 0);
            ofPoint min = ofPoint(observationRangeMin[0], observationRangeMin[1], 0);
            ofPoint max = ofPoint(observationRangeMax[0], observationRangeMax[1], 0);
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
    
    ofxGVFVarianceCoefficents coefficients;
    
    vector< vector< vector<float> > > templatesRaw;
    vector< vector< vector<float> > > templatesNormal;
    
#ifdef OPENFRAMEWORKS
    vector< vector<ofMesh> > representationsRaw;
    vector< vector<ofMesh> > representationsNormal;
#endif
    
};

#endif

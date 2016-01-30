//
//  GVFGesture.h
//  gvf
//
//  Created by Baptiste Caramiaux on 22/01/16.
//
//

#ifndef GVFGesture_h
#define GVFGesture_h

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif


class GVFGesture
{
public:
    
    GVFGesture()
    {
        inputDimensions = 2;
        setAutoAdjustRanges(true);
        templatesRaw    = vector<vector<vector<float > > >();
        templatesNormal = vector<vector<vector<float > > >();
        clear();
    }
    
    GVFGesture(int inputDimension){
        inputDimensions = inputDimension;
        setAutoAdjustRanges(true);
        templatesRaw    = vector<vector<vector<float > > >();
        templatesNormal = vector<vector<vector<float > > >();
        clear();
    }
    
    ~GVFGesture(){
        clear();
    }
    
    void setNumberDimensions(int dimensions){
        assert(dimensions > 0);
        inputDimensions = dimensions;
    }
    
    void setAutoAdjustRanges(bool b){
        //        if(b) bIsRangeMinSet = bIsRangeMaxSet = false;
        bAutoAdjustNormalRange = b;
    }
    
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
        //        bIsRangeMaxSet = true;
        normalise();
    }
    
    void setMinRange(vector<float> observationRangeMin){
        this->observationRangeMin = observationRangeMin;
        //        bIsRangeMinSet = true;
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
    
    void addObservation(vector<float> observation, int templateIndex = 0){
        if (observation.size() != inputDimensions)
            inputDimensions = observation.size();
        
        // check we have a valid templateIndex and correct number of input dimensions
        assert(templateIndex <= templatesRaw.size());
        assert(observation.size() == inputDimensions);
        
        // if the template index is same as the number of temlates make a new template
        if(templateIndex == templatesRaw.size()){ // make a new template
            
            // reserve space in raw and normal template storage
            templatesRaw.resize(templatesRaw.size() + 1);
            templatesNormal.resize(templatesNormal.size() + 1);
            
        }
        
        if(templatesRaw[templateIndex].size() == 0)
        {
            templateInitialObservation = observation;
            templateInitialNormal = observation;
        }
        
        for(int j = 0; j < observation.size(); j++)
            observation[j] = observation[j] - templateInitialObservation[j];
        
        // store the raw observation
        templatesRaw[templateIndex].push_back(observation);
        
        autoAdjustMinMax(observation);
        
        normalise();
    }
    
    
    
    void normalise()
    {
        templatesNormal.resize(templatesRaw.size());
        for(int t = 0; t < templatesRaw.size(); t++)
        {
            templatesNormal[t].resize(templatesRaw[t].size());
            for(int o = 0; o < templatesRaw[t].size(); o++)
            {
                templatesNormal[t][o].resize(inputDimensions);
                for(int d = 0; d < inputDimensions; d++)
                {
                    templatesNormal[t][o][d] = templatesRaw[t][o][d] / (observationRangeMax[d] - observationRangeMin[d]);
                    templateInitialNormal[d] = templateInitialObservation[d] / (observationRangeMax[d] - observationRangeMin[d]);
                }
            }
        }
    }
    
    void setTemplate(vector< vector<float> > & observations, int templateIndex = 0){
        for(int i = 0; i < observations.size(); i++){
            addObservation(observations[i], templateIndex);
        }
    }
    
    vector< vector<float> > & getTemplate(int templateIndex = 0){
        assert(templateIndex < templatesRaw.size());
        return templatesRaw[templateIndex];
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
    
    int getTemplateDimension(int templateIndex = 0){
        return templatesRaw[templateIndex][0].size();
    }
    
    vector<float>& getLastObservation(int templateIndex = 0){
        return templatesRaw[templateIndex][templatesRaw[templateIndex].size() - 1];
    }
    
    vector< vector< vector<float> > >& getTemplates(){
        return templatesRaw;
    }
    
    vector<float>& getInitialObservation(){
        return templateInitialObservation;
    }
    
    void deleteTemplate(int templateIndex = 0)
    {
        assert(templateIndex < templatesRaw.size());
        templatesRaw[templateIndex].clear();
        templatesNormal[templateIndex].clear();
    }
    
    void clear()
    {
        templatesRaw.clear();
        templatesNormal.clear();
        observationRangeMax.assign(inputDimensions, -INFINITY);
        observationRangeMin.assign(inputDimensions,  INFINITY);
    }
    
private:
    
    int inputDimensions;
    bool bAutoAdjustNormalRange;
    
    vector<float> observationRangeMax;
    vector<float> observationRangeMin;
    
    vector<float> templateInitialObservation;
    vector<float> templateInitialNormal;
    
    vector< vector< vector<float> > > templatesRaw;
    vector< vector< vector<float> > > templatesNormal;
    
    vector<vector<float> > gestureDataFromFile;
};

#endif /* GVFGesture_h */

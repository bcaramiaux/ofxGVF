#pragma once


template<class modelClass>
class IxModel {

public:

    IxModel()
    {
        themodel = new modelClass();
    }

    ~IxModel()
    {

    }
    
    
    // virtual void addExample(Mat example, int category)
    // {

    // }
    
    // virtual void classifyExample(const Mat query,
    //                              float &distance,
    //                              int &subscript)
    // {

    // }
    // virtual void classifyTest(const Mat query,
    //                              vector<float> &distance,
    //                              int &subscript)
    // {

    // }
    
    virtual void reset()
    {

    }
    
    virtual void save()
    {

    }
    
    virtual void load()
    {

    }

    virtual modelClass * getTheModel(){
        return themodel;
    }

    
private:

    modelClass* themodel;

    
};
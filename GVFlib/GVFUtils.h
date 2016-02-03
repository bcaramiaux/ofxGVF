//
//  GVFTypesAndUtils.h
//
//
//

#ifndef __H_GVFTYPES
#define __H_GVFTYPES

#include <map>
#include <vector>
#include <iostream>
#include <random>
#include <iostream>
#include <math.h>
#include <assert.h>

using namespace std;

/**
 * Configuration structure
 */
typedef struct
{
    int     inputDimensions;    /**< input dimesnion */
    bool    translate;          /**< translate flag */
    bool    segmentation;       /**< segmentation flag */
} GVFConfig;

/**
 * Parameters structure
 */
typedef struct
{
    float           tolerance;              /**< input dimesnion */
    float           distribution;
    int             numberParticles;
    int             resamplingThreshold;
    float           alignmentVariance;
    float           speedVariance;
    vector<float>   scaleVariance;
    vector<float>   dynamicsVariance;
    vector<float>   scalingsVariance;
    vector<float>   rotationsVariance;
    // spreadings
    float           alignmentSpreadingCenter;
    float           alignmentSpreadingRange;
    float           dynamicsSpreadingCenter;
    float           dynamicsSpreadingRange;
    float           scalingsSpreadingCenter;
    float           scalingsSpreadingRange;
    float           rotationsSpreadingCenter;
    float           rotationsSpreadingRange;
    
    int             predictionSteps;
    vector<float>   dimWeights;
} GVFParameters;

// Outcomes structure
typedef struct
{
    int likeliestGesture;
    vector<float> likelihoods;
    vector<float> alignments;
    vector<vector<float> > dynamics;
    vector<vector<float> > scalings;
    vector<vector<float> > rotations;
} GVFOutcomes;


//--------------------------------------------------------------
// init matrix by allocating memory
template <typename T>
inline void initMat(vector< vector<T> > & M, int rows, int cols){
    M.resize(rows);
    for (int n=0; n<rows; n++){
        M[n].resize(cols);
    }
}

//--------------------------------------------------------------
// init matrix and copy values from another matrix
template <typename T>
inline void setMat(vector< vector<T> > & C, vector< vector<float> > & M){
    int rows = M.size();
    int cols = M[0].size();
    //C.resize(rows);
    C = vector<vector<T> >(rows);
    for (int n=0; n<rows; n++){
        //C[n].resize(cols);
        C[n] = vector<T>(cols);
        for (int m=0;m<cols;m++){
            C[n][m] = M[n][m];
        }
    }
}

//--------------------------------------------------------------
// init matrix by allocating memory and fill with T value
template <typename T>
inline void setMat(vector< vector<T> > & M, T value, int rows, int cols){
    M.resize(rows);
    for (int n=0; n<rows; n++){
        M[n].resize(cols);
        for (int m=0; m<cols; m++){
            M[n][m] = value;
        }
    }
}

//--------------------------------------------------------------
// set matrix filled with T value
template <typename T>
inline void setMat(vector< vector<T> > & M, T value){
    for (int n=0; n<M.size(); n++){
        for (int m=0; m<M[n].size(); m++){
            M[n][m] = value;
        }
    }
}

//--------------------------------------------------------------
template <typename T>
inline void printMat(vector< vector<T> > & M){
    for (int k=0; k<M.size(); k++){
        cout << k << ": ";
        for (int l=0; l<M[0].size(); l++){
            cout << M[k][l] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

//--------------------------------------------------------------
template <typename T>
inline void printVec(vector<T> & V){
    for (int k=0; k<V.size(); k++){
        cout << k << ": " << V[k] << (k == V.size() - 1 ? "" : " ,");
    }
    cout << endl;
}

//--------------------------------------------------------------
template <typename T>
inline void initVec(vector<T> & V, int rows){
    V.resize(rows);
}

//--------------------------------------------------------------
template <typename T>
inline void setVec(vector<T> & C, vector<int> &V){
    int rows = V.size();
    C = vector<T>(rows);
    //C.resize(rows);
    for (int n=0; n<rows; n++){
        C[n] = V[n];
    }
}

//--------------------------------------------------------------
template <typename T>
inline void setVec(vector<T> & C, vector<float> & V){
    int rows = V.size();
    C.resize(rows);
    for (int n=0; n<rows; n++){
        C[n] = V[n];
    }
}

//--------------------------------------------------------------
template <typename T>
inline void setVec(vector<T> & V, T value){
    for (int n=0; n<V.size(); n++){
        V[n] = value;
    }
}

//--------------------------------------------------------------
template <typename T>
inline void setVec(vector<T> & V, T value, int rows){
    V.resize(rows);
    setVec(V, value);
}

//--------------------------------------------------------------
template <typename T>
inline vector< vector<T> > dotMat(vector< vector<T> > & M1, vector< vector<T> > & M2){
    // TODO(Baptiste)
}

//--------------------------------------------------------------
template <typename T>
inline vector< vector<T> > multiplyMatf(vector< vector<T> > & M1, T v){
    vector< vector<T> > multiply;
    initMat(multiply, M1.size(), M1[0].size());
    for (int i=0; i<M1.size(); i++){
        for (int j=0; j<M1[i].size(); j++){
            multiply[i][j] = M1[i][j] * v;
        }
    }
    return multiply;
}

//--------------------------------------------------------------
template <typename T>
inline vector< vector<T> > multiplyMatf(vector< vector<T> > & M1, vector< vector<T> > & M2){
    assert(M1[0].size() == M2.size()); // columns in M1 == rows in M2
    vector< vector<T> > multiply;
    initMat(multiply, M1.size(), M2[0].size()); // rows in M1 x cols in M2
    for (int i=0; i<M1.size(); i++){
        for (int j=0; j<M2[i].size(); j++){
            multiply[i][j] = 0.0f;
            for(int k=0; k<M1[0].size(); k++){
                multiply[i][j] += M1[i][k] * M2[k][j];
            }
            
        }
    }
    return multiply;
}

//--------------------------------------------------------------
template <typename T>
inline vector<T> multiplyMat(vector< vector<T> > & M1, vector< T> & Vect){
    assert(Vect.size() == M1[0].size()); // columns in M1 == rows in Vect
    vector<T> multiply;
    initVec(multiply, Vect.size());
    for (int i=0; i<M1.size(); i++){
        multiply[i] = 0.0f;
        for (int j=0; j<M1[i].size(); j++){
            multiply[i] += M1[i][j] * Vect[j];
        }
    }
    return multiply;
}

//--------------------------------------------------------------
template <typename T>
inline float getMeanVec(vector<T>& V){
    float tSum = 0.0f;
    for (int n=0; n<V.size(); n++){
        tSum += V[n];
    }
    return tSum / (float)V.size();
}

template <typename T>
inline vector<vector<float> > getRotationMatrix3d(T phi, T theta, T psi)
{
    vector< vector<float> > M;
    initMat(M,3,3);
    
    M[0][0] = cos(theta)*cos(psi);
    M[0][1] = -cos(phi)*sin(psi)+sin(phi)*sin(theta)*cos(psi);
    M[0][2] = sin(phi)*sin(psi)+cos(phi)*sin(theta)*cos(psi);
    
    M[1][0] = cos(theta)*sin(psi);
    M[1][1] = cos(phi)*cos(psi)+sin(phi)*sin(theta)*sin(psi);
    M[1][2] = -sin(phi)*cos(psi)+cos(phi)*sin(theta)*sin(psi);
    
    M[2][0] = -sin(theta);
    M[2][1] = sin(phi)*cos(theta);
    M[2][2] = cos(phi)*cos(theta);
    
    return M;
}

template <typename T>
float distance_weightedEuclidean(vector<T> x, vector<T> y, vector<T> w)
{
    int count = x.size();
    if (count <= 0) return 0;
    float dist = 0.0;
    for(int k = 0; k < count; k++) dist += w[k] * pow((x[k] - y[k]), 2);
    return dist;
}

////--------------------------------------------------------------
//vector<vector<float> > getRotationMatrix3d(float phi, float theta, float psi)
//{
//    vector< vector<float> > M;
//    initMat(M,3,3);
//    
//    M[0][0] = cos(theta)*cos(psi);
//    M[0][1] = -cos(phi)*sin(psi)+sin(phi)*sin(theta)*cos(psi);
//    M[0][2] = sin(phi)*sin(psi)+cos(phi)*sin(theta)*cos(psi);
//    
//    M[1][0] = cos(theta)*sin(psi);
//    M[1][1] = cos(phi)*cos(psi)+sin(phi)*sin(theta)*sin(psi);
//    M[1][2] = -sin(phi)*cos(psi)+cos(phi)*sin(theta)*sin(psi);
//    
//    M[2][0] = -sin(theta);
//    M[2][1] = sin(phi)*cos(theta);
//    M[2][2] = cos(phi)*cos(theta);
//    
//    return M;
//}

//float distance_weightedEuclidean(vector<float> x, vector<float> y, vector<float> w)
//{
//    int count = x.size();
//    if (count <= 0) return 0;
//    float dist = 0.0;
//    for(int k = 0; k < count; k++) dist += w[k] * pow((x[k] - y[k]), 2);
//    return dist;
//}

#endif

//
//  ofxGVFMatrix.h
//  ofxGVF-Example
//
//  Created by gameover on 12/11/13.
//
//

#ifndef __H_OFXGVFMATRIX
#define __H_OFXGVFMATRIX

#include <vector>
#include <iostream>

using namespace std;

// TODO: SEE BELOW FOR ofxGVFMatrix type/class...might also require ofxGVFVector...maybe over kill?
// Anyway, skeleton is below for comparison with global, c style operations as per below...

////////////////////////////////////////////////////////////////
//
// MATRICES & VECTORS
//
////////////////////////////////////////////////////////////////

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
    C.resize(rows);
    for (int n=0; n<rows; n++){
        C[n].resize(cols);
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
    C.resize(rows);
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
inline float getMeanVec(vector<T>& V){
    float tSum = 0.0f;
    for (int n=0; n<V.size(); n++){
        tSum += V[n];
    }
    return tSum / (float)V.size();
}

//template<typename T>
//class ofxGVFMatrix{
//
//public:
//
//    ofxGVFMatrix(){
//        matrix.clear();
//    };
//
//    ofxGVFMatrix(vector<T>& vec){
//        set(vec);
//    };
//
//    ofxGVFMatrix(vector< vector<T> >& mat){
//        set(mat);
//    };
//
//    ofxGVFMatrix(int rows, int cols){
//        set(rows, cols);
//    };
//
//    ofxGVFMatrix(T value, int rows, int cols){
//        set(value, rows, cols);
//    };
//
//    void set(vector<T>& vec){
//        matrix.clear();
//        matrix.resize(vec.size());
//        for (int n=0; n<vec.size(); n++){
//            matrix[n].push_back(vec[n]);
//        }
//    };
//
//    void set(vector< vector<T> >& mat){
//        matrix = mat;
//    };
//
//    void set(int rows, int cols){
//        matrix.clear();
//        matrix.resize(rows);
//        for (int n=0; n<rows; n++){
//            matrix[n].resize(cols);
//        }
//    }
//
//    void set(T value, int rows, int cols){
//        matrix.clear();
//        matrix.resize(rows);
//        for (int n=0; n<rows; n++){
//            matrix[n].resize(cols);
//            for (int m=0; m<cols; m++){
//                matrix[n][m] = value;
//            }
//        }
//    }
//
//    void mean(){
//        float tSum = 0.0f;
//        for (int n=0; n<rows(); n++) {
//            for (int m=0; m<cols(); m++){
//                tSum += matrix[n][m];
//            }
//        }
//        return tSum / size();
//    }
//
//    void print(){
//        for (int n=0; n<rows(); n++){
//            cout << n << ": ";
//            for (int m=0; m<cols(); m++)
//                cout << matrix[n][m] << " ";
//            cout << endl;
//        }
//        cout << endl;
//    }
//
//    int rows(){
//        return matrix.size();
//    }
//
//    int cols(){
//        return matrix[0].size();
//    }
//
//    int size(){
//        return rows() * cols();
//    }
//
//    ofxGVFMatrix& multiply(ofxGVFMatrix& rhs){
//        assert(cols() == rhs.rows()); // columns in M1 == rows in M2
//        ofxGVFMatrix t = ofxGVFMatrix(rows(), rhs.cols());
//        for (int i=0;i<rows();i++){
//            for (int j=0;j<rhs.cols();j++){
//                t[i][j] = 0.0f;
//                for(int k=0;k<cols();k++){
//                    t[i][j] += matrix[i][k] * rhs[k][j];
//                }
//            }
//        }
//        matrix = t;
//        return *this;
//    }
//
//    ofxGVFMatrix& multiply(vector<T>& rhs){
//        return multiply(ofxGVFMatrix(rhs));
//    }
//
//    ofxGVFMatrix& multiply(T& rhs){
//        for (int i=0;i<rows();i++){
//            for (int j=0;j<cols();j++){
//                matrix[i][j] *= rhs;
//            }
//        }
//        return *this;
//    }
////    TODO: Baptiste!
////    ofxGVFMatrix& dot(ofxGVFMatrix& rhs){
////        return *this;
////    }
//
//    vector<T>& operator[](int row){
//        return matrix[row];
//    }
//
//    const vector<T>& operator[](int row) const{
//        return matrix[row];
//    }
//
//    ofxGVFMatrix& operator*(vector<T>& rhs){
//        return multiply(rhs);
//    }
//
//    ofxGVFMatrix& operator*(ofxGVFMatrix& rhs){
//        return multiply(rhs);
//    }
//
//    ofxGVFMatrix& operator*(T& rhs){
//        return multiply(rhs);
//    }
//
//private:
//
//    vector< vector<T> > matrix;
//
//};

#endif

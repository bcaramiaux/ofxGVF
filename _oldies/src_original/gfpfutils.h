#ifndef GFPFUTILS_H
#define GFPFUTILS_H

#include <Eigen/Core>
#include <string>
#include <vector>

// load file into matrix
Eigen::MatrixXf loadGestureMatrixFromFile(std::string pathToFile);

// multi training
std::pair<Eigen::MatrixXf,Eigen::MatrixXf> multiTrain(std::vector<Eigen::MatrixXf> inputs, float smoothness);

// resampling of a matrix
Eigen::MatrixXf resampleGesture(Eigen::MatrixXf original, int nout);

// translate matrix to its first point and scale ...
Eigen::MatrixXf translateAndScale(Eigen::MatrixXf original);

// translate matrix w.r.t. its first point, no scaling
Eigen::MatrixXf translate(Eigen::MatrixXf original);

// rotate gesture by a specified angle in radians
Eigen::MatrixXf rotate(Eigen::MatrixXf original, float rotationAngle);

#endif

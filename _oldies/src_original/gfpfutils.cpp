#include "gfpfutils.h"
#include <fstream>
#include <vector>
#include <Eigen/LU>
#include <iostream>
using namespace std;
using namespace Eigen;

MatrixXf loadGestureMatrixFromFile(string pathToFile)
{
  ifstream myfile (pathToFile.c_str());
  if (myfile.is_open())
    {
      vector<Vector2f> lines;
      int lineno = 1;
      while(myfile.good())
	{
	  string line;
	  getline(myfile,line);
	  if (line.size() > 0)
	    {
	      float x,y;
	      int read = sscanf(line.c_str(), "%f %f",&x,&y); 
	      if( read != 2)
		{
		  cerr << "error in the file format at line " << lineno << endl;
		  cerr << "   read = " << read << endl;
		}
	      else
		{
		  Vector2f v;
		  v << x,y;
		  lines.push_back(v);
		}
	      lineno++;
	    }
	}
      // compact into matrix
      MatrixXf data(lines.size(),2);
      for(int i = 0; i < lines.size(); i++)
	{
	  data.row(i) = lines[i];
	}
      return data;
    }
  else
    {
      cerr << "unable to open file " << pathToFile << endl;
      MatrixXf bubi(1,1);
      return bubi;
    }
}

MatrixXf resampleGesture(MatrixXf original, int nout)
{
  MatrixXf resampled(nout,original.cols());
  for(int col = 0; col < original.cols(); col++)
    {
      VectorXf sigin = original.col(col);
      VectorXf sigout(nout);
      int nin = sigin.rows();
      float tin_index = 0;
      // first sample
      sigout[0] = sigin[0];
      // middle samples
      for(int n = 1; n < nout-1; n++)
	{
	  float tout = n/(nout - 1.);
	  while(tin_index/(nin-1.) <= tout)
	    tin_index++;
	  tin_index--;
	  float alpha = (nin-1.) * tout - tin_index;
	  float y = (1-alpha) * sigin[tin_index] + alpha * sigin[tin_index+1];
	  sigout[n] = y;
	}
      // last sample
      sigout[nout-1] = sigin[nin-1];
      // copy to matrix
      resampled.col(col) = sigout;
    }
  return resampled;
}

pair<MatrixXf,MatrixXf> multiTrain(vector<MatrixXf> inputs, float smoothness)
{
  int numrefs = inputs.size();
  // resamples according to average length of gestures
  int nsamples = 0;
  for(int i = 0; i < numrefs; i++)
    nsamples += inputs[i].rows();
  nsamples /= numrefs;
  vector<MatrixXf> resampledInputs;
  for(int i = 0; i < numrefs; i++)
    resampledInputs.push_back(resampleGesture(inputs[i],nsamples));

  // cout << "DEBUG - resampledinputs[0] = " << endl << resampledInputs[0] << endl;

  // compute the "average gesture"
  MatrixXf avgGesture(nsamples,2);
  avgGesture.setConstant(0);
  for(int i = 0; i < numrefs; i++)
    avgGesture += resampledInputs[i] / numrefs;

  // cout << "DEBUG - numref = " << numrefs << " and average is " << endl << avgGesture << endl;

  // compute the covariance matrices
  Matrix2f smoothingblock;
  smoothingblock(0,0) = smoothness;
  smoothingblock(0,1) = 0;
  smoothingblock(1,1) = smoothness;
  smoothingblock(1,0) = 0;
  MatrixXf invCovariance(2*nsamples,2);
  if(numrefs == 1)// || true)
    {
      for(int i = 0; i < nsamples; i++)
	invCovariance.block<2,2>(2*i,0) = smoothingblock.inverse();
    }
  else
    {
      for(int i = 0; i < nsamples; i++)
	{
	  // compute obs matrix for the sample
	  MatrixXf obsmat(numrefs,2);
	  for(int nr = 0; nr < numrefs; nr++)
	    {
	      obsmat.row(nr) = resampledInputs[nr].row(i) - avgGesture.row(i);
	    }
	  MatrixXf covblock = obsmat.transpose() * obsmat;
	  covblock /= numrefs;
	  covblock = (covblock * numrefs + smoothingblock)/(numrefs+1);
	  invCovariance.block(i*2,0,2,2) = covblock.inverse();
	}
    }
  pair<MatrixXf,MatrixXf> retval;
  retval.first = avgGesture;
  retval.second = invCovariance;
  return retval;
}


MatrixXf translateAndScale(MatrixXf original)
{
  // translate
  MatrixXf mat(original.rows(),original.cols());
  for(int i = 0; i < original.rows(); i++)
    mat.row(i) = original.row(i) - original.row(0);
  // scale
  float maxrange = (original.colwise().maxCoeff() - original.colwise().minCoeff()).maxCoeff(); 
  mat /= maxrange;
  return mat;
}


MatrixXf translate(Eigen::MatrixXf original)
{
  // translate
  MatrixXf mat(original.rows(),original.cols());
  for(int i = 0; i < original.rows(); i++)
    mat.row(i) = original.row(i) - original.row(0);
  return mat;
}


MatrixXf rotate(MatrixXf original, float rotationAngle)
{
  Matrix2f rotmat; rotmat << cos(rotationAngle), -sin(rotationAngle), sin(rotationAngle), cos(rotationAngle);
  MatrixXf res = original * rotmat.transpose();
  return res;
}

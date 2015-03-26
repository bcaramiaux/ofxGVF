
#include "../src/ofxGVF.h"
#include "IxModel.h"

#include <istream>
#include <sstream>
#include <string>
// #include <time>

#include <map>
#include <random>
#include <cmath>


using namespace std;



vector<int> getRandPerm(int n);
// load matrix from an ascii text file.
void load_matrix(std::istream* is, std::vector< std::vector<float> >* matrix,const string& delim = " \t")
{
  using namespace std;

  string      line;
  string      strnum;

    // clear first
  matrix->clear();

    // parse line by line
  while (getline(*is, line)) {
    matrix->push_back(vector<float>());

    for (string::const_iterator i = line.begin(); i != line.end(); ++ i) {
            // If i is not a delim, then append it to strnum
      if (delim.find(*i) == string::npos) {
        strnum += *i;
        if (i + 1 != line.end()) // If it's the last char, do not continue
          continue;
      }

            // if strnum is still empty, it means the previous char is also a
            // delim (several delims appear together). Ignore this char.
      if (strnum.empty())
        continue;

            // If we reach here, we got a number. Convert it to double.
      float       number;

      istringstream(strnum) >> number;
      matrix->back().push_back(number);

      strnum.clear();
    }
  }
}


int main(int argc, char ** argv)
{

  // IxModel<ofxGVF> gvf;
  // gvf.getTheModel()->testRndNum();

  ///////////////////////////////////
  // Database
  ///////////////////////////////////

  // Gesture names
  vector<string> gestureVocabulary;
  gestureVocabulary.push_back(string("1"));
  gestureVocabulary.push_back(string("2"));
  gestureVocabulary.push_back(string("3"));
  gestureVocabulary.push_back(string("4"));
  gestureVocabulary.push_back(string("5"));
  gestureVocabulary.push_back(string("6"));
  gestureVocabulary.push_back(string("7"));
  int vocabularySize = gestureVocabulary.size();

  // Number of trials
  int numTrials = 10;  


  // Generic Set-Up
  ofxGVFGesture currentGesture;
  ofxGVFOutcomes outcomes;
  
  ofxGVFConfig  config;
  config.inputDimensions  = 4;
  config.translate        = false;

  ofxGVFParameters  parameters;
  parameters.numberParticles       = 1000;
  parameters.resamplingThreshold   = 250;
  parameters.distribution          = 0.0f;
  parameters.alignmentVariance     = sqrt(0.000001f);
  parameters.dynamicsVariance      = vector<float>(1,sqrt(0.0001f));
  parameters.scalingsVariance      = vector<float>(1,sqrt(0.0001f));
  parameters.rotationsVariance     = vector<float>(1,sqrt(0.0f));
  parameters.predictionLoops       = 1;
  parameters.dimWeights            = vector<float>(1,sqrt(1.0f));

  ofxGVF *gvf  = new ofxGVF(config,parameters);

  vector<vector<int> > confmat;
  setMat(confmat,0,vocabularySize,vocabularySize);


  int avoidFirstNsamples = 400;

  // RUN TEST!
  int seed = -1;
  for (int testN=0; testN<10; testN++) {
    cout << "Test " << testN+1 << "/10" << endl;
    for (int subj=0; subj<2; subj++){

      // Generate random indexes
      // --------
      seed++;
      srand(seed);
      vector<vector<int> > randIndexes(vocabularySize);// could be deleted -------  
      for (int g=0; g<vocabularySize; g++)
        randIndexes[g] = getRandPerm(numTrials);


      // CLEAR GVF because new tests!
      // --------
      gvf->clear();


      // LEARNING
      // --------
      gvf->setState(ofxGVF::STATE_LEARNING);
      for (int k=0; k<vocabularySize; k++)
      {
        currentGesture.clear();
        ostringstream subjectid;
        ostringstream gestureid;
        ostringstream trialid;

        subjectid << subj+1;
        gestureid << k+1;
        trialid << randIndexes[k][0]+1;
        string fname = "/Users/caramiaux/Research/Code/Github/EMG-Analysis/PythonScripts/emganalysis_mgm021915/emganalysis_s"+subjectid.str()+"_g"+gestureid.str()+"_t"+trialid.str()+".txt";
        // cout << fname << endl;
        ifstream is(fname);
        vector<vector<float> > matrix;
        load_matrix(&is, &matrix);
        // cout << matrix.size() << endl;
        bool signalOn = false;
        for (int n=avoidFirstNsamples; n<matrix.size(); n++){

          vector<float> tmpmat;
          tmpmat.push_back(matrix[n][0]);
          tmpmat.push_back(matrix[n][1]);
          tmpmat.push_back(matrix[n][2]);
          tmpmat.push_back(matrix[n][3]);
          // tmpmat.push_back(matrix[n][2]);
          // tmpmat[2]=matrix[n][2];
          // tmpmat[3]=matrix[n][3];

          // segmentation
          // float absValAmp=0.0;
          // for (int m=0; m<matrix[n].size(); m++){
          //   absValAmp+=sqrt(matrix[n][m]*matrix[n][m])/matrix[n].size();
          // }
          // if (!signalOn){
          //   if (absValAmp>0.10)
          //     signalOn=true;
          // }
          // else{
            currentGesture.addObservation( tmpmat); //matrix[n] );
          // }
        }
        gvf->addGestureTemplate( currentGesture );
      }
      // cerr << "yo" << endl;
      // TESTING
      // --------
      gvf->setState(ofxGVF::STATE_FOLLOWING);
      ofxGVFParameters tmp = gvf->getParameters();
      // cout << "learned tolerance: " << tmp.tolerance << endl;
      gvf->setTolerance(0.3);
      for (int k=0; k<vocabularySize; k++)
      {
        gvf->restart();

        currentGesture.clear();

        // get data from file [TO CHANGE]
        ostringstream subjectid;
        ostringstream gestureid;
        ostringstream trialid;

        subjectid << subj+1;
        gestureid << k+1;
        trialid << randIndexes[k][1]+1;
        string fname = "/Users/caramiaux/Research/Code/Github/EMG-Analysis/PythonScripts/emganalysis_mgm021915/emganalysis_s"+subjectid.str()+"_g"+gestureid.str()+"_t"+trialid.str()+".txt";
        // cout << fname << endl;
        ifstream is(fname);
        vector<vector<float> > matrix;
        load_matrix(&is, &matrix);
        
        bool signalOn = false;
        for (int n=avoidFirstNsamples; n<matrix.size(); n++){

          vector<float> tmpmat;
          tmpmat.push_back(matrix[n][0]);
          tmpmat.push_back(matrix[n][1]);
          tmpmat.push_back(matrix[n][2]);
          tmpmat.push_back(matrix[n][3]);
          // tmpmat.push_back(matrix[n][2]);

          // add observation to the current gesture
          // segmentation
          // float absValAmp=0.0;
          // for (int m=0; m<matrix[n].size(); m++){
          //   // cerr << "yo" << endl;
          //   absValAmp+=sqrt(matrix[n][m]*matrix[n][m])/matrix[n].size();
          // }
          // if (!signalOn){
          //   if (absValAmp>0.10){
          //     signalOn=true;
          //     // cout << n << endl;
          //   }
          // }
          // else{
            currentGesture.addObservation( tmpmat ); //matrix[n] );
            // inference on the last observation
            gvf->update(currentGesture.getLastObservation());
          // }
        }

                  // output recognition
        int recognition = gvf->getMostProbableGestureIndex();
        // cout << " current gesture is " << k << " | recognition is " << recognition << endl;

        confmat[recognition][k]++;
      }
    }
    printMat(confmat);
    int sumdiag=0;
    int sumcolu=0;
    for (int vs=0;vs<vocabularySize;vs++){
      sumcolu+=confmat[vs][0];
      sumdiag+=confmat[vs][vs];
    }
    cout << "=> Accuracy: " << (sumdiag)/(float)(sumcolu*vocabularySize)*100 << "%%"  << endl;
  }

  

  // currentGesture.addObservation( gesture_sample );


  // gvf.addGestureTemplate( currentGesture );

  // // Following
  // gvf.setState(ofxGVF::STATE_FOLLOWING);

  // gvf.infer( gesture_sample );

  // outcomes = gvf.getOutcomes();

  return 0;

}




vector<int> getRandPerm(int n)
{
  vector<int> v(n);
  for(int i = 0; i < n; i++)
    v[i] = i;
  random_shuffle(v.begin(), v.begin()+n);
  return v;
}





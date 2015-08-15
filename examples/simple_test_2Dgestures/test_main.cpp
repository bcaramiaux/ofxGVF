
#include "ofxGVF.h"
#include "test_utils.h"

#include <istream>
#include <sstream>
#include <string>
#include <map>
#include <random>
#include <cmath>

using namespace std;



int main(int argc, char ** argv)
{

    // GVF VARIABLES
    ofxGVF *gvf;
    ofxGVFGesture currentGesture;
  
    // Build GVF with default setup
    gvf = new ofxGVF();

    // Change some parameters
    gvf->setNumberOfParticles(2000);
    gvf->setTolerance(0.2f);
    
    
    // --------------------------------
    // LEARNING
    // --------------------------------
    
    currentGesture.clear(); // in case!

    string fname = "/Users/caramiaux/Research/Code/Github/ofxGVF/_libtests/test_data/example2d_normal.txt";
    ifstream islearning(fname);
    
    vector<vector<float> > data;
    load_matrix(&islearning, &data);
    
    for (vector<vector<float> >::iterator frame = data.begin() ; frame != data.end(); ++frame)
        currentGesture.addObservation(*frame);
    
    gvf->addGestureTemplate(currentGesture);

    
    
    // --------------------------------
    // TESTING
    // --------------------------------
    
    gvf->setState(ofxGVF::STATE_FOLLOWING);

    fname = "/Users/caramiaux/Research/Code/Github/ofxGVF/_libtests/test_data/example2d_fast.txt";
    ifstream istesting(fname);
    
    currentGesture.clear();
    data.clear();
    load_matrix(&istesting, &data);
    
    for (vector<vector<float> >::iterator frame = data.begin() ; frame != data.end(); ++frame){
        currentGesture.addObservation(*frame); // in case we want to plot or to analyse gesture data
        gvf->update(currentGesture.getLastObservation());
        
        float phase = gvf->getOutcomes().estimations[0].alignment;
        float speed = gvf->getOutcomes().estimations[0].dynamics[0];
        cout << phase << " " << speed << endl;
    }
 

  return 0;

}





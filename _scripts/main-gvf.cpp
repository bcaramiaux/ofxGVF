
#include "../src/ofxGVF.h"
#include "IxModel.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>


using namespace std;


int main(int argc, char ** argv)
{

  IxModel<ofxGVF> gvf;

  gvf.getTheModel()->testRndNum();


  // // Generic Set-Up
  // gvf.setup();
  

  // // Learning
  // gvf.setState(ofxGVF::STATE_LEARNING);


  // currentGesture.addObservation( gesture_sample );


  // gvf.addGestureTemplate( currentGesture );

  // // Following
  // gvf.setState(ofxGVF::STATE_FOLLOWING);

  // gvf.infer( gesture_sample );

  // outcomes = gvf.getOutcomes();
 
  return 0;
  
}

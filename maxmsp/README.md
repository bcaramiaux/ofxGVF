ofxGVF for MaxMSP
===

Gesture Variation Follower for Max/MSP.

The Max object is in the folder maxexternal/. The Max object has been compiled with the Max SDK 6.1.4 and tested with Max 6 and Max 7, on Mac OSX 10.7,10.8,10.9.


Compilation
---

To compile the object, a Xcode Project (Mac OS) is available. To be able to compile, do

* Create a folder GVF in the your personal projects in the Max SDK, for instance for Max 6.1.4 SDK it will be in: MaxSDK-6.1.4/examples/myobjects/GVF/
  (Note that if the folder "myobjects" does not exsits, create one)
* Copy the all the files to this GVF/ folder
* In Xcode project, add the src (ofxGVF.cpp, ofxGVF.h, ofxGVFGesture.h, ofxGVFTypes.h) 
* Make sure that in ofxGVFTypes.h you set `#define OPENFRAMEWORKS 0`
* Compile 
* Once compiled, the Max/MSP object will be in the "maxexternal" folder. 
* Open the maxhelp patch to see the different messages you can use. 

Note that the library tr1 for random numbers is not included anymore in the new version of OSX so we use the std lib <random> which requires few flags in the C++ flag and to link with libc++.dylib in the build phase.




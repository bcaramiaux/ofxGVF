ofxGVF for MaxMSP
===

Xcode Project to create the GVF Max/MSP object. 

Compiled with Max SDK 6.1.4, tested in Max 7 on Mac OSX 10.9!


Installation
---

Create a folder GVF in the your personal projects in the Max SDK, for instance for Max 6.1.4 SDK it will be in:

MaxSDK-6.1.4/examples/myobjects/GVF/

Note that if the folder "myobjects" does not exsits, create one. 

Then copy the content of this repo in the folder GVF.


Compilation
---

Compiling both Max/MSP and PureData objects: make sure that in ofxGVFTypes.h you set
 
```
#define OPENFRAMEWORKS 0
```

Once compiled, the Max/MSP object will be in the "maxexternal" folder. Open the maxhelp patch to see the different messages you can use. 

Note that the library tr1 for random numbers is not included anymore in the new version of OSX so we use the std lib <random> which requires few flags in the C++ flag and to link with libc++.dylib in the build phase.




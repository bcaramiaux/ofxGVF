#ofxGVF-Kinect

This is an example of ofxGVF using MS Kinect. It works similarly to the "drawing shapes" example, using both hands as input instead of the mouse. It has some extra dependencies, so follow these steps to make it work:

1. Include these addons to your OF `addons` repository:

    * [**ofxOpenNI**](https://github.com/gameoverhack/ofxOpenNI)

    * [**ofxKinectFeatures**](https://github.com/asarasua/ofxKinectFeatures)

1. Copy the 'bin' folder from 'addons/ofxOpenNI/examples/opeNI-SimpleExamples/' to this folder (this is to have the correct folder structure and config files for openNI to work).

1. Copy the folder called 'lib' from '**ofxOpenNI**/mac/copy_to_data_openni_path' to the 'bin/data/openni' directory that has been created in step 2.

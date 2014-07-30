///////////////////////////////////////////////////////////////////////
//
//  GVF - Gesture Variation Follower Max/MSP Object
//
//
//  Copyright (C) 2013 Baptiste Caramiaux, Goldsmiths College, University of London
//
//  The GVF library is under the GNU Lesser General Public License (LGPL v3)
//  version: 09-2013
//
//
///////////////////////////////////////////////////////////////////////


#include "maxcpp6.h"
#include "ofxGVF.h"
#include "ofxGVFGesture.h"
#include <string>
#include <map>
#include <vector>
#include <unistd.h>


using namespace std;




class gvf : public MaxCpp6<gvf> {

private:

	ofxGVF              *bubi;
    ofxGVFGesture       currentGesture;
    ofxGVFConfig        config;
    ofxGVFParameters    parameters;
    ofxGVFOutcomes      outcomes;

public:
    

    gvf(t_symbol * sym, long argc, t_atom *argv)
    {
        
        setupIO(1, 3); // inlets / outlets
        
        post("gvf - realtime adaptive gesture recognition (version: 07-2014)");
        post("(c) Goldsmiths, University of London and Ircam - Centre Pompidou");
        
	    // CONFIGURATION of the GVF
        config.inputDimensions  = 2;
        config.translate        = true;
        config.segmentation     = false;
        
        // PARAMETERS are set by default
        
        // CREATE the corresponding GVF
        bubi = new ofxGVF(config);
		
    }
    
    ~gvf()
    {
        //post("destroying gvf object");
		if(bubi != NULL)
			delete bubi;
    }
    
	
	// methods:
	void bang(long inlet) {
		outlet_bang(m_outlets[0]);
	}
	void testfloat(long inlet, double v) {
        post(" float");
		outlet_float(m_outlets[0], v);
	}
	void testint(long inlet, long v) {
        post(" long");
		outlet_int(m_outlets[0], v);
	}
    

    
    
    ///////////////////////////////////////////////////////////
    //====================== LEARN
    ///////////////////////////////////////////////////////////
	void learn(long inlet, t_symbol * s, long ac, t_atom * av)
    {
        
        bubi->setState(ofxGVF::STATE_LEARNING);
        currentGesture.clear();
        
    }
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== FOLLOW
    ///////////////////////////////////////////////////////////
    void follow(long inlet, t_symbol * s, long ac, t_atom * av)
    {
        bubi->setState(ofxGVF::STATE_FOLLOWING);
    }
   
    void gestureOn(long inlet, t_symbol * s, long ac, t_atom * av)
    {
        // nothing
    }
    
    
    void gestureOff(long inlet, t_symbol * s, long ac, t_atom * av)
    {
    
        // add the current gesture to the template if in learning mode and
        // the current gesture not empty!
        if (bubi->getState()==ofxGVF::STATE_LEARNING && (currentGesture.getTemplateLength()>0))
            bubi->addGestureTemplate(currentGesture);
    
    }
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== DATA
    ///////////////////////////////////////////////////////////
    void data(long inlet, t_symbol * s, long ac, t_atom * av) {
        
        if(ac == 0)
        {
            post("invalid format, points have at least 1 coordinate");
            return;
        }
        
        // INCOMING DATA - CLEAR MODE
        // -----------------------------
        if(bubi->getState() == ofxGVF::STATE_CLEAR)
        {
            post("I'm in a standby (must learn something beforehand)");
            return;
        }
        
        switch(bubi->getState()){
                
            case ofxGVF::STATE_LEARNING:
            {
                vector<float> observation_vector(ac);
                for (int k=0; k<ac; k++)
                    observation_vector[k] = atom_getfloat(&av[k]);

                currentGesture.addObservation(observation_vector);

                break;
            }
                
            case ofxGVF::STATE_FOLLOWING:
            {
                vector<float> observation_vector(ac);
                for (int k=0; k<ac; k++)
                    observation_vector[k] = atom_getfloat(&av[k]);
                
                currentGesture.addObservation(observation_vector);
                
                // inference on the last observation
                bubi->infer(currentGesture.getLastObservation());

                
                // output recognition
                outcomes = bubi->getOutcomes();
                int numberOfTemplates = outcomes.estimations.size();
                
                t_atom *outAtoms = new t_atom[numberOfTemplates];
                
                for(int j = 0; j < numberOfTemplates; j++)
                    atom_setfloat(&outAtoms[j],outcomes.estimations[j].phase);
                outlet_anything(m_outlets[0], gensym("phase"), numberOfTemplates, outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[numberOfTemplates];
                for(int j = 0; j < numberOfTemplates; j++)
                    atom_setfloat(&outAtoms[j],outcomes.estimations[j].speed);
                outlet_anything(m_outlets[0], gensym("speed"), numberOfTemplates, outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[numberOfTemplates * outcomes.estimations[0].scale.size()];
                for(int j = 0; j < numberOfTemplates; j++)
                    for(int jj = 0; jj < outcomes.estimations[0].scale.size(); jj++)
                    atom_setfloat(&outAtoms[j],outcomes.estimations[j].scale[jj]);
                outlet_anything(m_outlets[0], gensym("scale"), numberOfTemplates * outcomes.estimations[0].scale.size(), outAtoms);
                delete[] outAtoms;

                outAtoms = new t_atom[numberOfTemplates * outcomes.estimations[0].rotation.size()];
                for(int j = 0; j < numberOfTemplates; j++)
                    for(int jj = 0; jj < outcomes.estimations[0].rotation.size(); jj++)
                        atom_setfloat(&outAtoms[j],outcomes.estimations[j].rotation[jj]);
                outlet_anything(m_outlets[0], gensym("angle"), numberOfTemplates * outcomes.estimations[0].rotation.size(), outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[numberOfTemplates];
                for(int j = 0; j < numberOfTemplates; j++)
                    atom_setfloat(&outAtoms[j],outcomes.estimations[j].probability);
                outlet_anything(m_outlets[1], gensym("weights"), numberOfTemplates, outAtoms);
                delete[] outAtoms;
                /*
                outcomes = bubi->getOutcomes();
                
                t_atom *outAtoms = new t_atom[outcomes.allPhases.size()];
                
                for(int j = 0; j < outcomes.allPhases.size(); j++)
                    atom_setfloat(&outAtoms[j],outcomes.allPhases[j]);
                outlet_anything(m_outlets[0], gensym("phase"), outcomes.allPhases.size(), outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[outcomes.allSpeeds.size()];
                for(int j = 0; j < outcomes.allSpeeds.size(); j++)
                    atom_setfloat(&outAtoms[j],outcomes.allSpeeds[j]);
                outlet_anything(m_outlets[0], gensym("speed"), outcomes.allSpeeds.size(), outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[outcomes.allScales.size()];
                for(int j = 0; j < outcomes.allScales.size(); j++)
                    atom_setfloat(&outAtoms[j],outcomes.allScales[j]);
                outlet_anything(m_outlets[0], gensym("scale"), outcomes.allScales.size(), outAtoms);
                delete[] outAtoms;

                outAtoms = new t_atom[outcomes.allRotations.size()];
                for(int j = 0; j < outcomes.allRotations.size(); j++)
                    atom_setfloat(&outAtoms[j],outcomes.allRotations[j]);
                outlet_anything(m_outlets[0], gensym("angle"), outcomes.allRotations.size(), outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[outcomes.allProbabilities.size()];
                for(int j = 0; j < outcomes.allProbabilities.size(); j++)
                    atom_setfloat(&outAtoms[j],outcomes.allProbabilities[j]);
                outlet_anything(m_outlets[1], gensym("weights"), outcomes.allProbabilities.size(), outAtoms);
                delete[] outAtoms;
                */
                 
                break;
            }
            
            default:
                // nothing
                break;
        }
    }
    
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== savetemplates
    ///////////////////////////////////////////////////////////
    void savetemplates(long inlet, t_symbol * s, long ac, t_atom * av){
        char* mpath = atom_string(av);
        string filename(mpath);
        bubi->saveTemplates(filename);
        
    }
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== loadtemplates
    ///////////////////////////////////////////////////////////
    void loadtemplates(long inlet, t_symbol * s, long ac, t_atom * av){
        char* mpath = atom_string(av);
        int i=0;
        while ( *(mpath+i)!='/' )
            i++;
        mpath = mpath+i;
        string filename(mpath);
        bubi->loadTemplates(filename);

    }
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== CLEAR
    ///////////////////////////////////////////////////////////
    void clear(long inlet, t_symbol * s, long ac, t_atom * av) {
        
        bubi->clear();
        
    }
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== PRINTME
    ///////////////////////////////////////////////////////////
    void printme(long inlet, t_symbol * s, long ac, t_atom * av) {
        
        post("Number of particles %d", bubi->getNumberOfParticles());
        post("Resampling Threshold %d", bubi->getResamplingThreshold());
        post("Tolerance %.2f", bubi->getParameters().tolerance);
        for(int i = 0; i < bubi->getNumberOfGestureTemplates(); i++)
        {
            post("reference %d: ", i);
            
            vector<vector<float> > tplt = bubi->getGestureTemplate(i).getTemplate();
            
            for(int j = 0; j < tplt.size(); j++)
            {
                post("%02.4f  %02.4f", tplt[j][0], tplt[j][1]);
            }
        }
        
    }
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== RESTART
    ///////////////////////////////////////////////////////////
    void restart(long inlet, t_symbol * s, long ac, t_atom * av)
    {
        
        currentGesture.clear();
        
        if(bubi->getState() == ofxGVF::STATE_FOLLOWING)
            bubi->spreadParticles();
    }
    
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== tolerance
    ///////////////////////////////////////////////////////////
    void tolerance(long inlet, t_symbol * s, long ac, t_atom * av) {
        float stdnew = atom_getfloat(&av[0]);
        if (stdnew == 0.0)
            stdnew = 0.1;
        parameters=bubi->getParameters();
        parameters.tolerance=stdnew;
        bubi->setParameters(parameters);
    }
    
    ///////////////////////////////////////////////////////////
    //====================== resampling_threshold
    ///////////////////////////////////////////////////////////
    void resampling_threshold(long inlet, t_symbol * s, long ac, t_atom * av) {
        int rtnew = atom_getlong(&av[0]);
        int cNS = bubi->getNumberOfParticles();
        if (rtnew >= cNS)
            rtnew = floor(cNS/2);
        bubi->setResamplingThreshold(rtnew);
    }

    ///////////////////////////////////////////////////////////
    //====================== spreading_means
    ///////////////////////////////////////////////////////////
    void rotation_spreading(long inlet, t_symbol * s, long ac, t_atom * av) {
        // do something
        parameters=bubi->getParameters();
        config=bubi->getConfig();
        if (config.inputDimensions==2)
            parameters.rotationInitialSpreading[0]=atom_getfloat(&av[0]);
        else if (config.inputDimensions==3) {
            if (ac==1)
                for (int k=0; k<config.inputDimensions; k++)
                    parameters.rotationInitialSpreading[k]=atom_getfloat(&av[0]);
            if (ac==3)
                for (int k=0; k<config.inputDimensions; k++)
                    parameters.rotationInitialSpreading[k]=atom_getfloat(&av[k]);
        }
        bubi->setParameters(parameters);
        
    }
    
    
    ///////////////////////////////////////////////////////////
    //====================== spreading_means
    ///////////////////////////////////////////////////////////
    void spreading_means(long inlet, t_symbol * s, long ac, t_atom * av) {
        // do something
    }
    
    ///////////////////////////////////////////////////////////
    //====================== spreading_ranges
    ///////////////////////////////////////////////////////////
    void spreading_ranges(long inlet, t_symbol * s, long ac, t_atom * av) {
        // do something
    }
    
    ///////////////////////////////////////////////////////////
    //====================== adaptation_speed
    ///////////////////////////////////////////////////////////
    void adaptation_speed(long inlet, t_symbol * s, long ac, t_atom * av) {
        vector<float> as;
        as.push_back(atom_getfloat(&av[0]));
        as.push_back(atom_getfloat(&av[1]));
        as.push_back(atom_getfloat(&av[2]));
        as.push_back(atom_getfloat(&av[3]));
        
        // do something
    }
    
    
    ///////////////////////////////////////////////////////////
    //====================== translate
    ///////////////////////////////////////////////////////////
    void translate(long inlet, t_symbol * s, long ac, t_atom * av){
        config = bubi->getConfig();
        config.translate = (atom_getlong(&av[0])==0)? false : true;
        bubi->setConfig(config);
    }
    
    ///////////////////////////////////////////////////////////
    //====================== translate
    ///////////////////////////////////////////////////////////
    void segmentation(long inlet, t_symbol * s, long ac, t_atom * av){
        config = bubi->getConfig();
        config.segmentation = (atom_getlong(&av[0])==0)? false : true;
        bubi->setConfig(config);
    }
    
};


//THIS IS FOR Max6.1

//C74_EXPORT extern "C" int main(void) {
//	// create a class with the given name:
//	gvf::makeMaxClass("gvf");
//	REGISTER_METHOD(gvf, bang);
//	REGISTER_METHOD_FLOAT(gvf, testfloat);
//	REGISTER_METHOD_LONG(gvf, testint);
//	REGISTER_METHOD_GIMME(gvf, test);
//}


extern "C" int main(void) {
    // create a class with the given name:
    gvf::makeMaxClass("gvf");
    REGISTER_METHOD(gvf, bang);
    REGISTER_METHOD_FLOAT(gvf, testfloat);
    REGISTER_METHOD_LONG(gvf,  testint);
    REGISTER_METHOD_GIMME(gvf, learn);
    REGISTER_METHOD_GIMME(gvf, follow);
    REGISTER_METHOD_GIMME(gvf, clear);
    REGISTER_METHOD_GIMME(gvf, data);
    REGISTER_METHOD_GIMME(gvf, printme);
    REGISTER_METHOD_GIMME(gvf, restart);
    REGISTER_METHOD_GIMME(gvf, tolerance);
    REGISTER_METHOD_GIMME(gvf, resampling_threshold);
    REGISTER_METHOD_GIMME(gvf, spreading_means);
    REGISTER_METHOD_GIMME(gvf, spreading_ranges);
    REGISTER_METHOD_GIMME(gvf, adaptation_speed);
    REGISTER_METHOD_GIMME(gvf, savetemplates);
    REGISTER_METHOD_GIMME(gvf, loadtemplates);
    REGISTER_METHOD_GIMME(gvf, translate);
    REGISTER_METHOD_GIMME(gvf, segmentation);
    REGISTER_METHOD_GIMME(gvf, gestureOff);
    REGISTER_METHOD_GIMME(gvf, gestureOn);
        REGISTER_METHOD_GIMME(gvf, rotation_spreading);
}

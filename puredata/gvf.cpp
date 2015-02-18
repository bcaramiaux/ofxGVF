
///////////////////////////////////////////////////////////////////////
//
//  GVF - Gesture Variation Follower PureData Object
//
//
//  Copyright (C) 2014 Baptiste Caramiaux, Goldsmiths College, University of London
//
//  The GVF library is under the GNU Lesser General Public License (LGPL v3)
//  version: 11-2014
//
///////////////////////////////////////////////////////////////////////


//#include <boost/random.hpp>
#include <vector>
#include "m_pd.h"
#include "ofxGVF.h"
#include "ofxGVFGesture.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

static t_class *gvf_class;


// Building Structure GVF
// ----------------------
typedef struct _gvf {

    // object 
    t_object  x_obj;
    
    // GVF related variables
    ofxGVF              *bubi;
	ofxGVFGesture       *currentGesture;
    ofxGVFConfig        config;
    ofxGVFParameters    parameters;
    ofxGVFOutcomes      outcomes;

    // outlets
    t_outlet *Position,*Vitesse,*Scaling,*Rotation,*Likelihoods,*Info;

} t_gvf;



// MESSAGES THAT CAN BE RECEIVED FROM PURE DATA
// =============================================

//// BASICS
static void gvf_learn           (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_start           (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_stop            (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_follow          (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_clear           (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_list            (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_printme         (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_restart         (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);

//// CONFIG
static void gvf_translate       (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_segmentation    (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);

//// PARAMETERS
static void gvf_tolerance       (t_gvf *x,const t_symbol *sss, int argc, t_atom *argv);
static void gvf_numberparticles       (t_gvf *x,const t_symbol *sss, int argc, t_atom *argv);
static void gvf_resamplingthreshold     (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_phaseadaptation (t_gvf *x,const t_symbol *sss, int argc, t_atom *argv);
static void gvf_speedadaptation (t_gvf *x,const t_symbol *sss, int argc, t_atom *argv);
static void gvf_scaleadaptation (t_gvf *x,const t_symbol *sss, int argc, t_atom *argv);
static void gvf_rotationadaptation (t_gvf *x,const t_symbol *sss, int argc, t_atom *argv);


//// I/O
static void gvf_savetemplates   (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_loadtemplates   (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);


//// DEPRECATED
static void gvf_gestureOn       (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_gestureOff      (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_adaptspeed      (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_data            (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);


static void gvf_auto(t_gvf *x);





// BUILDING THE OBJECT
// ===================

static void *gvf_new(t_symbol *s, int argc, t_atom *argv)
{
    post("\ngvf - gesture variation follower (version: 11-2014)");
    post("(c) Goldsmiths, University of London and IRCAM - Centre Pompidou");

    t_gvf *x = (t_gvf *)pd_new(gvf_class);


        
    // CONFIGURATION of the GVF
    x->config.inputDimensions  = 2;   
    x->config.translate        = true;
    x->config.segmentation     = false;
        
    // PARAMETERS are set by default
        
    // CREATE the corresponding GVF
    x->bubi = new ofxGVF(x->config);

    // CREATE THE GESTURE
    x->currentGesture = new ofxGVFGesture();


    // OUTLETS
    x->Position     = outlet_new(&x->x_obj, &s_list);
    x->Vitesse      = outlet_new(&x->x_obj, &s_list);
    x->Scaling      = outlet_new(&x->x_obj, &s_list);
    x->Rotation     = outlet_new(&x->x_obj, &s_list);
    x->Likelihoods  = outlet_new(&x->x_obj, &s_list);
    x->Info         = outlet_new(&x->x_obj, &s_list);
    
    return (void *)x;
}

static void gvf_destructor(t_gvf *x)
{
    //post("gvf destructor...");
    if(x->bubi != NULL)
        delete x->bubi;
}




///////////////////////////////////////////////////////////
//====================== LEARN
///////////////////////////////////////////////////////////
static void gvf_learn(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    x->bubi->setState(ofxGVF::STATE_LEARNING);
    if (argc==0){
        
        // output the current ID of the gesture being learned with the prefix "learningGesture"
        /*t_atom *outAtoms = new t_atom[1];
        atom_setlong(&outAtoms[0],x->bubi->getNumberOfGestureTemplates()+1);
        outlet_anything(x->info_outlet, gensym("learningGesture"), 1, outAtoms);
        delete[] outAtoms;*/
        
    }
    else if (argc==1) {
        
    }
    else if (argc==2) {
        
    }
    else {
        error("wrong number of argument in learn message");
    }
}


///////////////////////////////////////////////////////////
//====================== START
///////////////////////////////////////////////////////////
void gvf_start(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    if (x->bubi->getState()==ofxGVF::STATE_LEARNING)
        x->currentGesture->clear();
}


///////////////////////////////////////////////////////////
//====================== STOP
///////////////////////////////////////////////////////////
void gvf_stop(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    if (x->bubi->getState()==ofxGVF::STATE_LEARNING && (x->currentGesture->getTemplateLength()>0))
        x->bubi->addGestureTemplate(*(x->currentGesture));
}


///////////////////////////////////////////////////////////
//====================== FOLLOW
///////////////////////////////////////////////////////////
static void gvf_follow(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
        x->bubi->setState(ofxGVF::STATE_FOLLOWING);
}


// !!!DEPRECATED
static void gvf_gestureOn(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
     error("gestureOn message deprecated: do nothing");
}
    
// !!!DEPRECATED
static void gvf_gestureOff(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
     error("gestureOff message deprecated: do nothing");
            
}


///////////////////////////////////////////////////////////
//====================== LIST
///////////////////////////////////////////////////////////
void gvf_list(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    
    if(argc == 0)
    {
        post("invalid format, points have at least 1 coordinate");
        return;
    }
    if(x->bubi->getState() == ofxGVF::STATE_CLEAR)
    {
        post("I'm in a standby (must learn something beforehand)");
        return;
    }
    
    switch (x->bubi->getState()) {
        case ofxGVF::STATE_LEARNING:
        {
            vector<float> observation_vector(argc);
            
            for (int k=0; k<argc; k++)
                observation_vector[k] = atom_getfloat(&argv[k]);
            
            x->currentGesture->addObservation(observation_vector);
            
            break;
        }
        case ofxGVF::STATE_FOLLOWING:
        {
            vector<float> observation_vector(argc);
            for (int k=0; k<argc; k++)
                observation_vector[k] = atom_getfloat(&argv[k]);
            
            x->currentGesture->addObservation(observation_vector);
            
            // inference on the last observation
            x->bubi->infer(x->currentGesture->getLastObservation());
            
            
            // output recognition
            x->outcomes = x->bubi->getOutcomes();
            int numberOfTemplates = x->outcomes.estimations.size();
            
            t_atom *outAtoms = new t_atom[numberOfTemplates];
            
            for(int j = 0; j < numberOfTemplates; j++)
                SETFLOAT(&outAtoms[j],x->outcomes.estimations[j].phase);
            outlet_list(x->Position, &s_list, numberOfTemplates, outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[numberOfTemplates];
            for(int j = 0; j < numberOfTemplates; j++)
                SETFLOAT(&outAtoms[j],x->outcomes.estimations[j].speed);
            outlet_list(x->Vitesse, &s_list, numberOfTemplates, outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[numberOfTemplates * x->outcomes.estimations[0].scale.size()];
            for(int j = 0; j < numberOfTemplates; j++)
                for(int jj = 0; jj < x->outcomes.estimations[0].scale.size(); jj++)
                    SETFLOAT(&outAtoms[j],x->outcomes.estimations[j].scale[jj]);
            outlet_list(x->Scaling, &s_list, numberOfTemplates * x->outcomes.estimations[0].scale.size(), outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[numberOfTemplates * x->outcomes.estimations[0].rotation.size()];
            for(int j = 0; j < numberOfTemplates; j++)
                for(int jj = 0; jj < x->outcomes.estimations[0].rotation.size(); jj++)
                    SETFLOAT(&outAtoms[j],x->outcomes.estimations[j].rotation[jj]);
            outlet_list(x->Rotation, &s_list, numberOfTemplates * x->outcomes.estimations[0].rotation.size(), outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[numberOfTemplates];
            for(int j = 0; j < numberOfTemplates; j++)
                SETFLOAT(&outAtoms[j],x->outcomes.estimations[j].probability);
            outlet_list(x->Likelihoods, &s_list, numberOfTemplates, outAtoms);
            delete[] outAtoms;
            
            break;
        }
            
        default:
            // nothing
            break;
    }
    
}



///////////////////////////////////////////////////////////
//====================== DATA
///////////////////////////////////////////////////////////
static void gvf_data(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{

    if(argc == 0)
    {
        post("invalid format, points have at least 1 coordinate");
        return;
    }
    if(x->bubi->getState() == ofxGVF::STATE_CLEAR)
    {
        post("I'm in a standby (must learn something beforehand)");
        return;
    }

    switch (x->bubi->getState()) {
         case ofxGVF::STATE_LEARNING:
            {
                vector<float> observation_vector(argc);
                for (int k=0; k<argc; k++)
                    observation_vector[k] = atom_getfloat(&argv[k]);

                x->currentGesture->addObservation(observation_vector);
                //post("IN: %f %f", observation_vector[0], observation_vector[1]);

                break;
            }
        case ofxGVF::STATE_FOLLOWING:
            {
                vector<float> observation_vector(argc);
                for (int k=0; k<argc; k++)
                    observation_vector[k] = atom_getfloat(&argv[k]);
                
                x->currentGesture->addObservation(observation_vector);
                
                // inference on the last observation
                x->bubi->infer(x->currentGesture->getLastObservation());

                
                // output recognition
                x->outcomes = x->bubi->getOutcomes();
                int numberOfTemplates = x->outcomes.estimations.size();
                
                t_atom *outAtoms = new t_atom[numberOfTemplates];
                
                for(int j = 0; j < numberOfTemplates; j++)
                    SETFLOAT(&outAtoms[j],x->outcomes.estimations[j].phase);
                outlet_list(x->Position, &s_list, numberOfTemplates, outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[numberOfTemplates];
                for(int j = 0; j < numberOfTemplates; j++)
                    SETFLOAT(&outAtoms[j],x->outcomes.estimations[j].speed);
                outlet_list(x->Vitesse, &s_list, numberOfTemplates, outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[numberOfTemplates * x->outcomes.estimations[0].scale.size()];
                for(int j = 0; j < numberOfTemplates; j++)
                    for(int jj = 0; jj < x->outcomes.estimations[0].scale.size(); jj++)
                        SETFLOAT(&outAtoms[j],x->outcomes.estimations[j].scale[jj]);
                outlet_list(x->Scaling, &s_list, numberOfTemplates * x->outcomes.estimations[0].scale.size(), outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[numberOfTemplates * x->outcomes.estimations[0].rotation.size()];
                for(int j = 0; j < numberOfTemplates; j++)
                    for(int jj = 0; jj < x->outcomes.estimations[0].rotation.size(); jj++)
                        SETFLOAT(&outAtoms[j],x->outcomes.estimations[j].rotation[jj]);
                outlet_list(x->Rotation, &s_list, numberOfTemplates * x->outcomes.estimations[0].rotation.size(), outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[numberOfTemplates];
                for(int j = 0; j < numberOfTemplates; j++)
                    SETFLOAT(&outAtoms[j],x->outcomes.estimations[j].probability);
                outlet_list(x->Likelihoods, &s_list, numberOfTemplates, outAtoms);
                delete[] outAtoms;
                /*
                x->outcomes = x->bubi->getOutcomes();
                
                t_atom *outAtoms = new t_atom[x->outcomes.allPhases.size()];
                
                for(int j = 0; j < x->outcomes.allPhases.size(); j++)
                    SETFLOAT(&outAtoms[j],x->outcomes.allPhases[j]);
                outlet_list(x->Position, &s_list, x->outcomes.allPhases.size(), outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[x->outcomes.allSpeeds.size()];
                for(int j = 0; j < x->outcomes.allSpeeds.size(); j++)
                    SETFLOAT(&outAtoms[j],x->outcomes.allSpeeds[j]);
                outlet_list(x->Vitesse, &s_list, x->outcomes.allSpeeds.size(), outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[x->outcomes.allScales.size()];
                for(int j = 0; j < x->outcomes.allScales.size(); j++)
                    SETFLOAT(&outAtoms[j],x->outcomes.allScales[j]);
                outlet_list(x->Scaling, &s_list, x->outcomes.allScales.size(), outAtoms);
                delete[] outAtoms;

                outAtoms = new t_atom[x->outcomes.allRotations.size()];
                for(int j = 0; j < x->outcomes.allRotations.size(); j++)
                    SETFLOAT(&outAtoms[j],x->outcomes.allRotations[j]);
                outlet_list(x->Rotation, &s_list, x->outcomes.allRotations.size(), outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[x->outcomes.allProbabilities.size()];
                for(int j = 0; j < x->outcomes.allProbabilities.size(); j++)
                    SETFLOAT(&outAtoms[j],x->outcomes.allProbabilities[j]);
                outlet_list(x->Likelihoods, &s_list, x->outcomes.allProbabilities.size(), outAtoms);
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
//====================== CLEAR
///////////////////////////////////////////////////////////
void gvf_clear(t_gvf *x, const t_symbol *sss, int argc, t_atom *argv)
{
    x->bubi->clear();
    
    // output 0 for the number of learned gesture
    /*t_atom *outAtoms = new t_atom[1];
    atom_setlong(&outAtoms[0],0);
    outlet_anything(x->Info, gensym("learningGesture"), 1, outAtoms);
    delete[] outAtoms;*/
}



///////////////////////////////////////////////////////////
//====================== PRINTME
///////////////////////////////////////////////////////////
void gvf_printme(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    post("======================");
    post("Number of particles: %d", x->bubi->getNumberOfParticles());
    post("Resampling Threshold: %d", x->bubi->getResamplingThreshold());
    post("Tolerance: %.2f", x->bubi->getParameters().tolerance);
    post("Adaptation parameters:");
    post("  phase: %.7f", x->bubi->getParameters().phaseVariance);
    post("  speed: %.7f", x->bubi->getParameters().speedVariance);
    string scale_str = "scale: ";
    for (int k=0; k<x->bubi->getParameters().scaleVariance.size(); k++) {
        std::ostringstream ss;
        ss << x->bubi->getParameters().scaleVariance[k];
        scale_str = scale_str + ss.str() + " ";
    }
    post("  %s", scale_str.c_str());
    scale_str = "rotation: ";
    for (int k=0; k<x->bubi->getParameters().rotationVariance.size(); k++) {
        std::ostringstream ss;
        ss << x->bubi->getParameters().rotationVariance[k];
        scale_str = scale_str + ss.str() + " ";
    }
    post("  %s", scale_str.c_str());
    post("Number of recorded templates: %d", x->bubi->getNumberOfGestureTemplates());
    post("======================");
    for(int i = 0; i < x->bubi->getNumberOfGestureTemplates(); i++)
    {
        post("reference %d: ", i+1);
        
        vector<vector<float> > tplt = x->bubi->getGestureTemplate(i).getTemplate();
        
        for(int j = 0; j < tplt.size(); j++)
        {
            std::ostringstream ss;
            for(int k = 0; k < tplt[0].size(); k++){
                ss << tplt[j][k] << " ";
            }
            post("%s", ss.str().c_str());
        }
    }
}



///////////////////////////////////////////////////////////
//====================== SAVE_VOCABULARY
///////////////////////////////////////////////////////////
static void gvf_savetemplates(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    /*
     char* mpath = atom_getsymbol(argv);
     string filename(mpath);
     x->bubi->saveTemplates(filename);
     */
}


///////////////////////////////////////////////////////////
//====================== LOAD_VOCABULARY
///////////////////////////////////////////////////////////
static void gvf_loadtemplates(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    /*
        char* mpath = atom_string(argv);
        int i=0;
        while ( *(mpath+i)!='/' )
            i++;
        mpath = mpath+i;
        string filename(mpath);
        x->bubi->loadTemplates(filename);
    */
}


///////////////////////////////////////////////////////////
//====================== RESTART
///////////////////////////////////////////////////////////
static void gvf_restart(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    
        x->currentGesture->clear();
        
        if(x->bubi->getState() == ofxGVF::STATE_FOLLOWING)
            x->bubi->spreadParticles();
            
}


///////////////////////////////////////////////////////////
//====================== tolerance
///////////////////////////////////////////////////////////
static void gvf_tolerance(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    float stdnew = atom_getfloat(argv);
    if (stdnew == 0.0)
        stdnew = 0.1;
    
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change Tolerance
    x->parameters.tolerance=stdnew;
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
}


///////////////////////////////////////////////////////////
//====================== resampling_threshold
///////////////////////////////////////////////////////////
static void gvf_resamplingthreshold(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    int rtnew = atom_getint(&argv[0]);
    int cNS = x->bubi->getNumberOfParticles();
    if (rtnew >= cNS)
        rtnew = floor(cNS/2);
    
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change Resampling threshold
    x->parameters.resamplingThreshold  = rtnew;
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
}


///////////////////////////////////////////////////////////
//====================== numberOfParticles
///////////////////////////////////////////////////////////
void gvf_numberparticles(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    
    int nsNew = atom_getint(&argv[0]);
    
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change Resampling threshold
    x->parameters.numberParticles  = nsNew;
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
    
}


///////////////////////////////////////////////////////////
//====================== adaptation_speed
///////////////////////////////////////////////////////////
static void gvf_adaptation_speed(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    post("adaptation_speed message deprecated: do nothing");
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change ...
    
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
}


///////////////////////////////////////////////////////////
//====================== phaseAdaptation / scaleAdaptation / speedAdaptation / angleAdaptation
///////////////////////////////////////////////////////////
void gvf_phaseadaptation(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change phase adaptation variance
    x->parameters.phaseVariance = atom_getfloat(argv);
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
}
void gvf_speedadaptation(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change ...
    x->parameters.speedVariance = atom_getfloat(argv);
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
}
void gvf_scaleadaptation(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change ...
    if (argc == x->parameters.scaleVariance.size()) {
        for (int k=0; k< argc; k++)
            x->parameters.scaleVariance[k] = atom_getfloat(&argv[k]);
    }
    if (argc == 1) {
        for (int k=0; k< x->parameters.scaleVariance.size(); k++)
            x->parameters.scaleVariance[k] = atom_getfloat(argv);
    }
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
}
void gvf_rotationadaptation(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change ...
    if (argc == x->parameters.rotationVariance.size()) {
        for (int k=0; k< argc; k++)
            x->parameters.rotationVariance[k] = atom_getfloat(&argv[k]);
    }
    if (argc == 1) {
        for (int k=0; k< x->parameters.rotationVariance.size(); k++)
            x->parameters.rotationVariance[k] = atom_getfloat(argv);
    }
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
}



///////////////////////////////////////////////////////////
//====================== translate
///////////////////////////////////////////////////////////
static void gvf_translate(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
        x->config = x->bubi->getConfig();
        x->config.translate = (atom_getint(&argv[0])==0)? false : true;
        x->bubi->setConfig(x->config);
}
    
///////////////////////////////////////////////////////////
//====================== translate
///////////////////////////////////////////////////////////
static void gvf_segmentation(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
        x->config = x->bubi->getConfig();
        x->config.segmentation = (atom_getint(&argv[0])==0)? false : true;
        x->bubi->setConfig(x->config);
}


extern "C"
{
    void gvf_setup(void) {
        gvf_class = class_new( gensym("gvf"),(t_newmethod)gvf_new,(t_method)gvf_destructor,
                                sizeof(t_gvf),CLASS_DEFAULT,A_GIMME,0);
        
        //  MESSAGES
        
        // basics
        class_addmethod(gvf_class,(t_method)gvf_learn,gensym("learn"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_start,gensym("start"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_stop,gensym("stop"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_follow,gensym("follow"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_clear,gensym("clear"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_list,gensym("list"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_printme,gensym("printme"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_restart,gensym("restart"),A_GIMME,0);
        
        // config
        class_addmethod(gvf_class,(t_method)gvf_translate,gensym("translate"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_segmentation,gensym("segmentation"),A_GIMME,0);
    
        // parameters
        class_addmethod(gvf_class, (t_method)gvf_tolerance,gensym("tolerance"), A_GIMME, 0);
        class_addmethod(gvf_class, (t_method)gvf_resamplingthreshold,gensym("resamplingthreshold"), A_GIMME, 0);
        class_addmethod(gvf_class, (t_method)gvf_numberparticles, gensym("numberparticles"), A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_phaseadaptation, gensym("phaseadaptation"), A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_speedadaptation, gensym("speedadaptation"), A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_scaleadaptation, gensym("scaleadaptation"), A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_rotationadaptation, gensym("rotationadaptation"), A_GIMME,0);
        
        // I/O
        class_addmethod(gvf_class,(t_method)gvf_savetemplates,gensym("savetemplates"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_loadtemplates,gensym("loadtemplates"),A_GIMME,0);

        
        // deprecated
        class_addmethod(gvf_class,(t_method)gvf_data,gensym("data"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_gestureOn,gensym("gestureOn"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_gestureOff,gensym("gestureOff"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_adaptation_speed,gensym("adaptation_speed"),A_GIMME,0);
        


    }
}

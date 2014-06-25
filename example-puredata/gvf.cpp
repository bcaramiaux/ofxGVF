
///////////////////////////////////////////////////////////////////////
//
//  GVF - Gesture Variation Follower PureData Object
//
//
//  Copyright (C) 2014 Baptiste Caramiaux, Goldsmiths College, University of London
//
//  The GVF library is under the GNU Lesser General Public License (LGPL v3)
//  version: 05-2014
//
///////////////////////////////////////////////////////////////////////


//#include <boost/random.hpp>
#include <vector>
#include "m_pd.h"
#include "ofxGVF.h"
#include "ofxGVFGesture.h"
#include <iostream>
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
    t_outlet *Position,*Vitesse,*Scaling,*Rotation,*Likelihoods;

} t_gvf;



// MESSAGES THAT CAN BE RECEIVED FROM PURE DATA
// =============================================

static void gvf_learn           (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_follow          (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_clear           (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_data            (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_printme         (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_restart         (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_std             (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_rt              (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_means           (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_ranges          (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_adaptspeed      (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_savetemplates   (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_loadtemplates   (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_gestureOn       (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_gestureOff      (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_translate       (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);
static void gvf_segmentation    (t_gvf *x,const t_symbol *sss,int argc, t_atom *argv);

static void gvf_auto(t_gvf *x);





// BUILDING THE OBJECT
// ===================

static void *gvf_new(t_symbol *s, int argc, t_atom *argv)
{
    post("\ngvf - realtime adaptive gesture recognition (version: 06-2014)");
    post("(c) Goldsmiths, University of London and Ircam - Centre Pompidou");

    t_gvf *x = (t_gvf *)pd_new(gvf_class);


        
    // CONFIGURATION of the GVF
    x->config.inputDimensions  = 2;   
    x->config.translate        = true;
    x->config.segmentation     = true;
        
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
        x->currentGesture->clear();
        post("Learn one gesture...");
}


///////////////////////////////////////////////////////////
//====================== FOLLOW
///////////////////////////////////////////////////////////
static void gvf_follow(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
        x->bubi->setState(ofxGVF::STATE_FOLLOWING);
        post("Follow gesture...");
}


static void gvf_gestureOn(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
        // nothing
}
    
    
static void gvf_gestureOff(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
        // add the current gesture to the template if in learning mode and
        // the current gesture not empty!
    
        if (x->bubi->getState()==ofxGVF::STATE_LEARNING && (x->currentGesture->getTemplateLength()>0))
            x->bubi->addGestureTemplate(*(x->currentGesture));
            
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
                 
                break;
            }
            
            default:
                // nothing
                break;
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
//====================== CLEAR
///////////////////////////////////////////////////////////
static void gvf_clear(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    x->bubi->clear();
    post("Clear!");
}


///////////////////////////////////////////////////////////
//====================== PRINTME
///////////////////////////////////////////////////////////
static void gvf_printme(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{  
        post("Number of particles %d", x->bubi->getNumberOfParticles());
        post("Resampling Threshold %d", x->bubi->getResamplingThreshold());
        post("Tolerance %.2f", x->bubi->getParameters().tolerance);
        for(int i = 0; i < x->bubi->getNumberOfGestureTemplates(); i++)
        {
            post("reference %d: ", i);
            
            vector<vector<float> > tplt = x->bubi->getGestureTemplate(i).getTemplate();
            
            for(int j = 0; j < tplt.size(); j++)
            {
                post("%02.4f  %02.4f", tplt[j][0], tplt[j][1]);
            }
        }
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
        x->parameters=x->bubi->getParameters();
        x->parameters.tolerance=stdnew;
        x->bubi->setParameters(x->parameters);
}


///////////////////////////////////////////////////////////
//====================== resampling_threshold
///////////////////////////////////////////////////////////
static void gvf_resampling_threshold(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
        int rtnew = atom_getint(&argv[0]);
        int cNS = x->bubi->getNumberOfParticles();
        if (rtnew >= cNS)
            rtnew = floor(cNS/2);
        x->parameters=x->bubi->getParameters();
        x->parameters.resamplingThreshold  = rtnew;
        x->bubi->setParameters(x->parameters);
}


///////////////////////////////////////////////////////////
//====================== spreading_means
///////////////////////////////////////////////////////////
static void gvf_spreading_means(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
        // do something
}


///////////////////////////////////////////////////////////
//====================== spreading_ranges
///////////////////////////////////////////////////////////
static void gvf_spreading_ranges(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
        // do something
}


///////////////////////////////////////////////////////////
//====================== adaptation_speed
///////////////////////////////////////////////////////////
static void gvf_adaptation_speed(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    /*std::vector<float> as;
    as.push_back(getfloat(argv));
    as.push_back(getfloat(argv + 1));
    as.push_back(getfloat(argv + 2));
    as.push_back(getfloat(argv + 3));
    */
    // do something
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
        
        class_addmethod(gvf_class,(t_method)gvf_learn,gensym("learn"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_follow,gensym("follow"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_clear,gensym("clear"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_data,gensym("data"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_printme,gensym("printme"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_restart,gensym("restart"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_tolerance,gensym("tolerance"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_resampling_threshold,gensym("resampling_threshold"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_spreading_means,gensym("spreading_means"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_spreading_ranges,gensym("spreading_ranges"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_adaptation_speed,gensym("adaptation_speed"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_gestureOn,gensym("gestureOn"),A_GIMME,0);
        
        class_addmethod(gvf_class,(t_method)gvf_savetemplates,gensym("savetemplates"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_loadtemplates,gensym("loadtemplates"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_translate,gensym("translate"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_segmentation,gensym("segmentation"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_gestureOn,gensym("gestureOn"),A_GIMME,0);
        class_addmethod(gvf_class,(t_method)gvf_gestureOff,gensym("gestureOff"),A_GIMME,0);
    }
}

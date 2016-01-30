
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

#include "m_pd.h"
#include "GVF.h"
#include "GVFGesture.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

////////////////////////// object struct
typedef struct _gvf
{
    t_object            x_obj;
    // GVF related variables
    GVF              *bubi;
    GVFGesture       *currentGesture;
    GVFOutcomes      outcomes;
    // outlets
    t_outlet *Position,*Vitesse,*Scaling,*Rotation,*Likelihoods,*Info;
} t_gvf;



// MESSAGES THAT CAN BE RECEIVED FROM PURE DATA
// =============================================

//// BASICS
static void gvf_record          (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_start           (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_stop            (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_follow          (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_clear           (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_list            (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_printme         (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);

//// CONFIG
static void gvf_translate       (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_segmentation    (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);

//// PARAMETERS
static void gvf_tolerance           (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_particles           (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_resamplingthreshold (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);

static void gvf_dynamics            (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_scalings            (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_rotations           (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_anticipate          (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_spreadingdynamics   (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_spreadingscalings   (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_spreadingrotations  (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);

//// I/O
static void gvf_savetemplates       (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);
static void gvf_loadtemplates       (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv);


//static void gvf_auto(t_gvf *x);
static t_class *gvf_class;


// constructor of pd object
// ---------------------------------------------------------------------------
static void *gvf_new(t_symbol *s, int argc, t_atom *argv)
{
    post("gvf.beta - gesture variation follower (version: 0.3.0 [jan2016])");
    post("(c) B. Caramiaux, IRCAM and Goldsmiths, University of London");
    
    t_gvf *x = (t_gvf *)pd_new(gvf_class);
    if (x==NULL)
    {
        post("Error, couldn't create gvf object!");
    }
    else
    {
        x->bubi = new GVF();
        // current gesture
        x->currentGesture = new GVFGesture();
        // outlets
        x->Position     = outlet_new(&x->x_obj, &s_list);
        x->Vitesse      = outlet_new(&x->x_obj, &s_list);
        x->Scaling      = outlet_new(&x->x_obj, &s_list);
        x->Rotation     = outlet_new(&x->x_obj, &s_list);
        x->Likelihoods  = outlet_new(&x->x_obj, &s_list);
        x->Info         = outlet_new(&x->x_obj, &s_list);
    }
    return (void *)x;
}

// destructor of max object
// ---------------------------------------------------------------------------
static void gvf_destructor(t_gvf *x)
{
    if(x->bubi != NULL)
        delete x->bubi;
}



// "record"
// ---------------------------------------------------------------------------
static void gvf_record(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    
    if (argc==0)
    {
        x->bubi->setState(GVF::STATE_LEARNING);
    }
    else if (argc==1)
    {
        int gestureIndex = (int)atom_getfloat(&argv[0]);
        x->bubi->setState(GVF::STATE_LEARNING,{gestureIndex});
    }
    else {
        error("wrong number of argument in learn message");
    }
}

// "start"
// ---------------------------------------------------------------------------
void gvf_start(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    x->bubi->startGesture();
}

// "stop"
// ---------------------------------------------------------------------------
void gvf_stop(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    post("'stop' message does nothing, only start can be used");
}

// "follow"
// ---------------------------------------------------------------------------
static void gvf_follow(t_gvf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    x->bubi->setState(GVF::STATE_FOLLOWING);
    if (argc>0)     // arguments are gesture index to be activated
    {
        vector<int> activeGestures;
        for (int i = 0; i< argc; i++) activeGestures.push_back((int)atom_getfloat(&argv[i]));
        x->bubi->setActiveGestures(activeGestures);
    }
}

// data list
// ---------------------------------------------------------------------------
void gvf_list(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    
    if(argc == 0)
    {
        post("invalid format, points have at least 1 coordinate");
        return;
    }
    vector<float> observation(argc);
    for (int k=0; k<argc; k++)
        observation[k] = atom_getfloat(&argv[k]);
    switch (x->bubi->getState())
    {
        case GVF::STATE_LEARNING:
        {
            x->bubi->addObservation(observation);
            break;
        }
        case GVF::STATE_FOLLOWING:
        {
            if (x->bubi->getNumberOfGestureTemplates()>0)
            {
                x->outcomes = x->bubi->update(observation);
                int numberOfTemplates = x->bubi->getNumberOfGestureTemplates();
                
                t_atom *outAtoms = new t_atom[numberOfTemplates];
                for(int j = 0; j < numberOfTemplates; j++)
                    SETFLOAT(&outAtoms[j],x->outcomes.alignments[j]);
                outlet_list(x->Position, &s_list, numberOfTemplates, outAtoms);
                delete[] outAtoms;
                
                int dynamicsDimension = x->bubi->getDynamicsVariance().size();
                int scalingsDimension = x->bubi->getScalingsVariance().size();
                int rotationDimension = 0;
                
                outAtoms = new t_atom[numberOfTemplates * dynamicsDimension];
                for(int j = 0; j < numberOfTemplates; j++)
                    for(int jj = 0; jj < dynamicsDimension; jj++)
                        SETFLOAT(&outAtoms[j*dynamicsDimension+jj],x->outcomes.dynamics[j][jj]);
                outlet_list(x->Vitesse, &s_list, numberOfTemplates * dynamicsDimension, outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[numberOfTemplates * scalingsDimension];
                for(int j = 0; j < numberOfTemplates; j++)
                    for(int jj = 0; jj < scalingsDimension; jj++)
                        SETFLOAT(&outAtoms[j*scalingsDimension+jj],x->outcomes.scalings[j][jj]);
                outlet_list(x->Scaling, &s_list, numberOfTemplates * scalingsDimension, outAtoms);
                delete[] outAtoms;

                outAtoms = new t_atom[numberOfTemplates * rotationDimension];
                for(int j = 0; j < numberOfTemplates; j++)
                    for(int jj = 0; jj < rotationDimension; jj++)
                        SETFLOAT(&outAtoms[j*rotationDimension+jj],x->outcomes.rotations[j][jj]);
                outlet_list(x->Rotation, &s_list, numberOfTemplates * rotationDimension, outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[numberOfTemplates];
                for(int j = 0; j < numberOfTemplates; j++)
                    SETFLOAT(&outAtoms[j],x->outcomes.likelihoods[j]);
                outlet_list(x->Likelihoods, &s_list, numberOfTemplates, outAtoms);
                delete[] outAtoms;
            }
            break;
        }
        default:
            break;
    }
}


// "clear" msg
// ---------------------------------------------------------------------------
void gvf_clear(t_gvf *x, const t_symbol *sss, int argc, t_atom *argv)
{
    x->bubi->clear();
}

// "printme" msg
// ---------------------------------------------------------------------------
void gvf_printme(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    x->bubi->setState(GVF::STATE_FOLLOWING);
    post("======================");
    post("PARAMETERS");
    post("  Number of particles: %d", x->bubi->getNumberOfParticles());
    post("  Resampling Threshold: %d", x->bubi->getResamplingThreshold());
    post("  Tolerance: %.2f", x->bubi->getTolerance());
    post("  Adaptation parameters:");
    string scale_str = "      dynamics: ";
    for (int k=0; k<x->bubi->getDynamicsVariance().size(); k++) {
        std::ostringstream ss;
        ss << x->bubi->getDynamicsVariance()[k];
        scale_str = scale_str + ss.str() + " ";
    }
    post("  %s", scale_str.c_str());
    scale_str = "      scalings: ";
    for (int k=0; k<x->bubi->getScalingsVariance().size(); k++) {
        std::ostringstream ss;
        ss << x->bubi->getScalingsVariance()[k];
        scale_str = scale_str + ss.str() + " ";
    }
    post("  %s", scale_str.c_str());
    post("TEMPLATES");
    post("  Number of recorded templates: %d", x->bubi->getNumberOfGestureTemplates());
    if (x->bubi->getNumberOfGestureTemplates()>0)
        post("  Number of dimensions: %d", x->bubi->getGestureTemplate(0).getTemplateDimension());
    post("======================");
    for(int i = 0; i < x->bubi->getNumberOfGestureTemplates(); i++)
    {
        post("reference %d [%d] ", i+1, x->bubi->getGestureTemplate(i).getTemplateLength());
        vector<vector<float> > tplt = x->bubi->getGestureTemplate(i).getTemplate();
        for(int j = 0; j < 4; j++)
        {
            std::ostringstream ss;
            for(int k = 0; k < tplt[0].size(); k++)
                ss << tplt[j][k] << " ";
            post("%s", ss.str().c_str());
        }
        post("...");
        for(int j = tplt.size()-5; j < tplt.size(); j++)
        {
            std::ostringstream ss;
            for(int k = 0; k < tplt[0].size(); k++)
                ss << tplt[j][k] << " ";
            post("%s", ss.str().c_str());
        }
    }
    
    //    post("======================");
    //    post("Number of particles: %d", x->bubi->getNumberOfParticles());
    //    post("Resampling Threshold: %d", x->bubi->getResamplingThreshold());
    //    post("Tolerance: %.2f", x->bubi->getParameters().tolerance);
    //    post("Adaptation parameters:");
    //    post("  phase: %.7f", x->bubi->getParameters().phaseVariance);
    //    post("  speed: %.7f", x->bubi->getParameters().speedVariance);
    //    string scale_str = "scale: ";
    //    for (int k=0; k<x->bubi->getParameters().scaleVariance.size(); k++) {
    //        std::ostringstream ss;
    //        ss << x->bubi->getParameters().scaleVariance[k];
    //        scale_str = scale_str + ss.str() + " ";
    //    }
    //    post("  %s", scale_str.c_str());
    //    scale_str = "rotation: ";
    //    for (int k=0; k<x->bubi->getParameters().rotationVariance.size(); k++) {
    //        std::ostringstream ss;
    //        ss << x->bubi->getParameters().rotationVariance[k];
    //        scale_str = scale_str + ss.str() + " ";
    //    }
    //    post("  %s", scale_str.c_str());
    //    post("Number of recorded templates: %d", x->bubi->getNumberOfGestureTemplates());
    //    post("======================");
    //    for(int i = 0; i < x->bubi->getNumberOfGestureTemplates(); i++)
    //    {
    //        post("reference %d: ", i+1);
    //
    //        vector<vector<float> > tplt = x->bubi->getGestureTemplate(i).getTemplate();
    //
    //        for(int j = 0; j < tplt.size(); j++)
    //        {
    //            std::ostringstream ss;
    //            for(int k = 0; k < tplt[0].size(); k++){
    //                ss << tplt[j][k] << " ";
    //            }
    //            post("%s", ss.str().c_str());
    //        }
    //    }
}

// "tolerance" msg
// ---------------------------------------------------------------------------
static void gvf_tolerance(t_gvf *x, const t_symbol *sss, int argc, t_atom *argv)
{
    float tolerance = atom_getfloat(argv);
    if (tolerance <= 0.0)
        tolerance = 1.0;
    x->bubi->setTolerance(tolerance);
}


// "anticipate" msg
// ---------------------------------------------------------------------------
void gvf_anticipate(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    int predictionSteps = (int)atom_getfloat(argv);
    x->bubi->setPredictionSteps(predictionSteps);
}

// "resamplingthreshold" msg
// ---------------------------------------------------------------------------
void gvf_resamplingthreshold(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    int resamplingThreshold = (int)atom_getfloat(&argv[0]);
    int cNS = x->bubi->getNumberOfParticles();
    if (resamplingThreshold <= 0)
        resamplingThreshold = floor(cNS/2);
    x->bubi->setResamplingThreshold(resamplingThreshold);
}

// "particles" msg
// ---------------------------------------------------------------------------
void gvf_particles(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    int numberParticles = (int)atom_getfloat(&argv[0]);
    if (numberParticles<=0)
        numberParticles=1000;
    x->bubi->setNumberOfParticles(numberParticles);
}

// "dynamics" msg
// ---------------------------------------------------------------------------
void gvf_dynamics(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    int dynamicsDimension = x->bubi->getDynamicsVariance().size();
    if (argc == dynamicsDimension)
    {
        vector<float> variances(dynamicsDimension,0.0001);
        for (int k=0; k< argc; k++)
            variances[k] = sqrt(atom_getfloat(&argv[k]));
        x->bubi->setDynamicsVariance(variances);
    }
    if (argc == 1)
    {
        x->bubi->setDynamicsVariance(sqrt(atom_getfloat(argv)));
    }
}

// "scalings" msg
// ---------------------------------------------------------------------------
void gvf_scalings(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    int scalingsDimension = x->bubi->getScalingsVariance().size();
    if (argc == scalingsDimension)
    {
        vector<float> variances(scalingsDimension,0.0001);
        for (int k=0; k< argc; k++)
            variances[k] = sqrt(atom_getfloat(&argv[k]));
        x->bubi->setScalingsVariance(variances);
    }
    if (argc == 1)
    {
        x->bubi->setScalingsVariance(sqrt(atom_getfloat(argv)));
    }
}

// "rotations" msg
// ---------------------------------------------------------------------------
void gvf_rotations(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    int rotationDimension = x->bubi->getRotationsVariance().size();
    if (argc == rotationDimension)
    {
        vector<float> variances(rotationDimension,0.0001);
        for (int k=0; k< argc; k++)
            variances[k] = sqrt(atom_getfloat(&argv[k]));
        x->bubi->setRotationsVariance(variances);
    }
    if (argc == 1)
    {
        x->bubi->setRotationsVariance(sqrt(atom_getfloat(argv)));
    }
}

// "spreadingdynamics" msg
// ---------------------------------------------------------------------------
void gvf_spreadingdynamics (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv){
    if (argc!=2)
        return;
    x->bubi->setSpreadDynamics(atom_getfloat(&argv[0]),atom_getfloat(&argv[1]));
}

// "spreadingscalings" msg
// ---------------------------------------------------------------------------
void gvf_spreadingscalings (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv){
    if (argc!=2)
        return;
    x->bubi->setSpreadScalings(atom_getfloat(&argv[0]),atom_getfloat(&argv[1]));
}

// "spreadingrotations" msg
// ---------------------------------------------------------------------------
void gvf_spreadingrotations (t_gvf *x, const t_symbol *sss, int argc, t_atom *argv){
    if (argc!=2)
        return;
    x->bubi->setSpreadRotations(atom_getfloat(&argv[0]),atom_getfloat(&argv[1]));
}

// "translate" msg
// ---------------------------------------------------------------------------
void gvf_translate(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    bool translateFlag = ((int)atom_getfloat(&argv[0])==0)? false : true;
    x->bubi->translate(translateFlag);
}

// "segmentation" msg
// ---------------------------------------------------------------------------
void gvf_segmentation(t_gvf *x,const t_symbol *sss, int argc, t_atom *argv)
{
    bool segmentationFlag = ((int)atom_getfloat(&argv[0])==0)? false : true;
    x->bubi->segmentation(segmentationFlag);
}

// "export" msg
// ---------------------------------------------------------------------------
void gvf_export(t_gvf *x, const t_symbol *sss, int argc, t_atom *argv)
{
//    t_symbol* mpath = atom_getsym(argv);
//    string filename(mpath->s_name);
//    x->bubi->saveTemplates(filename);
}

// "import" msg
// ---------------------------------------------------------------------------
void gvf_import(t_gvf *x, const t_symbol *sss, int argc, t_atom *argv)
{
//    char* mpath = atom_string(argv);
//    int i=0;
//    while ( *(mpath+i)!='/' )
//        i++;
//    mpath = mpath+i;
//    string filename(mpath);
//    x->bubi->loadTemplates(filename);
}


extern "C"
{
    void gvf_setup(void)
    {
        gvf_class = class_new( gensym("gvf"),(t_newmethod)gvf_new,(t_method)gvf_destructor,
                              sizeof(t_gvf),CLASS_DEFAULT,A_GIMME,0);
        // basics
        class_addmethod(gvf_class, (t_method)gvf_record, gensym("record"),A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_start, gensym("start"),A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_stop, gensym("stop"),A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_follow, gensym("follow"),A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_clear, gensym("clear"),A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_list, gensym("list"),A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_printme, gensym("printme"),A_GIMME,0);
        // config
        class_addmethod(gvf_class, (t_method)gvf_translate, gensym("translate"),A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_segmentation, gensym("segmentation"),A_GIMME,0);
        // parameters
        class_addmethod(gvf_class, (t_method)gvf_tolerance,gensym("tolerance"), A_GIMME, 0);
        class_addmethod(gvf_class, (t_method)gvf_particles, gensym("particles"), A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_resamplingthreshold,gensym("resamplingthreshold"), A_GIMME, 0);
        class_addmethod(gvf_class, (t_method)gvf_dynamics, gensym("dynamics"), A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_scalings, gensym("scalings"), A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_rotations, gensym("rotations"), A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_anticipate, gensym("anticipate"), A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_spreadingdynamics, gensym("spreadingdynamics"), A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_spreadingscalings, gensym("spreadingscalings"), A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_spreadingrotations, gensym("spreadingrotations"), A_GIMME,0);
        // I/O
        class_addmethod(gvf_class, (t_method)gvf_export,gensym("export"),A_GIMME,0);
        class_addmethod(gvf_class, (t_method)gvf_import,gensym("import"),A_GIMME,0);
    }
}

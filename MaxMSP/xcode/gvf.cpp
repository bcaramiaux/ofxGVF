///////////////////////////////////////////////////////////////////////
//
//  GVF - Gesture Variation Follower Max/MSP Object
//
//
//  Copyright (C) 2014 Baptiste Caramiaux, Goldsmiths College, University of London
//
//  The GVF library is under the GNU Lesser General Public License (LGPL v3)
//  version: 11-2014
//
///////////////////////////////////////////////////////////////////////

#include "ext.h"
#include "ext_obex.h"
#include "GVF.h"
#include "GVFGesture.h"
#include <iostream>
#include <sstream>
#include <fstream>

////////////////////////// object struct
typedef struct _gvf
{
    t_object					ob;
    // GVF related variables
    GVF              *bubi;
    GVFOutcomes      outcomes;
    
    t_atom* out;
    
    // outlets
    //t_outlet *Position,*Vitesse,*Scaling,*Rotation,*Likelihoods;
    void *estimation_outlet;
    void *likelihoods_outlet;
    void *info_outlet;
    
} t_gvf;


static t_symbol *sym_getname = gensym("getname");



///////////////////////// function prototypes
//// NEW / FREE
void *gvf_new(t_symbol *s, long argc, t_atom *argv);
void gvf_free(t_gvf *x);

t_max_err getAttr(t_gvf *x, t_object *attr, long* ac, t_atom** av);
t_max_err setAttr(t_gvf *x, void *attr, long ac, t_atom *av);

//// BASICS
void gvf_record          (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_start           (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_stop            (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_follow          (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_clear           (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_list            (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_printme         (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);

//// CONFIG
void gvf_translate       (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_segmentation    (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);

//// PARAMETERS
void gvf_tolerance           (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_particles           (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_resamplingthreshold (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
//void gvf_alignment           (t_gvf *x,const t_symbol *sss, short argc, t_atom *argv);
void gvf_dynamics            (t_gvf *x,const t_symbol *sss, short argc, t_atom *argv);
void gvf_scalings            (t_gvf *x,const t_symbol *sss, short argc, t_atom *argv);
void gvf_rotations           (t_gvf *x,const t_symbol *sss, short argc, t_atom *argv);
void gvf_anticipate          (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_spreadingdynamics             (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_spreadingscalings             (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_spreadingrotations             (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);

//// I/O
void gvf_export   (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_import   (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);

//// DEPRECATED
void gvf_play          (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);



//////////////////////// global variables and co
void *gvf_class;


int C74_EXPORT main(void)
{
    
    // object initialization, NEW STYLE
    t_class *c;
    
    c = class_new("gvf", (method)gvf_new, (method)gvf_free, (long)sizeof(t_gvf),
                  0L /* leave NULL!! */, A_GIMME, 0);
    
    // basics
    class_addmethod(c, (method)gvf_record, "record", A_GIMME, 0);
    class_addmethod(c, (method)gvf_start, "start", A_GIMME, 0);
    class_addmethod(c, (method)gvf_stop, "stop", A_GIMME, 0);
    class_addmethod(c, (method)gvf_follow, "follow", A_GIMME, 0);
    class_addmethod(c, (method)gvf_play, "play", A_GIMME, 0);
    class_addmethod(c, (method)gvf_clear, "clear", A_GIMME, 0);
    class_addmethod(c, (method)gvf_list, "list", A_GIMME, 0);
    class_addmethod(c, (method)gvf_printme, "printme", A_GIMME, 0);
    
    // config
    class_addmethod(c, (method)gvf_translate, "translate", A_GIMME, 0);
    class_addmethod(c, (method)gvf_segmentation, "segmentation", A_GIMME, 0);
    
    // parameters
    class_addmethod(c, (method)gvf_tolerance, "tolerance", A_GIMME, 0);
    class_addmethod(c, (method)gvf_resamplingthreshold, "resamplingthreshold", A_GIMME, 0);
    class_addmethod(c, (method)gvf_particles, "particles", A_GIMME,0);
//    class_addmethod(c, (method)gvf_alignment, "alignment", A_GIMME,0);
    class_addmethod(c, (method)gvf_dynamics, "dynamics", A_GIMME,0);
    class_addmethod(c, (method)gvf_scalings, "scalings", A_GIMME,0);
    class_addmethod(c, (method)gvf_rotations, "rotations", A_GIMME,0);
    class_addmethod(c, (method)gvf_anticipate, "anticipate", A_GIMME,0);
    class_addmethod(c, (method)gvf_spreadingdynamics, "spreadingdynamics", A_GIMME,0);
    class_addmethod(c, (method)gvf_spreadingscalings, "spreadingscalings", A_GIMME,0);
    class_addmethod(c, (method)gvf_spreadingrotations, "spreadingrotations", A_GIMME,0);
    
    // I/O
    class_addmethod(c, (method)gvf_export, "export", A_GIMME,0);
    class_addmethod(c, (method)gvf_import, "import", A_GIMME,0);
    
    // DEPRECATED
    class_addmethod(c, (method)gvf_follow, "follow", A_GIMME, 0);
    
    
    
    CLASS_ATTR_FLOAT(c, "tolerance", 0, t_gvf, out);
    CLASS_ATTR_LABEL(c, "tolerance", 0, "tolerance");
    CLASS_ATTR_ACCESSORS(c, "tolerance", (method) getAttr, (method) setAttr);
    //    CLASS_ATTR_FILTER_CLIP(c, "tolerance", 0.0, );
    CLASS_ATTR_SAVE(c,"tolerance", 0.2);
    
    CLASS_ATTR_LONG(c, "particles", 0, t_gvf, out);
    CLASS_ATTR_LABEL(c, "particles", 0, "particles");
    CLASS_ATTR_ACCESSORS(c, "particles", (method) getAttr, (method) setAttr);
    CLASS_ATTR_FILTER_CLIP(c, "particles", 10, 100000);
    
    CLASS_ATTR_FLOAT(c, "dynamics", 0, t_gvf, out);
    CLASS_ATTR_LABEL(c, "dynamics", 0, "dynamics");
    CLASS_ATTR_ACCESSORS(c, "dynamics", (method) getAttr, (method) setAttr);
    CLASS_ATTR_FILTER_CLIP(c, "dynamics", 0.00000001, 0.1);
    //    CLASS_ATTR_SAVE(c,"dynamics", 0.0001);
    
    CLASS_ATTR_FLOAT(c, "scalings", 0, t_gvf, out);
    CLASS_ATTR_LABEL(c, "scalings", 0, "scalings");
    CLASS_ATTR_ACCESSORS(c, "scalings", (method) getAttr, (method) setAttr);
    CLASS_ATTR_FILTER_CLIP(c, "scalings", 0.00000001, 0.1);
    //    CLASS_ATTR_SAVE(c,"scalings", 0.0001);

    CLASS_ATTR_FLOAT(c, "rotations", 0, t_gvf, out);
    CLASS_ATTR_LABEL(c, "rotations", 0, "rotations");
    CLASS_ATTR_ACCESSORS(c, "rotations", (method) getAttr, (method) setAttr);
    CLASS_ATTR_FILTER_CLIP(c, "rotations", 0.00000001, 0.1);
    //    CLASS_ATTR_SAVE(c,"scalings", 0.0001);
    
    
    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    gvf_class = c;
    
    post("gvf.beta - gesture variation follower (version: 0.3.0 [jan2016])");
    post("(c) B. Caramiaux, IRCAM and Goldsmiths, University of London");
    
    return 0;
}

// constructor of max object
// ---------------------------------------------------------------------------
void *gvf_new(t_symbol *s, long argc, t_atom *argv)
{
    t_gvf *x = (t_gvf *)object_alloc((t_class *)gvf_class);
    if (x==NULL)
    {
        post("Error, couldn't create gvf object!");
    }
    else
    {
        x->bubi = new GVF();
        // outlets
        x->info_outlet           = outlet_new(x, NULL);
        x->likelihoods_outlet    = outlet_new(x, NULL);
        x->estimation_outlet     = outlet_new(x, NULL);
    }
    return (x);
}

// destructor of max object
// ---------------------------------------------------------------------------
void gvf_free(t_gvf *x)
{
    if(x->bubi != NULL)
        delete x->bubi;
}

// ---------------------------------------------------------------------------
t_max_err getAttr(t_gvf *x, t_object *attr, long* ac, t_atom** av)
{
    if ((*ac) == 0 || (*av) == NULL)
    {
        *ac = 1;
        if (!(*av = (t_atom *)getbytes(sizeof(t_atom) * (*ac))))
        {
            *ac = 0;
            return MAX_ERR_OUT_OF_MEM;
        }
    }
//    const GVFParameters parameters = x->bubi->getParameters();
    string attrname = ((t_symbol *)object_method((t_object *)attr, sym_getname))->s_name;
    
    if (attrname.compare("tolerance") == 0)
    {
        atom_setfloat((av[0]), x->bubi->getTolerance());
    }
    else if (attrname.compare("particles") == 0)
    {
        atom_setlong((av[0]), x->bubi->getNumberOfParticles());
    }
    else if (attrname.compare("dynamics") == 0)
    {
        atom_setfloat((av[0]), x->bubi->getDynamicsVariance()[0]);
    }
    else if (attrname.compare("scalings") == 0)
    {
        atom_setfloat((av[0]), x->bubi->getScalingsVariance()[0]);
    }
    else if (attrname.compare("rotations") == 0)
    {
        atom_setfloat((av[0]), x->bubi->getRotationsVariance()[0]);
    }
    return MAX_ERR_NONE;
    
}

// ---------------------------------------------------------------------------
t_max_err setAttr(t_gvf *x, void *attr, long ac, t_atom *av)
{
    
    t_symbol *attrsym = (t_symbol *)object_method((t_object *)attr, sym_getname);
    string attrname = attrsym->s_name;
    
    if (attrname.compare("tolerance") == 0)
    {
        x->bubi->setTolerance(atom_getfloat(&av[0]));
    }
    else if (attrname.compare("particles") == 0)
    {
        x->bubi->setNumberOfParticles(atom_getlong(&av[0]));
    }
    else if (attrname.compare("dynamics") == 0)
    {
        x->bubi->setDynamicsVariance(atom_getfloat(&av[0]));
    }
    else if (attrname.compare("scalings") == 0)
    {
        x->bubi->setScalingsVariance(atom_getfloat(&av[0]));
    }
    else if (attrname.compare("rotations") == 0)
    {
        x->bubi->setRotationsVariance(atom_getfloat(&av[0]));
    }
    return MAX_ERR_NONE;
    
}

// "record"
// ---------------------------------------------------------------------------
void gvf_record(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    if (argc==0)
    {
        x->bubi->setState(GVF::STATE_LEARNING);
        t_atom *outAtoms = new t_atom[1];
        atom_setlong(&outAtoms[0], x->bubi->getNumberOfGestureTemplates()+1);
        outlet_anything(x->info_outlet, gensym("learningGesture"), 1, outAtoms);
        delete[] outAtoms;
    }
    else if (argc==1)
    {
        int gestureIndex = atom_getlong(&argv[0]);
        x->bubi->setState(GVF::STATE_LEARNING,{gestureIndex});
        //        post("replacing gesture %i", atom_getlong(&argv[0]));
        //        x->currentGestureID = atom_getlong(&argv[0]);
        //        manuallyAssignedGesture = true;
    }
    else {
        error("wrong number of argument in learn message");
    }
    
}

// "start"
// ---------------------------------------------------------------------------
void gvf_start(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    x->bubi->startGesture();
}


// "stop" msg
// ---------------------------------------------------------------------------
void gvf_stop(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    error("'stop' message does nothing, only start can be used");
}

// "follow" msg
// ---------------------------------------------------------------------------
void gvf_play(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    error("'play' is deprecated, use follow instead");
    gvf_follow(x, sss, argc, argv);
    //    if (x->bubi->getNumberOfGestureTemplates()==0)
    //    {
    //        x->bubi->setState(GVF::STATE_CLEAR);
    //    }
    //    else{
    //        x->bubi->setState(GVF::STATE_FOLLOWING);
    //        // output the current ID of the gesture being learned with the prefix "learningGesture"
    //        t_atom *outAtoms = new t_atom[1];
    //        atom_setfloat(&outAtoms[0],x->bubi->getParameters().tolerance);
    //        outlet_anything(x->info_outlet, gensym("tolerance"), 1, outAtoms);
    //        delete[] outAtoms;
    //

}

// "follow" msg
// ---------------------------------------------------------------------------
void gvf_follow(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    
    x->bubi->setState(GVF::STATE_FOLLOWING);
    
    if (argc>0)     // arguments are gesture index to be activated
    {
        vector<int> activeGestures;
        for (int i = 0; i< argc; i++) activeGestures.push_back(atom_getlong(&argv[i]));
        x->bubi->setActiveGestures(activeGestures);
    }
    
    //    t_atom *outAtoms = new t_atom[1];
    //    atom_setfloat(&outAtoms[0],x->bubi->getParameters().tolerance);
    //    outlet_anything(x->info_outlet, gensym("tolerance"), 1, outAtoms);
    //    delete[] outAtoms;
    //
    //    outAtoms = new t_atom[1];
    //    atom_setfloat(&outAtoms[0],x->bubi->getConfig().inputDimensions);
    //    outlet_anything(x->info_outlet, gensym("dimensions"), 1, outAtoms);
    //    delete[] outAtoms;
    if (x->bubi->getNumberOfGestureTemplates()>0){
        t_atom *outAtoms = new t_atom[1];
        atom_setfloat(&outAtoms[0],x->bubi->getGestureTemplate(0).getNumberDimensions());
        outlet_anything(x->info_outlet, gensym("dimensions"), 1, outAtoms);
        delete[] outAtoms;
    }
    
}

// data list
// ---------------------------------------------------------------------------
void gvf_list(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
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
                // inference on the last observation
                x->outcomes = x->bubi->update(observation);

                int numberOfTemplates = x->bubi->getNumberOfGestureTemplates();
                
                t_atom *outAtoms = new t_atom[numberOfTemplates];
                
                for(int j = 0; j < numberOfTemplates; j++)
                    atom_setfloat(&outAtoms[j],x->outcomes.alignments[j]);
                outlet_anything(x->estimation_outlet, gensym("alignment"), numberOfTemplates, outAtoms);
                delete[] outAtoms;
                
                int dynamicsDimension = x->bubi->getDynamicsVariance().size();
                int scalingsDimension = x->bubi->getScalingsVariance().size();
                int rotationDimension = x->bubi->getRotationsVariance().size();
                
                outAtoms = new t_atom[numberOfTemplates * dynamicsDimension];
                for(int j = 0; j < numberOfTemplates; j++)
                    for(int jj = 0; jj < dynamicsDimension; jj++)
                        atom_setfloat(&outAtoms[j*dynamicsDimension+jj],x->outcomes.dynamics[j][jj]);
                outlet_anything(x->estimation_outlet, gensym("dynamics"), numberOfTemplates * dynamicsDimension, outAtoms);
                delete[] outAtoms;
                
                outAtoms = new t_atom[numberOfTemplates * scalingsDimension];
                for(int j = 0; j < numberOfTemplates; j++)
                    for(int jj = 0; jj < scalingsDimension; jj++)
                        atom_setfloat(&outAtoms[j*scalingsDimension+jj],x->outcomes.scalings[j][jj]);
                outlet_anything(x->estimation_outlet, gensym("scalings"), numberOfTemplates * scalingsDimension, outAtoms);
                delete[] outAtoms;
                
                if (rotationDimension!=0)
                {
                    outAtoms = new t_atom[numberOfTemplates * rotationDimension];
                    for(int j = 0; j < numberOfTemplates; j++)
                        for(int jj = 0; jj < rotationDimension; jj++)
                            atom_setfloat(&outAtoms[j*rotationDimension+jj],x->outcomes.rotations[j][jj]);
                    outlet_anything(x->estimation_outlet, gensym("rotations"), numberOfTemplates * rotationDimension, outAtoms);
                    delete[] outAtoms;
                }
                
                
                outAtoms = new t_atom[numberOfTemplates];
                for(int j = 0; j < numberOfTemplates; j++)
                    atom_setfloat(&outAtoms[j],x->outcomes.likelihoods[j]);
                outlet_anything(x->likelihoods_outlet, gensym("likelihoods"), numberOfTemplates, outAtoms);
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
void gvf_clear(t_gvf *x, const t_symbol *sss, short argc, t_atom *argv)
{
    x->bubi->clear();
}

// "printme" msg
// ---------------------------------------------------------------------------
void gvf_printme(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    x->bubi->setState(GVF::STATE_FOLLOWING);
    post("======================");
    //    post("CONFIGURATION");
    //    post("  Segmentation: %d", x->bubi->getConfig().segmentation);
    //    post("  Normalization: %d", x->bubi->getConfig().normalization);
    //    if (x->bubi->getConfig().normalization)
    //        post("  Global normalization factor: %f", x->bubi->getGlobalNormalizationFactor());
    post("PARAMETERS");
    post("  Number of particles: %d", x->bubi->getNumberOfParticles());
    post("  Resampling Threshold: %d", x->bubi->getResamplingThreshold());
    post("  Tolerance: %.2f", x->bubi->getTolerance());
    post("  Adaptation parameters:");
//    post("      phase: %.7f", x->bubi->getAlignmentVariance());
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
//    scale_str = "  Weights: ";
//    for (int k=0; k<x->bubi->getWeights().size(); k++) {
//        std::ostringstream ss;
//        ss << x->bubi->getWeights()[k];
//        scale_str = scale_str + ss.str() + " ";
//    }
//    post("  %s", scale_str.c_str());
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
//        for(int j = 0; j < tplt.size(); j++)
//        {
//            std::ostringstream ss;
//            for(int k = 0; k < tplt[0].size(); k++)
//                ss << tplt[j][k] << " ";
//            post("%s", ss.str().c_str());
//        }
    }
}

// "tolerance" msg
// ---------------------------------------------------------------------------
void gvf_tolerance(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    float tolerance = atom_getfloat(argv);
    if (tolerance <= 0.0)
        tolerance = 1.0;
    x->bubi->setTolerance(tolerance);
}

// "anticipate" msg
// ---------------------------------------------------------------------------
void gvf_anticipate(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    int predictionSteps = atom_getlong(argv);
    x->bubi->setPredictionSteps(predictionSteps);
}

// "resamplingthreshold" msg
// ---------------------------------------------------------------------------
void gvf_resamplingthreshold(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    int resamplingThreshold = atom_getlong(&argv[0]);
    int cNS = x->bubi->getNumberOfParticles();
    if (resamplingThreshold <= 0)
        resamplingThreshold = floor(cNS/2);
    x->bubi->setResamplingThreshold(resamplingThreshold);
}

// "particles" msg
// ---------------------------------------------------------------------------
void gvf_particles(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    int numberParticles = atom_getlong(&argv[0]);
    if (numberParticles<=0)
        numberParticles=1000;
    x->bubi->setNumberOfParticles(numberParticles);
}

//// "alignment" msg
//// ---------------------------------------------------------------------------
//void gvf_alignment(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
//{
//    x->bubi->setAlignmentVariance(sqrt(atom_getfloat(argv)));
//}

// "dynamics" msg
// ---------------------------------------------------------------------------
void gvf_dynamics(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
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
void gvf_scalings(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
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
void gvf_rotations(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
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
void gvf_spreadingdynamics (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv){
    if (argc!=2)
        return;
    x->bubi->setSpreadDynamics(atom_getfloat(&argv[0]),atom_getfloat(&argv[1]));
}

// "spreadingscalings" msg
// ---------------------------------------------------------------------------
void gvf_spreadingscalings (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv){
    if (argc!=2)
        return;
    x->bubi->setSpreadScalings(atom_getfloat(&argv[0]),atom_getfloat(&argv[1]));
}

// "spreadingrotations" msg
// ---------------------------------------------------------------------------
void gvf_spreadingrotations (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv){
    if (argc!=2)
        return;
    x->bubi->setSpreadRotations(atom_getfloat(&argv[0]),atom_getfloat(&argv[1]));
}

// "translate" msg
// ---------------------------------------------------------------------------
void gvf_translate(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    bool translateFlag = (atom_getlong(&argv[0])==0)? false : true;
    x->bubi->translate(translateFlag);
}

// "segmentation" msg
// ---------------------------------------------------------------------------
void gvf_segmentation(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    bool segmentationFlag = (atom_getlong(&argv[0])==0)? false : true;
    x->bubi->segmentation(segmentationFlag);
}

// "export" msg
// ---------------------------------------------------------------------------
void gvf_export(t_gvf *x, const t_symbol *sss, short argc, t_atom *argv)
{
    t_symbol* mpath = atom_getsym(argv);
    string filename(mpath->s_name);
    x->bubi->saveTemplates(filename);
}

// "import" msg
// ---------------------------------------------------------------------------
void gvf_import(t_gvf *x, const t_symbol *sss, short argc, t_atom *argv)
{
    
    char* mpath = atom_string(argv);
    int i=0;
    while ( *(mpath+i)!='/' )
        i++;
    mpath = mpath+i;
    string filename(mpath);
    x->bubi->loadTemplates(filename);
    
}


//void logGVF(t_gvf *x)
//{
//    idfile++;
//
//    for(int j = 0; j < x->bubi->getNumberOfGestureTemplates(); j++){
//        ostringstream convert; convert << j;
//        string filename = "/Users/caramiaux/Research/Code/MaxSDK-6.1.4/examples/maxprojects/gvf-develop/maxexternal/datatests/gvflog-template-"+convert.str()+".txt";
//
//        GVFGesture temp = x->bubi->getGestureTemplate(j);
//        vector<vector<float> > observations = temp.getTemplateRaw();
//        std::ofstream file_writeObs(filename.c_str());
//        for(int j = 0; j < observations.size(); j++){
//            for(int k = 0; k < observations[j].size(); k++)
//                file_writeObs << observations[j][k] << " ";
//            file_writeObs << endl;
//        }
//        file_writeObs.close();
//    }
//
//    string filename = "/Users/caramiaux/Research/Code/MaxSDK-6.1.4/examples/maxprojects/gvf-develop/maxexternal/datatests/gvflog-observations.txt";
//    vector<vector<float> > observations = x->currentGesture->getTemplateRaw();
//    std::ofstream file_writeObs(filename.c_str());
//
//    for(int j = 0; j < observations.size(); j++){
//        for(int k = 0; k < observations[j].size(); k++)
//            file_writeObs << observations[j][k] << " ";
//        file_writeObs << endl;
//    }
//    file_writeObs.close();
//
//    vector<float> align             = x->bubi->getAlignment();
//    vector<int> classes             = x->bubi->getClasses();
//    vector<vector<float> > dynas    = x->bubi->getDynamics();
//    vector<vector<float> > scals    = x->bubi->getScalings();
//    vector<float> prior             = x->bubi->getPrior();
//    vector<vector<float> > vref     = x->bubi->getVecRef();
//    vector<float> vobs              = x->bubi->getVecObs();
//
//
//    ostringstream convert; convert << idfile;
//    std::string directory = "/Users/caramiaux/Research/Code/MaxSDK-6.1.4/examples/maxprojects/gvf-develop/maxexternal/datatests/gvflog-particles-"+convert.str()+".txt";
//    std::ofstream file_write(directory.c_str());
//
//    for(int j = 0; j < dynas.size(); j++)
//    {
//        file_write << align[j] << " ";
//        for(int k = 0; k < dynas[j].size(); k++)
//            file_write << dynas[j][k] << " ";
//        for(int k = 0; k < scals[j].size(); k++)
//            file_write << scals[j][k] << " ";
//        file_write << prior[j] << " ";
//        file_write << classes[j] << " ";
//        for(int k = 0; k < vref[j].size(); k++)
//            file_write << vref[j][k] << " ";
//        for(int k = 0; k < vobs.size(); k++)
//            file_write << vobs[k] << " ";
//        file_write << endl;
//    }
//    file_write.close();
//
//    vector<float> estAlign = x->bubi->getEstimatedAlignment();
//    vector< vector<float> > estDynas = x->bubi->getEstimatedDynamics();
//    vector< vector<float> > estScals = x->bubi->getEstimatedScalings();
//
//    directory = "/Users/caramiaux/Research/Code/MaxSDK-6.1.4/examples/maxprojects/gvf-develop/maxexternal/datatests/gvflog-estimations-"+convert.str()+".txt";
//    std::ofstream file_write2(directory.c_str());
//
//    for(int j = 0; j < estAlign.size(); j++)
//    {
//        file_write2 << estAlign[j] << " ";
//        for(int k = 0; k < estDynas[j].size(); k++)
//            file_write2 << estDynas[j][k] << " ";
//        for(int k = 0; k < estScals[j].size(); k++)
//            file_write2 << estScals[j][k] << " ";
//        file_write2 << endl;
//    }
//    file_write2.close();
//
//
//}


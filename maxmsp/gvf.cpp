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





#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object
#include "ofxGVF.h"
#include "ofxGVFGesture.h"
#include <iostream>
#include <sstream>
#include <fstream>





////////////////////////// object struct
typedef struct _gvf
{
	t_object					ob;			// the object itself (must be first)
    
    // GVF related variables
    ofxGVF              *bubi;
	ofxGVFGesture       *currentGesture;
    ofxGVFConfig        config;
    ofxGVFParameters    parameters;
    ofxGVFOutcomes      outcomes;
    
    // outlets
    //t_outlet *Position,*Vitesse,*Scaling,*Rotation,*Likelihoods;
    void *estimation_outlet;
    void *likelihoods_outlet;
    void *info_outlet;
    
} t_gvf;


///////////////////////// function prototypes
//// NEW / FREE
void *gvf_new(t_symbol *s, long argc, t_atom *argv);
void gvf_free(t_gvf *x);

//// BASICS
void gvf_record           (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_start           (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_stop            (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_play          (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_clear           (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_list            (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_printme         (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_restart         (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);

//// CONFIG
void gvf_translate       (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_segmentation    (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_normalize       (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);

//// PARAMETERS
void gvf_tolerance           (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_studentnu           (t_gvf *x,const t_symbol *sss, short argc, t_atom *argv);
void gvf_numberparticles     (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_resamplingthreshold (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_alignment           (t_gvf *x,const t_symbol *sss, short argc, t_atom *argv);
void gvf_dynamics            (t_gvf *x,const t_symbol *sss, short argc, t_atom *argv);
void gvf_scalings            (t_gvf *x,const t_symbol *sss, short argc, t_atom *argv);
void gvf_anticipate           (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);

//// I/O
void gvf_savetemplates   (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_loadtemplates   (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_log         (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);

//// DEPRECATED
void gvf_gestureOn       (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_gestureOff      (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);
void gvf_adaptation_speed (t_gvf *x, const t_symbol *sss, short argc, t_atom *argv);



int idfile = 0;
void logGVF(t_gvf *x);


//////////////////////// global class pointer variable
void *gvf_class;


int C74_EXPORT main(void)
{
	
	// object initialization, NEW STYLE
	t_class *c;
	
	c = class_new("gvf", (method)gvf_new, (method)gvf_free, (long)sizeof(t_gvf),
				  0L /* leave NULL!! */, A_GIMME, 0);
	
    //  MESSAGES

    // basics
    class_addmethod(c, (method)gvf_record, "record", A_GIMME, 0);
    class_addmethod(c, (method)gvf_start, "start", A_GIMME, 0);
    class_addmethod(c, (method)gvf_stop, "stop", A_GIMME, 0);
    class_addmethod(c, (method)gvf_play, "play", A_GIMME, 0);
    class_addmethod(c, (method)gvf_clear, "clear", A_GIMME, 0);
    class_addmethod(c, (method)gvf_list, "list", A_GIMME, 0);
    class_addmethod(c, (method)gvf_printme, "printme", A_GIMME, 0);
    class_addmethod(c, (method)gvf_restart, "restart", A_GIMME, 0);
    
    // config
    class_addmethod(c, (method)gvf_translate, "translate", A_GIMME, 0);
    class_addmethod(c, (method)gvf_segmentation, "segmentation", A_GIMME, 0);
    class_addmethod(c, (method)gvf_normalize, "normalize", A_GIMME, 0);
    
    // parameters
    class_addmethod(c, (method)gvf_tolerance, "tolerance", A_GIMME, 0);
    class_addmethod(c, (method)gvf_studentnu, "studentnu", A_GIMME, 0);
    class_addmethod(c, (method)gvf_resamplingthreshold, "resamplingthreshold", A_GIMME, 0);
    class_addmethod(c, (method)gvf_numberparticles, "numberparticles", A_GIMME,0);
    class_addmethod(c, (method)gvf_alignment, "alignment", A_GIMME,0);
    class_addmethod(c, (method)gvf_dynamics, "dynamics", A_GIMME,0);
    class_addmethod(c, (method)gvf_scalings, "scalings", A_GIMME,0);
    class_addmethod(c, (method)gvf_anticipate, "anticipate", A_GIMME,0);
    
    
    // I/O
    class_addmethod(c, (method)gvf_savetemplates, "savetemplates", A_GIMME,0);
    class_addmethod(c, (method)gvf_loadtemplates, "loadtemplates", A_GIMME,0);
    class_addmethod(c, (method)gvf_log, "log", A_GIMME,0);
    
	
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	gvf_class = c;

    post("gvf.beta - gesture variation follower (version: 0.2.1 [feb2015])");
    post("(c) Goldsmiths, University of London and IRCAM - Centre Pompidou");
    
	return 0;
}


///////////////////////////////////////////////////////////
//====================== FREE GVF
///////////////////////////////////////////////////////////
void gvf_free(t_gvf *x)
{
    if(x->bubi != NULL)
        delete x->bubi;
}

///////////////////////////////////////////////////////////
//====================== NEW GVF
///////////////////////////////////////////////////////////
void *gvf_new(t_symbol *s, long argc, t_atom *argv)
{
	t_gvf *x = NULL;
    
    //x = (t_gvf *)newobject(gvf_class);
    x = (t_gvf *)object_alloc((t_class *)gvf_class);

    
    if (x==NULL){
        post("Error, gvf object NULL (see code)");
    }
    else{
    
        // CONFIGURATION of the GVF
        x->config.inputDimensions  = 2;
        x->config.translate        = true;
        x->config.segmentation     = false;
        x->config.logOn            = false;
        
        // PARAMETERS are set by default
        
        // CREATE the corresponding GVF
        x->bubi = new ofxGVF(x->config);
        
        // CREATE THE GESTURE
        x->currentGesture = new ofxGVFGesture();

        x->info_outlet           = outlet_new(x, NULL);
        x->likelihoods_outlet    = outlet_new(x, NULL);
        x->estimation_outlet     = outlet_new(x, NULL);

    }

	return (x);
}




///////////////////////////////////////////////////////////
//====================== LEARN
///////////////////////////////////////////////////////////
void gvf_record(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    x->bubi->setState(ofxGVF::STATE_LEARNING);
    if (argc==0){
    
        // output the current ID of the gesture being learned with the prefix "learningGesture"
        t_atom *outAtoms = new t_atom[1];
        atom_setlong(&outAtoms[0],x->bubi->getNumberOfGestureTemplates()+1);
        outlet_anything(x->info_outlet, gensym("learningGesture"), 1, outAtoms);
        delete[] outAtoms;
        
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
void gvf_start(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
//    if (x->bubi->getState()==ofxGVF::STATE_LEARNING)
//        x->currentGesture->clear();

        x->currentGesture->clear();
        idfile = 0;
        
        if(x->bubi->getState() == ofxGVF::STATE_FOLLOWING)
            x->bubi->restart();

}


///////////////////////////////////////////////////////////
//====================== STOP
///////////////////////////////////////////////////////////
void gvf_stop(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    if (x->bubi->getState()==ofxGVF::STATE_LEARNING && (x->currentGesture->getTemplateLength()>0))
            x->bubi->addGestureTemplate(*(x->currentGesture));
    
}


///////////////////////////////////////////////////////////
//====================== FOLLOW
///////////////////////////////////////////////////////////
void gvf_play(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    x->bubi->setState(ofxGVF::STATE_FOLLOWING);
    
    // output the current ID of the gesture being learned with the prefix "learningGesture"
    t_atom *outAtoms = new t_atom[1];
    atom_setfloat(&outAtoms[0],x->bubi->getParameters().tolerance);
    outlet_anything(x->info_outlet, gensym("tolerance"), 1, outAtoms);
    delete[] outAtoms;
}


void gvf_gestureOn(t_gvf *x, const t_symbol *sss, short argc, t_atom *argv)
{
     error("gestureOn message deprecated: do nothing");
}


void gvf_gestureOff(t_gvf *x, const t_symbol *sss, short argc, t_atom *argv)
{
     error("gestureOff message deprecated: do nothing");
}

///////////////////////////////////////////////////////////
//====================== LIST
///////////////////////////////////////////////////////////
void gvf_list(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    
    if(argc == 0)
    {
        post("invalid format, points have at least 1 coordinate");
        return;
    }
    if(x->bubi->getState() == ofxGVF::STATE_CLEAR)
    {
        post("I'm in a standby (must record something beforehand)");
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
            
            // log just before inference
            if (x->bubi->getConfig().logOn)
                logGVF(x);
            
            // inference on the last observation
            x->bubi->update(x->currentGesture->getLastObservation());
        
            
            // output recognition
            x->outcomes = x->bubi->getOutcomes();
            int numberOfTemplates = x->bubi->getNumberOfGestureTemplates();
            
            t_atom *outAtoms = new t_atom[numberOfTemplates];
            
            for(int j = 0; j < numberOfTemplates; j++)
                atom_setfloat(&outAtoms[j],x->outcomes.estimations[j].alignment);
            outlet_anything(x->estimation_outlet, gensym("alignment"), numberOfTemplates, outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[numberOfTemplates * x->bubi->getDynamicsDim()];
            for(int j = 0; j < numberOfTemplates; j++)
                for(int jj = 0; jj < x->bubi->getDynamicsDim(); jj++)
                    atom_setfloat(&outAtoms[j*x->bubi->getDynamicsDim()+jj],x->outcomes.estimations[j].dynamics[jj]);
            outlet_anything(x->estimation_outlet, gensym("dynamics"), numberOfTemplates * x->bubi->getDynamicsDim(), outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[numberOfTemplates * x->bubi->getScalingsDim()];
            for(int j = 0; j < numberOfTemplates; j++)
                for(int jj = 0; jj < x->bubi->getScalingsDim(); jj++)
                    atom_setfloat(&outAtoms[j*x->bubi->getScalingsDim()+jj],x->outcomes.estimations[j].scalings[jj]);
            outlet_anything(x->estimation_outlet, gensym("scalings"), numberOfTemplates * x->bubi->getScalingsDim(), outAtoms);
            delete[] outAtoms;

            
            outAtoms = new t_atom[numberOfTemplates];
            for(int j = 0; j < numberOfTemplates; j++)
                atom_setfloat(&outAtoms[j],x->outcomes.estimations[j].probability);
            outlet_anything(x->likelihoods_outlet, gensym("likelihoods"), numberOfTemplates, outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[1];
            float sumLikelihoods = 0.0f;
            for(int k=0; k<x->bubi->getNumberOfGestureTemplates(); k++) sumLikelihoods += x->outcomes.estimations[k].likelihood;
            atom_setfloat(&outAtoms[0],sumLikelihoods/x->bubi->getNumberOfParticles());
            outlet_anything(x->likelihoods_outlet, gensym("accuracy"), 1, outAtoms);
            delete[] outAtoms;


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
void gvf_clear(t_gvf *x, const t_symbol *sss, short argc, t_atom *argv)
{
    x->bubi->clear();

    // output 0 for the number of learned gesture
    t_atom *outAtoms = new t_atom[1];
    atom_setlong(&outAtoms[0],0);
    outlet_anything(x->info_outlet, gensym("learningGesture"), 1, outAtoms);
    delete[] outAtoms;
}


///////////////////////////////////////////////////////////
//====================== PRINTME
///////////////////////////////////////////////////////////
void gvf_printme(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    post("======================");
    post("CONFIGURATION");
    post("  Segmentation: %d", x->bubi->getConfig().segmentation);
    post("  Normalization: %d", x->bubi->getConfig().normalization);
    if (x->bubi->getConfig().normalization)
        post("  Global normalization factor: %f", x->bubi->getGlobalNormalizationFactor());
    post("PARAMETERS");
    post("  Number of particles: %d", x->bubi->getNumberOfParticles());
    post("  Resampling Threshold: %d", x->bubi->getResamplingThreshold());
    post("  Tolerance: %.2f", x->bubi->getParameters().tolerance);
    post("  Adaptation parameters:");
    post("      phase: %.7f", x->bubi->getParameters().alignmentVariance);
    string scale_str = "      dynamics: ";
    for (int k=0; k<x->bubi->getParameters().dynamicsVariance.size(); k++) {
        std::ostringstream ss;
        ss << x->bubi->getParameters().dynamicsVariance[k];
        scale_str = scale_str + ss.str() + " ";
    }
    post("  %s", scale_str.c_str());
    scale_str = "      scalings: ";
    for (int k=0; k<x->bubi->getParameters().scalingsVariance.size(); k++) {
        std::ostringstream ss;
        ss << x->bubi->getParameters().scalingsVariance[k];
        scale_str = scale_str + ss.str() + " ";
    }
    post("  %s", scale_str.c_str());
    post("TEMPLATES");
    post("  Number of recorded templates: %d", x->bubi->getNumberOfGestureTemplates());
    post("  Number of dimensions: %d", x->bubi->getConfig().inputDimensions);
    post("======================");
    for(int i = 0; i < x->bubi->getNumberOfGestureTemplates(); i++)
    {
        post("reference %d: ", i+1);
        
        vector<vector<float> > tplt = x->bubi->getGestureTemplate(i).getTemplate();
        
        for(int j = 0; j < tplt.size(); j++)
        {
            std::ostringstream ss;
            for(int k = 0; k < tplt[0].size(); k++){
                if (x->bubi->getConfig().normalization)
                    ss << tplt[j][k]/x->bubi->getGlobalNormalizationFactor() << " ";
                else
                    ss << tplt[j][k] << " ";
            }
            post("%s", ss.str().c_str());
        }
    }
}




void logGVF(t_gvf *x)
{
    idfile++;
    
    for(int j = 0; j < x->bubi->getNumberOfGestureTemplates(); j++){
        ostringstream convert; convert << j;
        string filename = "/Users/caramiaux/Research/Code/MaxSDK-6.1.4/examples/maxprojects/gvf-develop/maxexternal/datatests/gvflog-template-"+convert.str()+".txt";
        
        ofxGVFGesture temp = x->bubi->getGestureTemplate(j);
        vector<vector<float> > observations = temp.getTemplateRaw();
        std::ofstream file_writeObs(filename.c_str());
        for(int j = 0; j < observations.size(); j++){
            for(int k = 0; k < observations[j].size(); k++)
                file_writeObs << observations[j][k] << " ";
            file_writeObs << endl;
        }
        file_writeObs.close();
    }
    
    string filename = "/Users/caramiaux/Research/Code/MaxSDK-6.1.4/examples/maxprojects/gvf-develop/maxexternal/datatests/gvflog-observations.txt";
    vector<vector<float> > observations = x->currentGesture->getTemplateRaw();
    std::ofstream file_writeObs(filename.c_str());
    
    for(int j = 0; j < observations.size(); j++){
        for(int k = 0; k < observations[j].size(); k++)
            file_writeObs << observations[j][k] << " ";
        file_writeObs << endl;
    }
    file_writeObs.close();
    
    vector<float> align             = x->bubi->getAlignment();
    vector<int> classes             = x->bubi->getClasses();
    vector<vector<float> > dynas    = x->bubi->getDynamics();
    vector<vector<float> > scals    = x->bubi->getScalings();
    vector<float> prior             = x->bubi->getPrior();
    vector<vector<float> > vref     = x->bubi->getVecRef();
    vector<float> vobs              = x->bubi->getVecObs();
    
    
    ostringstream convert; convert << idfile;
    std::string directory = "/Users/caramiaux/Research/Code/MaxSDK-6.1.4/examples/maxprojects/gvf-develop/maxexternal/datatests/gvflog-particles-"+convert.str()+".txt";
    std::ofstream file_write(directory.c_str());
    
    for(int j = 0; j < dynas.size(); j++)
    {
        file_write << align[j] << " ";
        for(int k = 0; k < dynas[j].size(); k++)
            file_write << dynas[j][k] << " ";
        for(int k = 0; k < scals[j].size(); k++)
            file_write << scals[j][k] << " ";
        file_write << prior[j] << " ";
        file_write << classes[j] << " ";
        for(int k = 0; k < vref[j].size(); k++)
            file_write << vref[j][k] << " ";
        for(int k = 0; k < vobs.size(); k++)
            file_write << vobs[k] << " ";
        file_write << endl;
    }
    file_write.close();

    vector<float> estAlign = x->bubi->getEstimatedAlignment();
    vector< vector<float> > estDynas = x->bubi->getEstimatedDynamics();
    vector< vector<float> > estScals = x->bubi->getEstimatedScalings();
    
    directory = "/Users/caramiaux/Research/Code/MaxSDK-6.1.4/examples/maxprojects/gvf-develop/maxexternal/datatests/gvflog-estimations-"+convert.str()+".txt";
    std::ofstream file_write2(directory.c_str());
    
    for(int j = 0; j < estAlign.size(); j++)
    {
        file_write2 << estAlign[j] << " ";
        for(int k = 0; k < estDynas[j].size(); k++)
            file_write2 << estDynas[j][k] << " ";
        for(int k = 0; k < estScals[j].size(); k++)
            file_write2 << estScals[j][k] << " ";
        file_write2 << endl;
    }
    file_write2.close();

    
}



///////////////////////////////////////////////////////////
//====================== RESTART
///////////////////////////////////////////////////////////
void gvf_restart(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    
    x->currentGesture->clear();
    idfile = 0;
    
    if(x->bubi->getState() == ofxGVF::STATE_FOLLOWING)
        x->bubi->restart();
    
}


///////////////////////////////////////////////////////////
//====================== tolerance
///////////////////////////////////////////////////////////
void gvf_tolerance(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    float stdnew = atom_getfloat(argv);
    if (stdnew <= 0.0)
        stdnew = 1.0;
    
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change Tolerance
    x->parameters.tolerance=stdnew;
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);

}

///////////////////////////////////////////////////////////
//====================== tolerance
///////////////////////////////////////////////////////////
void gvf_anticipate(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    int stdnew = atom_getlong(argv);
    
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change Tolerance
    x->parameters.predictionLoops=stdnew;
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
    
}

/////////////////////////////////////////////////////////////
////====================== tolerance
/////////////////////////////////////////////////////////////
void gvf_studentnu(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    float nu = atom_getfloat(argv);
    if (nu < 0.0) nu = 1.0;
    
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change Tolerance
    x->parameters.distribution=nu;
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
    
}


///////////////////////////////////////////////////////////
//====================== resampling_threshold
///////////////////////////////////////////////////////////
void gvf_resamplingthreshold(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    
    int rtnew = atom_getlong(&argv[0]);
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
void gvf_numberparticles(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    
    int nsNew = atom_getlong(&argv[0]);
    
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change Resampling threshold
    x->parameters.numberParticles  = nsNew;
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
    
    
}




///////////////////////////////////////////////////////////
//====================== phaseAdaptation / scaleAdaptation / speedAdaptation / angleAdaptation
///////////////////////////////////////////////////////////
void gvf_alignment(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change phase adaptation variance
    x->parameters.alignmentVariance = sqrt(atom_getfloat(argv));
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
}

void gvf_dynamics(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change ...
    if (argc == x->bubi->getDynamicsDim()) {
        for (int k=0; k< argc; k++)
            x->parameters.dynamicsVariance[k] = sqrt(atom_getfloat(&argv[k]));
    }
    if (argc == 1) {
        for (int k=0; k< x->bubi->getDynamicsDim(); k++)
            x->parameters.dynamicsVariance[k] = sqrt(atom_getfloat(argv));
    }
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
}

void gvf_scalings(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    // Get the current parameters
    x->parameters = x->bubi->getParameters();
    
    // Change ...
    if (argc == x->bubi->getScalingsDim()) {
        for (int k=0; k< argc; k++)
            x->parameters.scalingsVariance[k] = sqrt(atom_getfloat(&argv[k]));
    }
    if (argc == 1) {
        for (int k=0; k< x->bubi->getScalingsDim(); k++)
            x->parameters.scalingsVariance[k] = sqrt(atom_getfloat(argv));
    }
    
    // Set the new parameters
    x->bubi->setParameters(x->parameters);
}





///////////////////////////////////////////////////////////
//====================== translate
///////////////////////////////////////////////////////////
void gvf_translate(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    
    x->config = x->bubi->getConfig();
    x->config.translate = (atom_getlong(&argv[0])==0)? false : true;
    x->bubi->setConfig(x->config);
    
}

///////////////////////////////////////////////////////////
//====================== segmentation
///////////////////////////////////////////////////////////
void gvf_segmentation(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    
    x->config = x->bubi->getConfig();
    x->config.segmentation = (atom_getlong(&argv[0])==0)? false : true;
    x->bubi->setConfig(x->config);
    
}

///////////////////////////////////////////////////////////
//====================== normalize
///////////////////////////////////////////////////////////
void gvf_normalize(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    
    x->config = x->bubi->getConfig();
    x->config.normalization = (atom_getlong(&argv[0])==0)? false : true;
    x->bubi->setConfig(x->config);
    
}

///////////////////////////////////////////////////////////
//====================== normalize
///////////////////////////////////////////////////////////
void gvf_log(t_gvf *x,const t_symbol *sss, short argc, t_atom *argv)
{
    
    x->config = x->bubi->getConfig();
    x->config.logOn = (atom_getlong(&argv[0])==0)? false : true;
    x->bubi->setConfig(x->config);
    
}

///////////////////////////////////////////////////////////
//====================== SAVE_VOCABULARY
///////////////////////////////////////////////////////////
void gvf_savetemplates(t_gvf *x, const t_symbol *sss, short argc, t_atom *argv)
{
     t_symbol* mpath = atom_getsym(argv);
     string filename(mpath->s_name);
     x->bubi->saveTemplates(filename);
}



///////////////////////////////////////////////////////////
//====================== LOAD_VOCABULARY
///////////////////////////////////////////////////////////
void gvf_loadtemplates(t_gvf *x, const t_symbol *sss, short argc, t_atom *argv)
{
    
     char* mpath = atom_string(argv);
     int i=0;
     while ( *(mpath+i)!='/' )
     i++;
     mpath = mpath+i;
     string filename(mpath);
     x->bubi->loadTemplates(filename);
    
}


//
//  main.cpp
//  gfpf
//
//  Created by Thomas Rushmore on 16/05/2013.
//  Copyright (c) 2013 Thomas Rushmore. All rights reserved.
//

#include <boost/random.hpp>
#include <Eigen/Core>
#include <vector>

#include "m_pd.h"
#include "gfpf.h"
#include "globalutilities.h"

//#include "flext.h"

static t_class *gfpf_class;

// global vars
t_int restarted_l;
t_int restarted_d;
std::pair<t_float,t_float> xy0_l;
std::pair<t_float,t_float> xy0_d;
std::vector<t_float> vect_0_l;
std::vector<t_float> vect_0_d;
int argc_offset = 1;

enum {STATE_CLEAR, STATE_LEARNING, STATE_FOLLOWING};


typedef struct _gfpf {
    // t_object must be first
    t_object  x_obj;
    
    //baptiste's
    gfpf *bubi;
	t_int state;
	t_int lastreferencelearned;
    std::map<t_int,std::vector<std::pair<t_float,t_float> > > refmap;
	t_int Nspg, Rtpg;
	t_float sp, sv, sr, ss, so; // pos,vel,rot,scal,observation
	t_int pdim;
	Eigen::VectorXf mpvrs;
	Eigen::VectorXf rpvrs;
    
} t_gfpf;

static void *gfpf_new(t_symbol *s, int argc, t_atom *argv)
{
    post("gfpf - realtime adaptive gesture recognition");
    post("pd object - v 1.0.4");
    post("(C) Baptiste Caramiaux, Ircam, Goldsmiths");
    
    t_gfpf *x = (t_gfpf *)pd_new(gfpf_class);
    
    // create inlets and outlets
    //inlet_new(&x->x_obj,&x->x_obj.ob_pd,0,gensym("gfpfcontrol"));

    // initialise variables
    x->Nspg = 2000;
    t_int ns = x->Nspg; //!!
    x->Rtpg = 1000;
    t_int rt = x->Rtpg; //!!
    
    x->sp = 0.0001;
    x->sv = 0.01;
    x->ss = 0.0001;
    x->sr = 0.000001;
    x->so = 0.2;
    x->pdim = 4;
    Eigen::VectorXf sigs(x->pdim);
    sigs << x->sp, x->sv, x->ss, x->sr;
    
    x->bubi = new gfpf(ns, sigs, 1./(x->so * x->so), rt, 0.);
    x->mpvrs = Eigen::VectorXf(x->pdim);
    x->rpvrs = Eigen::VectorXf(x->pdim);
    x->mpvrs << 0.05, 1.0, 1.0, 0.0;
    x->rpvrs << 0.1,  0.4, 0.3, 0.0;
    
    restarted_l=1;
    restarted_d=1;
    
    if (argc > 0)
        x->Nspg = getint(argv);
    if (argc > 1)
        x->Rtpg = getint(argv + 1);
    
    return (void *)x;
}

static void gfpf_destructor(t_gfpf *x)
{
    post("\ngdpf destructor");
}

static void testmethod(t_gfpf *x,const t_symbol *sss,int argc, t_atom *argv)
{
    post("Test method works");
}

static void gfpfcontrol(t_gfpf *x,const t_symbol *sss,int argc, t_atom *argv)
{

    // check which param is being controlled
    std::string sym(atom_getsymbol(argv)->s_name);
    post("\nSelector: %s",sym.c_str());
    post("Argc: %d",argc);
    
//    t_float ar = getfloat(argv);
//    t_float ar2 = getfloat(argv+1);
//    t_int ar3 = getint(argv+2);
    //std::string sym(sss->s_name);
    
    argc -= argc_offset;
    argv++;
    
    if(sym.compare("learn") == 0)
    {
        if(argc != 1)
        {
            post("need another argument");
            return;
        }
        int refI = getint(argv);
        if(refI != x->lastreferencelearned+1)
        {
            post("you need to learn reference %d first",x->lastreferencelearned+1);
            return;
        }
        x->lastreferencelearned++;
        x->refmap[refI] = std::vector<std::pair<float, float> >();
        post("learning reference %d", refI);
        x->bubi->addTemplate();
        x->state = STATE_LEARNING;
        restarted_l=1;
        
    }
    else if(sym.compare("follow") == 0)
    {
        if(x->lastreferencelearned >= 0)
        {
            post("I'm about to follow!");
            x->bubi->spreadParticles(x->mpvrs,x->rpvrs);
            //post("nb of gest after following %i", bubi->getNbOfGestures());
            x->state = STATE_FOLLOWING;
        }
        else
        {
            post("no reference has been learned");
            return;
        }
    }
    else if(sym.compare("clear") == 0)
    {
        x->lastreferencelearned = -1;
        x->bubi->clear();
        restarted_l=1;
        restarted_d=1;
        x->state = STATE_CLEAR;
    }
    else if(sym.compare("data") == 0)
    {
        if(x->state == STATE_CLEAR)
        {
            post("what am I supposed to do ... I'm in standby!");
            return;
        }
        if(argc == 0)
        {
            post("invalid format, points have at least 1 coordinate");
            return;
        }
        if(x->state == STATE_LEARNING)
        {
            std::vector<float> vect(argc);
            if (argc ==2){
                if (restarted_l==1)
                {
                    // store the incoming list as a vector of float
                    for (int k=0; k<argc; k++){
                        //vect[k] = GetFloat(argv[k]);
                        //may not be correct..
                        vect[k] = getfloat(argv + k);
                    }
                    // keep track of the first point
                    vect_0_l = vect;
                    restarted_l=0;
                }
                for (int k=0; k<argc; k++){
                    vect[k] = getfloat(argv + k);
                    vect[k]=vect[k]-vect_0_l[k];
                }
            }
            else {
                for (int k=0; k<argc; k++)
                    vect[k] = getfloat(argv + k);
            }
            
            //				pair<float,float> xy;
            //				xy.first  = x -xy0_l.first;
            //				xy.second = y -xy0_l.second;
            
            // Fill template
            x->bubi->fillTemplate(x->lastreferencelearned,vect);
            
        }
        else if(x->state == STATE_FOLLOWING)
        {
            std::vector<float> vect(argc);
            if (argc==2){
                if (restarted_d==1)
                {
                    // store the incoming list as a vector of float
                    for (int k=0; k<argc; k++){
                        vect[k] = getfloat(argv + k);
                    }
                    
                    // keep track of the first point
                    vect_0_d = vect;
                    restarted_d=0;
                }
                
                for (int k=0; k<argc; k++){
                    vect[k] = vect[k] = getfloat(argv + k);
                    vect[k]=vect[k]-vect_0_d[k];
                }
            }
            else{
                post("%i",argc);
                for (int k=0; k<argc; k++)
                    vect[k] = vect[k] = getfloat(argv + k);
            }
            
            //post("%f %f",xy(0,0),xy(0,1));
            // ------- Fill template
            x->bubi->infer(vect);

            // output recognition
            Eigen::MatrixXf statu = x->bubi->getEstimatedStatus();
            //getGestureProbabilities();
            
            t_atom *outAtoms = new t_atom[statu.rows()];
            // de-refed. may be wrong.
            for(int j = 0; j < statu.rows(); j++)
                SETFLOAT(&outAtoms[j],statu(j,0));
                //SetFloat(outAtoms[j],statu(j,0));
            //ToOutList(0, statu.rows(), outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[statu.rows()];
            for(int j = 0; j < statu.rows(); j++)
                SETFLOAT(&outAtoms[j],statu(j,1));
            //ToOutList(1, statu.rows(), outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[statu.rows()];
            for(int j = 0; j < statu.rows(); j++)
                SETFLOAT(&outAtoms[j],statu(j,2));
            //ToOutList(2, statu.rows(), outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[statu.rows()];
            for(int j = 0; j < statu.rows(); j++)
                SETFLOAT(&outAtoms[j],statu(j,3));
            //ToOutList(3, statu.rows(), outAtoms);
            delete[] outAtoms;
            
            Eigen::VectorXf gprob = x->bubi->getGestureConditionnalProbabilities();
            outAtoms = new t_atom[gprob.size()];
            for(int j = 0; j < gprob.size(); j++)
                SETFLOAT(&outAtoms[j],gprob(j,0));
            //ToOutList(4, gprob.size(), outAtoms);
            delete[] outAtoms;
            
            gprob = x->bubi->getGestureLikelihoods();
            outAtoms = new t_atom[gprob.size()];
            for(int j = 0; j < gprob.size(); j++)
                SETFLOAT(&outAtoms[j],gprob(j,0));
            //ToOutList(5, gprob.size(), outAtoms);
            delete[] outAtoms;
            
            
            
        }
    }
    else if(sym.compare("printme") == 0)
    {
        post("N. particles %d: ", x->bubi->getNbOfParticles());
        post("Resampling Th. %d: ", x->bubi->getResamplingThreshold());
        post("Means %.3f %.3f %.3f %.3f: ", x->mpvrs[0], x->mpvrs[1], x->mpvrs[2], x->mpvrs[3]);
        post("Ranges %.3f %.3f %.3f %.3f: ", x->rpvrs[0], x->rpvrs[1], x->rpvrs[2], x->rpvrs[3]);
        for(int i = 0; i < x->bubi->getNbOfTemplates(); i++)
        {
            post("reference %d: ", i);
            std::vector<std::vector<float> > tplt = x->bubi->getTemplateByInd(i);
            for(int j = 0; j < tplt.size(); j++)
            {
                post("%02.4f  %02.4f", tplt[j][0], tplt[j][1]);
            }
        }
        
    }
    else if(sym.compare("restart") == 0)
    {
        restarted_l=1;
        if(x->state == STATE_FOLLOWING)
        {
            
            x->bubi->spreadParticles(x->mpvrs,x->rpvrs);
            
            restarted_l=1;
            restarted_d=1;
        }
    }
    else if(sym.compare("std") == 0)
    {
        
        float stdnew = getfloat(argv);
        if (stdnew == 0.0)
            stdnew = 0.1;
        x->bubi->setIcovSingleValue(1/(stdnew*stdnew));
    }
    else if(sym.compare("rt") == 0)
    {
        int rtnew = getint(argv);
        int cNS = x->bubi->getNbOfParticles();
        if (rtnew >= cNS)
            rtnew = floor(cNS/2);
        x->bubi->setResamplingThreshold(rtnew);
    }
    else if(sym.compare("means") == 0)
    {
        x->mpvrs = Eigen::VectorXf(x->pdim);
        x->mpvrs << getfloat(argv), getfloat(argv + 1), getfloat(argv + 2), getfloat(argv + 3);
    }
    else if(sym.compare("ranges") == 0)
    {
        x->rpvrs = Eigen::VectorXf(x->pdim);
        x->rpvrs << getfloat(argv), getfloat(argv + 1), getfloat(argv + 2), getfloat(argv + 3);
    }
    else if(sym.compare("adaptSpeed") == 0)
    {
        std::vector<float> as;
        as.push_back(getfloat(argv));
        as.push_back(getfloat(argv + 1));
        as.push_back(getfloat(argv + 2));
        as.push_back(getfloat(argv + 3));
        
        x->bubi->setAdaptSpeed(as);
    }
}

extern "C"
{
    void gfpf_setup(void) {
        gfpf_class = class_new( gensym("gfpf"),
                                (t_newmethod)gfpf_new,
                                (t_method)gfpf_destructor,
                                sizeof(t_gfpf),
                                CLASS_DEFAULT,
                                A_GIMME,
                                0 );
        class_addmethod(gfpf_class,(t_method)gfpfcontrol,gensym("list"),A_GIMME,0);
        class_addmethod(gfpf_class,(t_method)testmethod,gensym("test"),A_GIMME,0);
        
        
    }
}
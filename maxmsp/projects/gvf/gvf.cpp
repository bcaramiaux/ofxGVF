///////////////////////////////////////////////////////////////////////
//
//  GVF - Gesture Variation Follower Max/MSP Object
//
//
//  Copyright (C) 2013 Baptiste Caramiaux, Goldsmiths College, University of London
//
//  The GVF library is under the GNU Lesser General Public License (LGPL v3)
//  version: 19-09-2013
//
//  contact: b.caramiaux@gold.ac.uk
//
///////////////////////////////////////////////////////////////////////


#include "maxcpp6.h"
#include "GestureVariationFollower.h"
#include <string>
#include <map>
#include <vector>
#include <Eigen/Core>
#include <unistd.h>

using namespace std;


enum {STATE_CLEAR, STATE_LEARNING, STATE_FOLLOWING};

int restarted_l;
int restarted_d;
pair<float,float> xy0_l;
pair<float,float> xy0_d;
vector<float> vect_0_l;
vector<float> vect_0_d;




class gvf : public MaxCpp6<gvf> {
private:
	GestureVariationFollower *bubi;
	int state;
	int lastreferencelearned;
	map<int,vector<pair<float,float> > > refmap;
	int Nspg, Rtpg;
	float sp, sv, sr, ss, so; // pos,vel,rot,scal,observation
	int pdim;
	Eigen::VectorXf mpvrs;
	Eigen::VectorXf rpvrs;
    float value_mmax;
    bool offline_recognition;
    int toBeTranslated;
    
public:
    
    //	Example(t_symbol * sym, long ac, t_atom * av) {
    //		setupIO(2, 2); // inlets / outlets
    //	}
    gvf(t_symbol * sym, long argc, t_atom *argv)
    {
        setupIO(1, 3); // inlets / outlets
        post("gvf - realtime adaptive gesture recognition (version: 19-09-2013)");
        post("(c) Goldsmiths, University of London and Ircam - Centre Pompidou");
        post("    contact: Baptiste Caramiaux b.caramiaux@gold.ac.uk");
        
		// default values
		Nspg = 2000; int ns = Nspg; //!!
		Rtpg = 500; int rt = Rtpg; //!!
        
        
		sp = 0.0001;
		sv = 0.001;
		ss = 0.0001;
		sr = 0.000000;
		
		
		so = 0.2;
		
		pdim = 4;
		Eigen::VectorXf sigs(pdim);
		sigs << sp, sv, ss, sr;
		
		bubi = new GestureVariationFollower(ns, sigs, 1./(so * so), rt, 0.);
		
		
		mpvrs = Eigen::VectorXf(pdim);
		rpvrs = Eigen::VectorXf(pdim);
		mpvrs << 0.05, 1.0, 1.0, 0.0;
		rpvrs << 0.1,  0.4, 0.3, 0.0;
		
		restarted_l=1;
		restarted_d=1;
        
		state = STATE_CLEAR;
        
        lastreferencelearned = -1;
        value_mmax = -INFINITY;
        
        toBeTranslated = 1;
        offline_recognition = false;
		
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
        post("shit float");
		outlet_float(m_outlets[0], v);
	}
	void testint(long inlet, long v) {
        post("shit long");
		outlet_int(m_outlets[0], v);
	}
    
    // Methods order:
    // - learn
    // - follow
    // - data
    // - save_vocabulary
    // - load_vocabulary
    // - clear
    // - printme
    // - restart
    // - tolerance
    // - resampling_threshold
    // - spreading_means
    // - spreading_ranges
    // - adaptation_speed
    
    
    ///////////////////////////////////////////////////////////
    //====================== LEARN
    ///////////////////////////////////////////////////////////
	void learn(long inlet, t_symbol * s, long ac, t_atom * av) {
        
        if(ac != 1)
        {
            post("wrong number of argument (must be 1)");
            return;
        }
        int refI = atom_getlong(&av[0]);
        refI = refI-1; // start at 1 in the patch but 0 in C++
        if(refI != lastreferencelearned+1)
        {
            post("you need to learn reference %d first",lastreferencelearned+1);
            return;
        }
        lastreferencelearned++;
        refmap[refI] = vector<pair<float, float> >();
        post("learning reference %d", refI+1);
        
        bubi->addTemplate();
        
        state = STATE_LEARNING;
        
        restarted_l=1;
        
    }
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== FOLLOW
    ///////////////////////////////////////////////////////////
    void follow(long inlet, t_symbol * s, long ac, t_atom * av) {
        
        if(lastreferencelearned >= 0)
        {
            //post("I'm about to follow!");
            
            bubi->spreadParticles(mpvrs,rpvrs);
            restarted_l=1;
            restarted_d=1;
            //post("nb of gest after following %i", bubi->getNbOfGestures());
            
            state = STATE_FOLLOWING;
            
        }
        else
        {
            post("no reference has been learned");
            return;
        }
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
        if(state == STATE_CLEAR)
        {
            post("I'm in a standby (must learn something beforehand)");
            return;
        }
        
        // INCOMING DATA - LEARNING MODE
        // -----------------------------
        else if(state == STATE_LEARNING)
        {
            
            vector<float> vect(ac);
            
            if (toBeTranslated){
            // incoming observation is 2-dimensional: keeping track of the first point (offset)
//            if (ac ==2){
                if (restarted_l==1)
                {
                    // keep track of the first point
                    for (int k=0; k<ac; k++)
                    {
                        vect[k] = atom_getfloat(&av[k]);
                    }
                    vect_0_l = vect;
                    restarted_l=0;
                }
                // store the incoming list as a vector of float
                for (int k=0; k<ac; k++)
                {
                    vect[k] = atom_getfloat(&av[k]);
                    vect[k]=vect[k]-vect_0_l[k];
                }
            }
            // otherwise just store the incoming obs
            else {
                for (int k=0; k<ac; k++){
                    vect[k] = atom_getfloat(&av[k]);
                    if (vect[k]>=value_mmax)
                        value_mmax=abs(vect[k]);
                }
            }
            //            float newcov = so *value_mmax;
            //            bubi->setIcovSingleValue(1/(newcov*newcov));
            // Fill template with the observation
            bubi->fillTemplate(lastreferencelearned,vect);
            
            
        }
        
        // INCOMING DATA - FOLLOWING MODE
        // ------------------------------
        else if(state == STATE_FOLLOWING)
        {
                
                vector<float> vect(ac);

            if (toBeTranslated){
                
                // incoming observation is 2-dimensional: translate the data by the offset
//                if (ac==2){
                    if (restarted_d==1)
                    {
                        // store the incoming list as a vector of float
                        for (int k=0; k<ac; k++)
                        {
                            vect[k] = atom_getfloat(&av[k]);
                        }
                        vect_0_d = vect;
                        restarted_d=0;
                    }
                    // store the data in vect and translate
                    for (int k=0; k<ac; k++)
                    {
                        vect[k] = atom_getfloat(&av[k]);
                        vect[k]=vect[k]-vect_0_d[k];
                    }
                }
                else
                {
                    for (int k=0; k<ac; k++)
                        vect[k] = atom_getfloat(&av[k]);
                }

                // perform the inference with the current observation
                bubi->infer(vect);

            
            
            // output recognition
            
            Eigen::MatrixXf statu = bubi->getEstimatedStatus(); //getGestureProbabilities();
            t_atom *outAtoms = new t_atom[statu.rows()];
            
            for(int j = 0; j < statu.rows(); j++)
                atom_setfloat(&outAtoms[j],statu(j,0));
            outlet_anything(m_outlets[0], gensym("phase"), statu.rows(), outAtoms);
            delete[] outAtoms;
            
            for(int j = 0; j < statu.rows(); j++)
                atom_setfloat(&outAtoms[j],statu(j,1));
            outlet_anything(m_outlets[0], gensym("speed"), statu.rows(), outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[statu.rows()];
            for(int j = 0; j < statu.rows(); j++)
                atom_setfloat(&outAtoms[j],statu(j,2));
            outlet_anything(m_outlets[0], gensym("scale"), statu.rows(), outAtoms);
            delete[] outAtoms;
            
            outAtoms = new t_atom[statu.rows()];
            for(int j = 0; j < statu.rows(); j++)
                atom_setfloat(&outAtoms[j],statu(j,3));
            outlet_anything(m_outlets[0], gensym("angle"), statu.rows(), outAtoms);
            delete[] outAtoms;
            
            Eigen::VectorXf gprob = bubi->getGestureConditionnalProbabilities();
            outAtoms = new t_atom[gprob.size()];
            for(int j = 0; j < gprob.size(); j++)
                atom_setfloat(&outAtoms[j],gprob[j]);
            outlet_anything(m_outlets[1], gensym("weights"), statu.rows(), outAtoms);
            delete[] outAtoms;
            
            std::vector<float> aw = bubi->abs_weights;
            outAtoms = new t_atom[aw.size()];
            for(int j = 0; j < aw.size(); j++)
                atom_setfloat(&outAtoms[j],aw[j]);
            outlet_anything(m_outlets[2], gensym("absweights"), aw.size(), outAtoms);
            delete[] outAtoms;
            
            std::vector<float>* offs = bubi->offset;
            outAtoms = new t_atom[(*offs).size()];
            for(int j = 0; j < (*offs).size(); j++)
                atom_setfloat(&outAtoms[j],(*offs)[j]);
            outlet_anything(m_outlets[2], gensym("offset"), (*offs).size(), outAtoms);
            delete[] outAtoms;
            
        }
    }
    
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== SAVE_VOCABULARY
    ///////////////////////////////////////////////////////////
    void save_vocabulary(long inlet, t_symbol * s, long ac, t_atom * av){
        char* mpath = atom_string(av);
        //        string filename = "/Users/caramiaux/gotest";
        int i=0;
        while ( *(mpath+i)!='/' )
            i++;
        mpath = mpath+i;
        string filename(mpath);
        //post("%s", filename.c_str());
        bubi->saveTemplates(filename);
        
    }
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== LOAD_VOCABULARY
    ///////////////////////////////////////////////////////////
    void load_vocabulary(long inlet, t_symbol * s, long ac, t_atom * av){
        char* mpath = atom_string(av);
        //        string filename = "/Users/caramiaux/gotest.txt";
        int i=0;
        while ( *(mpath+i)!='/' )
            i++;
        mpath = mpath+i;
        string filename(mpath);
        bubi->loadTemplates(filename);
        lastreferencelearned=bubi->getNbOfTemplates()-1;
        
        t_atom* outAtoms = new t_atom[1];
        atom_setlong(&outAtoms[0],bubi->getNbOfTemplates());
        outlet_anything(m_outlets[2], gensym("vocabulary_size"), 1, outAtoms);
        delete[] outAtoms;
    }
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== CLEAR
    ///////////////////////////////////////////////////////////
    void clear(long inlet, t_symbol * s, long ac, t_atom * av) {
        lastreferencelearned = -1;
        
        bubi->clear();
        
        restarted_l=1;
        restarted_d=1;
        
        value_mmax = -INFINITY;
        
        state = STATE_CLEAR;
    }
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== PRINTME
    ///////////////////////////////////////////////////////////
    void printme(long inlet, t_symbol * s, long ac, t_atom * av) {
        post("N. particles %d: ", bubi->getNbOfParticles());
        post("Resampling Th. %d: ", bubi->getResamplingThreshold());
        post("Tolerance %.2f: ", bubi->getObservationNoiseStd());
        post("Means %.3f %.3f %.3f %.3f: ", mpvrs[0], mpvrs[1], mpvrs[2], mpvrs[3]);
        post("Ranges %.3f %.3f %.3f %.3f: ", rpvrs[0], rpvrs[1], rpvrs[2], rpvrs[3]);
        for(int i = 0; i < bubi->getNbOfTemplates(); i++)
        {
            post("reference %d: ", i);
            vector<vector<float> > tplt = bubi->getTemplateByInd(i);
            for(int j = 0; j < tplt.size(); j++)
            {
                post("%02.4f  %02.4f", tplt[j][0], tplt[j][1]);
            }
        }
        
    }
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== RESTART
    ///////////////////////////////////////////////////////////
    void restart(long inlet, t_symbol * s, long ac, t_atom * av) {
        restarted_l=1;
        if(state == STATE_FOLLOWING)
        {
            
            bubi->spreadParticles(mpvrs,rpvrs);
            
            restarted_l=1;
            restarted_d=1;
        }
    }
    
    
    
    
    ///////////////////////////////////////////////////////////
    //====================== tolerance
    ///////////////////////////////////////////////////////////
    void tolerance(long inlet, t_symbol * s, long ac, t_atom * av) {
        float stdnew = atom_getfloat(&av[0]);
        if (stdnew == 0.0)
            stdnew = 0.1;
        bubi->setIcovSingleValue(1/(stdnew*stdnew));
    }
    
    ///////////////////////////////////////////////////////////
    //====================== resampling_threshold
    ///////////////////////////////////////////////////////////
    void resampling_threshold(long inlet, t_symbol * s, long ac, t_atom * av) {
        int rtnew = atom_getlong(&av[0]);
        int cNS = bubi->getNbOfParticles();
        if (rtnew >= cNS)
            rtnew = floor(cNS/2);
        bubi->setResamplingThreshold(rtnew);
    }
    
    ///////////////////////////////////////////////////////////
    //====================== spreading_means
    ///////////////////////////////////////////////////////////
    void spreading_means(long inlet, t_symbol * s, long ac, t_atom * av) {
        mpvrs = Eigen::VectorXf(pdim);
        mpvrs << atom_getfloat(&av[0]), atom_getfloat(&av[1]), atom_getfloat(&av[2]), atom_getfloat(&av[3]);
    }
    
    ///////////////////////////////////////////////////////////
    //====================== spreading_ranges
    ///////////////////////////////////////////////////////////
    void spreading_ranges(long inlet, t_symbol * s, long ac, t_atom * av) {
        rpvrs = Eigen::VectorXf(pdim);
        rpvrs << atom_getfloat(&av[0]), atom_getfloat(&av[1]), atom_getfloat(&av[2]), atom_getfloat(&av[3]);
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
        
        bubi->setAdaptSpeed(as);
    }
    
    void probThresh(long inlet, t_symbol * s, long ac, t_atom * av) {
        bubi->probThresh = atom_getlong(&av[0])*bubi->getNbOfParticles();
    }
    
    void probThreshMin(long inlet, t_symbol * s, long ac, t_atom * av) {
        bubi->probThreshMin = atom_getlong(&av[0])*bubi->getNbOfParticles();
    }
    
    ///////////////////////////////////////////////////////////
    //====================== translate
    ///////////////////////////////////////////////////////////
    void translate(long inlet, t_symbol * s, long ac, t_atom * av){
        toBeTranslated = atom_getlong(&av[0]);
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
    REGISTER_METHOD_GIMME(gvf, save_vocabulary);
    REGISTER_METHOD_GIMME(gvf, load_vocabulary);
    REGISTER_METHOD_GIMME(gvf, translate);
}

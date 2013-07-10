//
//  gfpfhandler.h
//  graphicsExample
//
//  Created by Thomas Rushmore on 21/06/2013.
//
//

#ifndef __graphicsExample__gfpfhandler__
#define __graphicsExample__gfpfhandler__

#include <iostream>
#include "gfpf.h"

enum {STATE_CLEAR, STATE_LEARNING, STATE_FOLLOWING};

class gfpfhandler{
public:
    gfpfhandler(int s_rt,int s_gt);
    ~gfpfhandler();
    
    void teach(int p_type);
    
    void gfpf_learn      (int argc, int argv);
    void gfpf_follow     (int argc, int *argv);
    void gfpf_clear      (int argc, int *argv);
    void gfpf_data       (int argc, float *argv);
    void gfpf_printme    (int argc, int *argv);
    void gfpf_restart    (int argc, int *argv);
    void gfpf_std        (int argc, float *argv);
    void gfpf_rt         (int argc, int *argv);
    void gfpf_means      (int argc, int *argv);
    void gfpf_ranges     (int argc, int *argv);
    void gfpf_adaptspeed (int argc, int *argv);

private:
    int selected_gesture;
    gfpf *gf;
    float sp, sv, sr, ss, so;
    int rp,pdim,ns,rt;
    Eigen::VectorXf mpvrs;
	Eigen::VectorXf rpvrs;
    int Nspg, Rtpg;
    int state;
	int lastreferencelearned;
    std::map<int,std::vector<std::pair<float,float> > > refmap;
    int restarted_l;
    int restarted_d;
    std::pair<float,float> xy0_l;
    std::pair<float,float> xy0_d;
    std::vector<float> vect_0_l;
    std::vector<float> vect_0_d;
   

};
#endif /* defined(__graphicsExample__gfpfhandler__) */

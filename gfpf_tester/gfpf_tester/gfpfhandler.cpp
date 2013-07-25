//
//  gfpfhandler.cpp
//  graphicsExample
//
//  Created by Thomas Rushmore on 21/06/2013.
//
//

#include "gfpfhandler.h"

gfpfhandler::gfpfhandler(int s_rt,int s_gt)
{
    sp = 0.0001;
    sv = 0.01;
    ss = 0.0001;
    sr = 0.000001;
    so = 0.2;
    rp = 1000;
    pdim = 4;
    Nspg = 2000;
    ns = Nspg;
    Rtpg = 1000;
    rt = Rtpg;
    
    Rtpg = s_rt;
    rt = s_rt;
    if(s_gt <= 0 || s_gt > 5){
        printf("setGesture must be integer between 1-45inclusive\n");
        printf("setGesture defaulting to 1\n");
        selected_gesture = 1;
    } else {
        selected_gesture = s_gt;
    }
    
    Eigen::VectorXf sigs(pdim);
    sigs << sp, sv, ss, sr;
    // pos,vel,rot,scal,observation
    
    gf = new gfpf(ns, sigs, 1./(so * so), rt, 0.);
    mpvrs = Eigen::VectorXf(pdim);
    rpvrs = Eigen::VectorXf(pdim);
    mpvrs << 0.05, 1.0, 1.0, 0.0;
    rpvrs << 0.1,  0.4, 0.3, 0.0;
    state = STATE_CLEAR;
    lastreferencelearned = -1;
    restarted_l=1;
    restarted_d=1;
}

gfpfhandler::~gfpfhandler()
{
    if(gf != NULL)
        delete gf;
//    if(refmap != NULL)
//        delete refmap;

}

void gfpfhandler::teach(int p_type)
{
    printf("Running teach()\n");
   // gfpf_rt(<#int argc#>, <#int *argv#>)
    gfpf_restart(0, 0);
    gfpf_std(0,0);
    int num_templates = 3;
    gfpf_clear(0,0);
    std::string dir = "/Users/thomasrushmore/EAVI/PD/gfpflibrary/test gestures/tem";
    std::string dirg = "/Users/thomasrushmore/EAVI/PD/gfpflibrary/test gestures/g";
    
    
    for(int i = 0 ; i < num_templates; i++)
    {
       
        int ab = i;
        gfpf_learn(1, ab);
        std::string state_file(dir);
        char buf[10];
        sprintf(buf, "%d", i+1);
        state_file.append(buf);
        state_file.append(".txt");
        //post(state_file.c_str());
        std::ifstream state_summary;
        state_summary.open(state_file.c_str());
        float a;
        float b;
        float *ar = new float[2];
        
            
        while(true){
            state_summary >> a;
            state_summary >> b;
            ar[0] = a;
            ar[1] = b;
            gfpf_data(2, ar);
            if(state_summary.eof()) break;
        }
        gfpf_restart(0, 0);
        state_summary.close();
        delete ar;
    }
    
    // gfpf follow
    gfpf_follow(0, 0);
    // gfpf data
    std::string tem_folder;
    
    if(p_type == 0)
        tem_folder = dir;
    else
        tem_folder = dirg;
    
    std::string state_file(tem_folder);
    
    char buf[10];
    int gesture = selected_gesture;
    
    sprintf(buf, "%d",gesture);
    state_file.append(buf);
    state_file.append(".txt");
    //post(state_file.c_str());
    std::ifstream state_summary;
    state_summary.open(state_file.c_str());
    float *ar = new float[2];

    float a,b;
    while(true){
        state_summary >> a;
        state_summary >> b;
        ar[0] = a;
        ar[1] = b;
        gfpf_data(2, ar);
        if(state_summary.eof()) break;
    }
    delete ar;
    
    gfpf_restart(0, 0);

    
    
}

void gfpfhandler::gfpf_learn(int argc, int argv)
{
    if(argc != 1)
    {
        return;
    }
    int refI = argv;
    // delete this on release
    if(refI != lastreferencelearned+1)
    {
        return;
    }
    lastreferencelearned++;
    //refmap[refI];// = std::vector<std::pair<float, float> >();
    refmap[refI] = std::vector<std::pair<float, float> >();
    gf->addTemplate();
    state = STATE_LEARNING;
    restarted_l=1;
}

 void gfpfhandler:: gfpf_follow(int argc, int *argv)
{
    if(lastreferencelearned >= 0)
    {
        gf->spreadParticles(mpvrs,rpvrs);
        //printf("nb of gest after following %i", gf->getNbOfGestures());
        state = STATE_FOLLOWING;
    }
    else
    {
        return;
    }
}

void gfpfhandler::gfpf_clear(int argc, int *argv)
{
    lastreferencelearned = -1;
    gf->clear();
    restarted_l=1;
    restarted_d=1;
    state = STATE_CLEAR;
}


void gfpfhandler::gfpf_data(int argc, float *argv)
{
    
    if(state == STATE_CLEAR)
    {
        return;
    }
    if(argc == 0)
    {
        return;
    }
    if(state == STATE_LEARNING)
    {
        std::vector<float> vect(argc);
        if (argc ==2){
            if (restarted_l==1)
            {
                // store the incoming list as a vector of float
                for (int k=0; k<argc; k++){
                    //vect[k] = *(argv[k]);
                    //may not be correct..
                    vect[k] = *(argv + k);
                }
                // keep track of the first point
                vect_0_l = vect;
                restarted_l=0;
            }
            for (int k=0; k<argc; k++){
                vect[k] = *(argv + k);
                // normalize 
                vect[k]=vect[k]-vect_0_l[k];
            }
        }
        else {
            for (int k=0; k<argc; k++)
                vect[k] = *(argv + k);
        }
        
        //				pair<float,float> xy;
        //				xy.first  = x -xy0_l.first;
        //				xy.second = y -xy0_l.second;
        
        // Fill template
        gf->fillTemplate(lastreferencelearned,vect);
        
    }
    else if(state == STATE_FOLLOWING)
    {
        std::vector<float> vect(argc);
        if (argc==2){
            if (restarted_d==1)
            {
                // store the incoming list as a vector of float
                for (int k=0; k<argc; k++){
                    vect[k] = *(argv + k);
                }
                // keep track of the first point
                vect_0_d = vect;
                restarted_d=0;
            }
            for (int k=0; k<argc; k++){
                vect[k] = vect[k] = *(argv + k);
                vect[k]= vect[k]-vect_0_d[k];
            }
        }
        else{
            printf("%i",argc);
            for (int k=0; k<argc; k++)
                vect[k] = vect[k] = *(argv + k);
        }
        //printf("%f %f",xy(0,0),xy(0,1));
        // ------- Fill template
        gf->infer(vect);
        // output recognition
        Eigen::MatrixXf statu = gf->getEstimatedStatus();
        //ggetGestureProbabilities();
        float tot = gf->inferTotalGestureActivity();

//        int *outAtoms = new int[statu.rows()];
//        // de-refed. may be wrong.
//        for(int j = 0; j < statu.rows(); j++)
//            SETFLOAT(&outAtoms[j],statu(j,0));
//        outlet_list(Position, &s_list, statu.rows(), outAtoms);
//        delete[] outAtoms;
//        
//        outAtoms = new int[statu.rows()];
//        for(int j = 0; j < statu.rows(); j++)
//            SETFLOAT(&outAtoms[j],statu(j,1));
//        outlet_list(Vitesse, &s_list, statu.rows(), outAtoms);
//        delete[] outAtoms;
//        
//        outAtoms = new int[statu.rows()];
//        for(int j = 0; j < statu.rows(); j++)
//            SETFLOAT(&outAtoms[j],statu(j,2));
//        outlet_list(Rotation, &s_list, statu.rows(), outAtoms);
//        delete[] outAtoms;
//        
//        outAtoms = new int[statu.rows()];
//        for(int j = 0; j < statu.rows(); j++)
//            SETFLOAT(&outAtoms[j],statu(j,3));
//        outlet_list(Scaling, &s_list, statu.rows(), outAtoms);
//        delete[] outAtoms;
//        
//        Eigen::VectorXf gprob = gf->getGestureConditionnalProbabilities();
//        outAtoms = new int[gprob.size()];
//        for(int j = 0; j < gprob.size(); j++)
//            SETFLOAT(&outAtoms[j],gprob(j,0));
//        outlet_list(Recognition, &s_list, gprob.size(), outAtoms);
//        delete[] outAtoms;
//        
//        gprob = gf->getGestureLikelihoods();
//        outAtoms = new int[gprob.size()];
//        for(int j = 0; j < gprob.size(); j++)
//            SETFLOAT(&outAtoms[j],gprob(j,0));
//        outlet_list(Likelihoods, &s_list, gprob.size(), outAtoms);
//        delete[] outAtoms;
    }
}

 void gfpfhandler::gfpf_printme(int argc, int *argv)
{
    printf("\nN. particles %d: ", gf->getNbOfParticles());
    printf("Resampling Th. %d: ", gf->getResamplingThreshold());
    printf("Means %.3f %.3f %.3f %.3f: ", mpvrs[0], mpvrs[1], mpvrs[2], mpvrs[3]);
    printf("Ranges %.3f %.3f %.3f %.3f: ", rpvrs[0], rpvrs[1], rpvrs[2], rpvrs[3]);
    for(int i = 0; i < gf->getNbOfTemplates(); i++)
    {
        printf("reference %d: ", i);
        std::vector<std::vector<float> > tplt = gf->getTemplateByInd(i);
        for(int j = 0; j < tplt.size(); j++)
        {
            printf("%02.4f  %02.4f", tplt[j][0], tplt[j][1]);
        }
    }
}

 void gfpfhandler::gfpf_restart(int argc, int *argv)
{
    //executiontimer tmr("Restart");
    restarted_l=1;
    if(state == STATE_FOLLOWING)
    {
        gf->spreadParticles(mpvrs,rpvrs);
        restarted_l=1;
        restarted_d=1;
        gf->writeGesturesToFile();

    }
}

 void gfpfhandler::gfpf_std(int argc, float *argv)
{
    float stdnew = 0.0;
    if (stdnew == 0.0)
        stdnew = 0.1;
    gf->setIcovSingleValue(1/(stdnew*stdnew));
}

 void gfpfhandler::gfpf_rt(int argc, int *argv)
{
    int rtnew = *(argv);
    int cNS = gf->getNbOfParticles();
    if (rtnew >= cNS)
        rtnew = floor(cNS/2);
    gf->setResamplingThreshold(rtnew);
}

void gfpfhandler::gfpf_means(int argc, int *argv)
{
    mpvrs = Eigen::VectorXf(pdim);
    mpvrs << *(argv), *(argv + 1), *(argv + 2), *(argv + 3);
}

void gfpfhandler::gfpf_ranges(int argc, int *argv)
{
    rpvrs = Eigen::VectorXf(pdim);
    rpvrs << *(argv), *(argv + 1), *(argv + 2), *(argv + 3);
}

void gfpfhandler::gfpf_adaptspeed(int argc, int *argv)
{
    std::vector<float> as;
    as.push_back(*(argv));
    as.push_back(*(argv + 1));
    as.push_back(*(argv + 2));
    as.push_back(*(argv + 3));
    
    gf->setAdaptSpeed(as);
}
//
//  gvfhandler.cpp
//  ofgvfVisualiser
//
//  Created by Thomas Rushmore on 21/06/2013.
//  Modified by Igor Correa
//

#include "gvfhandler.h"

gvfhandler::gvfhandler()
{
    // initialisation of parameters. Funcions are provided to change most of them at runtime
    sigPosition = 0.0001;
    sigSpeed = 0.01;
    sigScale = 0.0001;
    sigRotation = 0.000001;
    smoothingCoef = 0.2;
    ns = Nspg = 2000;

    rp = 1000;
    pdim = 4;
    rt = Rtpg = 1000;
    
    // variance coefficients
    Eigen::VectorXf sigs(pdim);

    // possition, speed, scale, rotation
    sigs << sigPosition, sigSpeed, sigScale, sigRotation;
    
    mygvf = new GestureVariationFollower(ns, sigs, 1./(smoothingCoef * smoothingCoef), rt, 0.);
    
    mpvrs = Eigen::VectorXf(pdim);
    rpvrs = Eigen::VectorXf(pdim);
    mpvrs << 0.05, 1.0, 1.0, 0.0;
    rpvrs << 0.1,  0.4, 0.3, 0.0;
    
    state = STATE_CLEAR;
    
    lastreferencelearned = -1;
    restarted_l=1;
    restarted_d=1;
    
    refmap = new std::map<int,std::vector<std::pair<float,float> > >;
}

gvfhandler::~gvfhandler()
{
    if(mygvf != NULL)
        delete mygvf;
    if(refmap != NULL)
        delete refmap;

}

int gvfhandler::gvf_learn()
{
    lastreferencelearned++;
    (*refmap)[lastreferencelearned] = std::vector<std::pair<float, float> >();
    mygvf->addTemplate();
    state = STATE_LEARNING;
    restarted_l=1;
    return lastreferencelearned;
}

 void gvfhandler:: gvf_follow()
{
    if(lastreferencelearned >= 0)
    {
        mygvf->spreadParticles(mpvrs,rpvrs);
        state = STATE_FOLLOWING;
    }
    else
    {
        return;
    }
}

void gvfhandler::gvf_clear()
{
    lastreferencelearned = -1;
    mygvf->clear();
    restarted_l=1;
    restarted_d=1;
    state = STATE_CLEAR;
    templates.clear();
}

void gvfhandler::gvf_data(ofPoint p)
{
    float pointf[2];
    pointf[0] = p[0] + 1;
    pointf[1] = p[1] + 1;
    gvf_data(2, pointf);
}

void gvfhandler::gvf_data(int argc, float *argv)
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
        mygvf->fillTemplate(lastreferencelearned,vect);
        
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
                vect[k]=vect[k]-vect_0_d[k];
            }
        }
        else{
            printf("%i",argc);
            for (int k=0; k<argc; k++)
                vect[k] = vect[k] = *(argv + k);
        }
        //printf("%f %f",xy(0,0),xy(0,1));
        
        // ------- Fill template
        mygvf->infer(vect);
        // output recognition
        Eigen::MatrixXf statu = mygvf->getEstimatedStatus();
        //getGestureProbabilities();
        Eigen::VectorXf glikelihoods = mygvf->getGestureLikelihoods();
        Eigen::VectorXf gprob = mygvf->getGestureConditionnalProbabilities();

        char temp[100];
        int templates_count = gprob.size();
        
        recogInfo.clear();
        
        for(int i = 0; i < templates_count; i++)
        {
            recognitionInfo info;
            info.likelihoods = glikelihoods(i, 0);
            info.probability = gprob(i, 0);
            info.phase = statu(i, 0);
            info.speed = statu(i, 1);
            info.scale = statu(i, 2);
            info.rotation = statu(i, 3);
            recogInfo.push_back(info);
        }
    }
}

string gvfhandler::gvf_get_status()
{
    char temp[4][100];
    string status_string;

    strcpy(temp[0],"\nProbability\n");
    strcpy(temp[1],"Phase\n");
    strcpy(temp[2],"Speed\n");
    strcpy(temp[3],"Scale\n");
    for(int i = 0; i < recogInfo.size(); i++)
    {
        sprintf(temp[0], "%s%5.2f ",temp[0], recogInfo[i].probability);
        sprintf(temp[1], "%s%5.2f ",temp[1], recogInfo[i].phase);
        sprintf(temp[2], "%s%5.2f ",temp[2], recogInfo[i].speed);
        sprintf(temp[3], "%s%5.2f ",temp[3], recogInfo[i].scale);
    }
    for(int i = 0; i < 4; i++)
    {
        strcat(temp[i], "\n");
        status_string.append(temp[i]);
    }
    return status_string;

}

 void gvfhandler::gvf_restart()
{
    restarted_l=1;
    if(state == STATE_FOLLOWING)
    {
        mygvf->spreadParticles(mpvrs,rpvrs);
        restarted_l=1;
        restarted_d=1;
    }
}

 void gvfhandler::gvf_std(float smoothingCoeficient)
{
    float stdnew = smoothingCoeficient;
    if (stdnew == 0.0)
        stdnew = 0.1;
    mygvf->setIcovSingleValue(1/(stdnew*stdnew));
}

 void gvfhandler::gvf_rt(int resamplingThreshold)
{
    int rtnew = resamplingThreshold;
    int cNS = mygvf->getNbOfParticles();
    if (rtnew >= cNS)
        rtnew = floor(cNS/2);
    mygvf->setResamplingThreshold(rtnew);
}

void gvfhandler::gvf_adaptspeed(std::vector<float> varianceCoeficients)
{
    mygvf->setAdaptSpeed(varianceCoeficients);
}

void gvfhandler::setNumberOfParticles(int newNs)
{
    mygvf->setNumberOfParticles(newNs);
    gvf_restart();
}

int gvfhandler::getTemplateCount()
{
    return mygvf->getNbOfTemplates();
}

std::vector<std::vector<float> > gvfhandler::get_template_data(int index)
{
    return mygvf->getTemplateByInd(index);
}

std::vector<recognitionInfo> gvfhandler::getRecogInfo()
{
    return recogInfo;
}

recognitionInfo gvfhandler::getRecogInfoOfMostProbable()
{
    return recogInfo[getIndexMostProbable()];
}

int gvfhandler::getIndexMostProbable()
{
    float mostProbable;
    int indexMostProbable;
    int templateCount = getTemplateCount();
    if(recogInfo.size() > 0)
    {
        indexMostProbable = 0;
        mostProbable = recogInfo[indexMostProbable].probability;
        for(int i = 1; i < templateCount; i++)
        {
            if(recogInfo[i].probability > mostProbable)
            {
                indexMostProbable = i;
                mostProbable = recogInfo[indexMostProbable].probability;
            }
        }
        return indexMostProbable;
    }
    return -1;
}

gvfGesture gvfhandler::getRecognisedGestureRepresentation()
{

    int indexMostProbable = getIndexMostProbable();
    
    // if there is a probable gesture...
    if(indexMostProbable > -1)
    {
        std::vector<std::vector<float> > templateData;
        std::vector<std::vector<float> > partialGestureData;
        std::vector<float> gesturePoint;
        recognitionInfo info = recogInfo[indexMostProbable];
    
        // a new gesture is created and by combining the template with the recognition info
        // and estimated gesture can be created.
        gvfGesture g = getTemplateGesture(indexMostProbable);

        // the whole data goes to templateData
        templateData = g.getData();
        int templateSize = templateData.size();
        
        // but only the amount proportinal to the estimated phase will be copied to the estimated gesture
        int amountToBeCopied = templateSize * info.phase;
        
        // scale and rotation are also taken into account
        float estimatedScale = info.scale;
        float rotation = info.rotation;
                
        for(int i = 0; i < amountToBeCopied; i++)
        {
            Eigen::VectorXf vref(2);
            vref[0] = templateData[i][0];
            vref[1] = templateData[i][1];
            
            float alpha = info.rotation;
            Matrix2f rotmat;
            rotmat << cos(alpha), -sin(alpha), sin(alpha), cos(alpha);

            vref = rotmat * vref;
            vref = vref * estimatedScale;
            
            gesturePoint.push_back(vref.x());
            gesturePoint.push_back(vref.y());
            partialGestureData.push_back(gesturePoint);
            gesturePoint.clear();
        }
        gesturePoint.push_back(0);
        gesturePoint.push_back(0);
        
        g.setData(partialGestureData);
        g.centraliseDrawing = true;
        return g;
    }
    return gvfGesture();
}


int gvfhandler::get_state()
{
    return state;
}

void gvfhandler::addTemplateGesture(ofPoint initialPoint, ofColor templateColor)
{
    // hardcoded pixel values determine the position of the templates shown close to the bottom of the screen
    float x, y, w, h;
    float totalWidth = 0.97 * ofGetWindowWidth();
    float intendedWidth = 300; // when the space runs out, the width will be reduced
    float xOffset = 15;
    float YBottomOffset = 15;
    float aspectRatio = 1.33;
    
    int templatesCount = templates.size() + 1;

    if(intendedWidth * templatesCount > totalWidth)
    {
        w = totalWidth / templatesCount;
        h = w / aspectRatio;
        y = ofGetWindowHeight() - (h + YBottomOffset);
        
        for(int i = 0; i < templatesCount - 1; i++)
        {
            x = xOffset + w * i;
            templates[i].setDrawArea(ofRectangle(x, y, w, h));
        }
        x = xOffset + w * (templatesCount - 1);
    }
    else
    {
        w = intendedWidth;
        h = w / aspectRatio;
        y = ofGetWindowHeight() - (h + YBottomOffset);
        x = xOffset + w * (templatesCount - 1);
    }
    
    gvfGesture g = gvfGesture(ofRectangle(x, y, w, h));
    initialPoint.x+= 1;
    initialPoint.y+= 1;
    g.initialise(initialPoint);
    g.centraliseDrawing = true;
    g.setColor(templateColor);
    
    templates.push_back(g);
}

gvfGesture gvfhandler::getTemplateGesture(int index)
{
    gvfGesture g = templates[index];
    g.setData(get_template_data(index));
    return g;
}

void gvfhandler::drawTemplates(float scale)
{
    for(int i = 0; i < templates.size(); i++)
    {
        gvfGesture g = getTemplateGesture(i);
        g.setAppearance(g.getColor(), 1.5, 255, 50, 1);
        g.drawBoundaries = false;
        g.draw(scale);
    }
}

Eigen::VectorXf gvfhandler::getVref()
{
    return mygvf->vref;
}

void gvfhandler::printParticleInfo(gvfGesture currentGesture)
{
    std::vector<std::vector<float> > pp = mygvf->particlesPositions;
    int ppSize = pp.size();
    float scale = 1;

    if(ppSize > 0)
    {

    // as the colors show, the vector returned by getG()
    // does not seem to be in synch with the information returned by particlesPositions
    Eigen::VectorXi gestureIndex = mygvf->getG();
        Eigen::VectorXf weights = mygvf->getW();

        ofFill();
        
        float weightAverage = weights.mean();
        for(int i = 0; i < ppSize; i++)
        {
            gvfGesture g = templates[gestureIndex[i]];
            ofRectangle drawArea = currentGesture.getDrawArea();
            ofPoint initialPoint = currentGesture.getInitialOfPoint() ;

            // each particle position is retrieved
            ofPoint point(pp[i][0], pp[i][1]);

            // and then scaled and translated in order to be drawn
            float x = (((point.x * scale - 0.5) * 2) + 1 + initialPoint.x) * ofGetWindowWidth() + drawArea.x;
            float y = (((point.y * scale - 0.5) * 2) + 1 + initialPoint.y) * ofGetWindowWidth() + drawArea.y;

            // the weight of the particle is normalised
            // and then used as the radius of the circle representing the particle
            float radius = weights[i]/weightAverage;
            ofColor c = g.getColor();

            c.setBrightness(198);
            ofSetColor(c);
            ofCircle(x, y, radius);

        }
    }
}
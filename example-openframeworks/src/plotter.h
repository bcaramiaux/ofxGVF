//
//  plotter.h
//  graphicsExample
//
//  Created by Thomas Rushmore on 18/06/2013.
//
//

#ifndef __graphicsExample__plotter__
#define __graphicsExample__plotter__

#include <iostream>
#include "ofMain.h"
#include <vector>

class Plotter{
public:
    Plotter();
    ~Plotter();
    
    void draw();
    void SetAxis(int s_origin_offset,int s_axis_length,int s_mid_y_axis,int s_scr_wid,int s_scr_hei,int s_grph_div,int s_n_bars);
    void drawgrid();
    
private:
    int counter;
    int origin_offset;
    int axis_length;
    int mid_y_axis;
    int scr_wid;
    int scr_hei;
    int grph_div;
    int n_bars;
    
    ofPoint axisPoints[3];
    std::vector<int> x_bars;
    std::vector<int> y_bars;
    
};
#endif /* defined(__graphicsExample__plotter__) */

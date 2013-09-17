//
//  plotter.cpp
//  graphicsExample
//
//  Created by Thomas Rushmore on 18/06/2013.
//
//

#include "plotter.h"

Plotter::Plotter()
{
    counter = 0;
    ofSetLineWidth(2);
}

Plotter::~Plotter()
{
    
}

void Plotter::draw()
{
    ofSetColor(0);

    // axis
    ofLine(axisPoints[0].x, axisPoints[0].y, axisPoints[1].x, axisPoints[1].y);
    ofLine(axisPoints[0].x, axisPoints[0].y, axisPoints[2].x, axisPoints[2].y);
    drawgrid();

}

void Plotter::drawgrid()
{
    int y_b_co_ord = scr_hei - origin_offset;
    int y_t_co_ord = y_b_co_ord - axis_length;
    ofSetColor(220);
    for(int i = 1 ; i < n_bars; i++)
    {
        ofLine(x_bars[i], y_b_co_ord, x_bars[i], y_t_co_ord);
    }
}

void Plotter::SetAxis(int s_origin_offset,int s_axis_length,int s_mid_y_axis,int s_scr_wid,int s_scr_hei,int s_grph_div,int s_n_bars)
{
    origin_offset = s_origin_offset;
    axis_length   = s_axis_length;
    mid_y_axis    = s_mid_y_axis;
    scr_wid       = s_scr_wid;
    scr_hei       = s_scr_hei;
    grph_div      = s_grph_div;
    n_bars        = s_n_bars;
    
    // orig
    axisPoints[0].x = origin_offset;
    axisPoints[0].y = scr_hei - origin_offset;
    // y axis
    axisPoints[1].x = origin_offset;
    axisPoints[1].y = axisPoints[0].y - axis_length;
    // x axis
    axisPoints[2].x = origin_offset + axis_length;
    axisPoints[2].y = axisPoints[0].y;
    
    int x_co_ord = origin_offset;
    int y_co_ord = scr_hei - origin_offset;
    for(int i = 0 ; i < n_bars; i++)
    {
        x_bars.push_back(x_co_ord);
        x_co_ord += s_grph_div;
        y_bars.push_back(y_co_ord);
        y_co_ord -= s_grph_div;
    }
    
}
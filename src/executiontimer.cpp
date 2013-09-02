//
//  executiontimer.cpp
//  gfpf object
//
//  Created by Thomas Rushmore on 21/05/2013.
//  Copyright (c) 2013 Thomas Rushmore. All rights reserved.
//

#include "executiontimer.h"

double executiontimer::conversion = 0.0;

executiontimer::executiontimer(const char* mes)
{
    alive  = true;
    output = mes;
    output.append(" method execution time : ");
    startTimer();
}

executiontimer::~executiontimer()
{
    if(alive){
        stopTimer();
       // post("%s %" PRIu64"",output.c_str(),diff);
    }
}

void executiontimer::kill()
{
    stopTimer();
  //  post("%s %" PRIu64"",output.c_str(),diff);
    alive = false;
}

void executiontimer::startTimer()
{
    start = mach_absolute_time();
}

void executiontimer::stopTimer()
{
    stop = mach_absolute_time();
    
    // conversion code taken from
    // http://www.macresearch.org/tutorial_performance_and_time
    // does not currently work
    diff = stop - start;
    conversion = 0.0;
    
    if(conversion == 0.0)
    {
        mach_timebase_info_data_t info;
        kern_return_t err = mach_timebase_info( &info );
        if(err == 0){
            conversion = 1e-9 * (double) info.numer / (double) info.denom;
        }
    }
    seconds = conversion * (double)diff;
    reset();

}

double executiontimer::getDiffSeconds()
{
    return seconds;
}

void executiontimer::reset()
{
    start = 0.0;
    stop  = 0.0;
    
}
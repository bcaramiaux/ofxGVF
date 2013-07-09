//
//  executiontimer.h
//  gfpf object
//
//  Created by Thomas Rushmore on 21/05/2013.
//  Copyright (c) 2013 Thomas Rushmore. All rights reserved.
//

#ifndef __gfpf_object__executiontimer__
#define __gfpf_object__executiontimer__

#include <iostream>
#include <stdint.h>
#include <inttypes.h>
#include <mach/mach_time.h>
#include <string>


class executiontimer{
public:
    executiontimer(const char* mes);
    ~executiontimer();
    void startTimer();
    void kill();
    void stopTimer();
    double getDiffSeconds();
private:
    void reset();
    
    uint64_t start;
    uint64_t stop;
    uint64_t diff;
    double seconds;
    static double conversion;
    std::string output;
    bool alive;
};
#endif /* defined(__gfpf_object__executiontimer__) */

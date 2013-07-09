//
//  main.cpp
//  gfpf_tester
//
//  Created by Thomas Rushmore on 09/07/2013.
//  Copyright (c) 2013 Thomas Rushmore. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include "gfpfhandler.h"

int main(int argc, const char * argv[])
{
    if(argc <= 2)
    {
        std::cerr << "Usage : int resamplThreshold, int testGestureIdx " << std::endl;
        return 1;
    }
    int rt = atoi(argv[1]);
    if(rt > 2000){
        std::cerr << "RT must be less than #particles (default:2000) " << std::endl;
        return 1;
    }
    // sim user-input gesture
    int gt = atoi(argv[2]);

    gfpfhandler gfpfhandl(rt,gt);
    gfpfhandl.teach();
    
    printf("Complete.\n");
    return 0;
}


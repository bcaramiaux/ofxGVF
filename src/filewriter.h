//
//  filewriter.h
//  gfpf object
//
//  Created by Thomas Rushmore on 03/07/2013.
//  Copyright (c) 2013 Thomas Rushmore. All rights reserved.
//

#ifndef __gfpf_object__filewriter__
#define __gfpf_object__filewriter__

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

class filewriter{
public:
    filewriter(std::string name,int folder);
    ~filewriter();
    void addValue(float input);
    void writeFile();
    void resetValues();
    int size();
    int scanDirectory();
    
private:
    std::vector<float> values;
    std::string directory;
    std::string parent_directory;
    
};

#endif /* defined(__gfpf_object__filewriter__) */

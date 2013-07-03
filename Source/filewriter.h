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

class filewriter{
public:
    filewriter(std::string name);
    ~filewriter();
    void addValue(float input);
    void writeFile();
private:
    std::vector<float> values;
    std::string directory;
    
};

#endif /* defined(__gfpf_object__filewriter__) */

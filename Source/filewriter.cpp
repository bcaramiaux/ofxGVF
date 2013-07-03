//
//  filewriter.cpp
//  gfpf object
//
//  Created by Thomas Rushmore on 03/07/2013.
//  Copyright (c) 2013 Thomas Rushmore. All rights reserved.
//

#include "filewriter.h"

filewriter::filewriter(std::string name)
{
    directory = ("/Users/thomasrushmore/Desktop/data/");
    directory.append(name);
    directory.append(".txt");
}

filewriter::~filewriter()
{
    
}

void filewriter::addValue(float input)
{
    values.push_back(input);
}

void filewriter::writeFile()
{
    std::ofstream file_write(directory.c_str());
    for(int i=0; i<values.size(); i++)
        file_write << values[i] << " ";
    file_write.close();
    values.clear();
}
//
//  filewriter.cpp
//  gfpf object
//
//  Created by Thomas Rushmore on 03/07/2013.
//  Copyright (c) 2013 Thomas Rushmore. All rights reserved.
//

#include "filewriter.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

filewriter::filewriter(std::string name,int folder)
{
    directory = ("/Users/thomasrushmore/Desktop/data/");
    parent_directory = directory;
    directory.append("run");
    char buf[10];
    sprintf(buf, "%d", folder);
    directory.append(buf);
    directory.append("/");
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
    //values.clear();
}

void filewriter::resetValues()
{
    values.clear();
}

int filewriter::size()
{
    return values.size();
}

int filewriter::scanDirectory()
{
    DIR* direc = opendir(parent_directory.c_str());
    dirent* pdir;
    std::vector<std::string> file_names;
    while((pdir = readdir(direc))){
            std::string tmp_f_name = pdir->d_name;
        if(tmp_f_name.substr(0,3) == "run"){
            file_names.push_back(tmp_f_name);
        }
    }

    closedir(direc);

    int folder_num = (int)file_names.size() + 1;
        
    parent_directory.append("run");
    char buf[10];
    sprintf(buf, "%d", folder_num);
    parent_directory.append(buf);

    if (0 != mkdir(parent_directory.c_str(), 0744)) {
        perror ("mkdir failed");
        exit(1);
    }
    return folder_num;
}
//
//  test_utils.h
//  Test
//
//  Created by Baptiste Caramiaux on 20/05/15.
//  Copyright (c) 2015 Baptiste Caramiaux. All rights reserved.
//

#ifndef Test_test_utils_h
#define Test_test_utils_h

void load_matrix(std::istream* is, std::vector< std::vector<float> >* matrix, const string& delim = " \t")
{
    using namespace std;
    
    string      line;
    string      strnum;
    
    // clear first
    matrix->clear();
    
    // parse line by line
    while (getline(*is, line)) {
        matrix->push_back(vector<float>());
        
        for (string::const_iterator i = line.begin(); i != line.end(); ++ i) {
            // If i is not a delim, then append it to strnum
            if (delim.find(*i) == string::npos) {
                strnum += *i;
                if (i + 1 != line.end()) // If it's the last char, do not continue
                    continue;
            }
            
            // if strnum is still empty, it means the previous char is also a
            // delim (several delims appear together). Ignore this char.
            if (strnum.empty())
                continue;
            
            // If we reach here, we got a number. Convert it to double.
            float       number;
            
            istringstream(strnum) >> number;
            matrix->back().push_back(number);
            
            strnum.clear();
        }
    }
}

#endif

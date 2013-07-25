//
//  globalutilities.h
//  gfpf object
//
//  Created by Thomas Rushmore on 17/05/2013.
//  Copyright (c) 2013 Thomas Rushmore. All rights reserved.
//

#ifndef gfpf_object_globalutilities_h
#define gfpf_object_globalutilities_h

#include <limits>
#include <string>

#include "m_pd.h"

static void postint(int x)
{
    // is this right with pd t_int datatype?
    char buffer[std::numeric_limits<int>::digits];
    char buffer2[std::numeric_limits<float>::is_integer];
    snprintf(buffer, sizeof(buffer), "%d", x);
    post(buffer);
}

static t_float getfloat(const t_atom *a)
{
    return atom_getfloat(const_cast<t_atom*>(a));
}

static t_int getint(const t_atom *a)
{
    return atom_getint(const_cast<t_atom*>(a));
}


// template would make more sense. trouble with format specifier
template <class T>
T postnum(T a){
    char buffer[std::numeric_limits<T>::digits];
    snprintf(buffer, sizeof(buffer), "%d", a);
    post(buffer);
}

#endif

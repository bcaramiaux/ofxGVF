/*

Write MaxMSP (v5) objects in C++

-- general --

Allows the definition of a max object as a C++ class. 

Most of the mechanics of typical max objects are encapsulated by the curiously recurring template pattern. 

note: be careful to only include this header once, to avoid linker errors!
note: this probably won't work with virtuals -- and definitely won't work with multiple inheritance!

-- licence --
maxcpp is distributed under the permissive BSD-like MIT licence:

Copyright (c) 2009-2013 Graham Wakefield

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef MAXMSP_CPP_6_H
#define MAXMSP_CPP_6_H

#include "ext.h"
#include "ext_obex.h"
#include "ext_common.h"
#include "commonsyms.h"
#include "z_dsp.h"

#include <new>

#define MAX_CPP_VERSION	0.600

// for A_NOTHING methods (e.g. bang):
#define REGISTER_METHOD(CLASS, METHOD)	class_addmethod(	\
	(t_class *)CLASS::m_class,								\
	(method)CLASS::MaxMethod<&CLASS::METHOD>::call,			\
	#METHOD,												\
	0);	
// for A_GIMME methods (t_symbol * s, long argc, void * argv):
#define REGISTER_METHOD_GIMME(CLASS, METHOD)	class_addmethod(	\
	(t_class *)CLASS::m_class,								\
	(method)CLASS::MaxMethodGimme<&CLASS::METHOD>::call,	\
	#METHOD,												\
	A_GIMME,												\
	0);	
	
// for A_FLOAT methods (double v):
#define REGISTER_METHOD_FLOAT(CLASS, METHOD)	class_addmethod(	\
	(t_class *)CLASS::m_class,								\
	(method)CLASS::MaxMethodFloat<&CLASS::METHOD>::call,	\
	#METHOD,												\
	A_FLOAT,												\
	0);	
	
// for A_INT methods (long v):
#define REGISTER_METHOD_LONG(CLASS, METHOD)	class_addmethod(	\
	(t_class *)CLASS::m_class,								\
	(method)CLASS::MaxMethodLong<&CLASS::METHOD>::call,	\
	#METHOD,												\
	A_LONG,												\
	0);
	
// for DSP
#define REGISTER_PERFORM(CLASS, METHOD) object_method( \
	dsp64, \
	gensym("dsp_add64"), \
	(t_object *)this, \
	MaxMethodPerform64<&CLASS::METHOD>::call,\
	0, \
	NULL);

// a purely static base class for Max and MSP objects:
template <typename T>
class MaxCppBase {
public:	
	
	static t_class * m_class;
	
	// template functors to forward Max messages to class methods:
	
	typedef void (T::*maxmethodgimme)(long inlet, t_symbol * s, long ac, t_atom * av);
	template<maxmethodgimme F>
	struct MaxMethodGimme {
		static void call(T * x, t_symbol * s, long ac, t_atom * av) { ((x)->*F)(proxy_getinlet((t_object *)x), s, ac, av); }
	};
	
	typedef void (T::*maxmethod)(long inlet);
	template<maxmethod F>
	struct MaxMethod {
		static void call(T * x) { ((x)->*F)(proxy_getinlet((t_object *)x)); }
	};
	
	typedef void (T::*maxmethodlong)(long inlet, long v);
	template<maxmethodlong F>
	struct MaxMethodLong {
		static void call(T * x, long v) { ((x)->*F)(proxy_getinlet((t_object *)x), v); }
	};
	
	typedef void (T::*maxmethodfloat)(long inlet, double v);
	template<maxmethodfloat F>
	struct MaxMethodFloat {
		static void call(T * x, double v) { ((x)->*F)(proxy_getinlet((t_object *)x), v); }
	};

};

// note: only include this file once to prevent linker errors!
template<typename T> t_class * MaxCppBase<T>::m_class = 0;

// inherit from this one for non-audio objects
template <typename T>
class MaxCpp6 : public MaxCppBase<T> {
public:
	t_object m_ob;
	void **	m_outlets;
	void **	m_inletproxies;
	long m_whichinlet;
	
	static t_class * makeMaxClass(const char * classname) {
		common_symbols_init();
		t_class * c = class_new(classname, (method)MaxCpp6<T>::maxcpp_create, (method)MaxCpp6<T>::maxcpp_destroy, sizeof(T), NULL, A_GIMME, 0);
		class_register(CLASS_BOX, c);
		MaxCppBase<T>::m_class = c;
		return c;
	}
	
	static void * maxcpp_create(t_symbol * sym, long ac, t_atom * av) {
		void * x = object_alloc(MaxCppBase<T>::m_class);
		new(x) T(sym, ac, av);
		return (T *)x; 
	}
	
	static void maxcpp_destroy(t_object * x) {
		T * t = (T *)x;
		t->~T();
	}
	
	void setupIO(unsigned int numinlets = 1, unsigned int numoutlets = 1) {
		if (numinlets > 0) {
			unsigned int numproxies = numinlets - 1;
			m_inletproxies = (void **)sysmem_newptr(sizeof(void *) * numproxies);
			for (unsigned int i=1; i<=numproxies; i++)
				m_inletproxies[i] = proxy_new(this, i, &this->m_whichinlet); // generic inlet
		}
		m_outlets = (void **)sysmem_newptr(sizeof(void *) * numoutlets);
		for (unsigned int i=0; i<numoutlets; i++)
			m_outlets[numoutlets - i - 1] = outlet_new(this, NULL); // generic outlet
	}
	
	// C++ operator overload to treat MaxCpp6 objects as t_objects
	operator t_object & () { return m_ob; }
};

// inherit from this one for audio objects
template <typename T>
class MspCpp6 : public MaxCppBase<T> {
public:

	typedef void (T::*maxmethod_perform64)(double **ins, long numins, double **outs, long numouts, long sampleframes);
	template<maxmethod_perform64 F>
	struct MaxMethodPerform64 {
		static void call(T *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) { 
			((x)->*F)(ins, numins, outs, numouts, sampleframes); 
		}
	};

	t_pxobject m_ob; 
	
	
	static t_class * makeMaxClass(const char * classname) {
		common_symbols_init();
		t_class * c = class_new(classname, (method)MspCpp6<T>::maxcpp_create, (method)MspCpp6<T>::maxcpp_destroy, sizeof(T), NULL, A_GIMME, 0);
		class_dspinit(c);
		
		class_addmethod(c, (method)MspCpp6<T>::maxcpp_dsp64, "dsp64", A_CANT, 0);
		
		class_register(CLASS_BOX, c);
		MaxCppBase<T>::m_class = c;
		return c;
	}
	
	static void * maxcpp_create(t_symbol * sym, long ac, t_atom * av) {
		void * x = object_alloc(MaxCppBase<T>::m_class);
		new(x) T(sym, ac, av);
		return (T *)x; 
	}
	
	void setupIO(unsigned int signal_inlets, unsigned int signal_outlets) {
		dsp_setup((t_pxobject *)this, signal_inlets);
		// prevent recycling of inputs for outputs
		m_ob.z_misc = Z_NO_INPLACE;
		for (unsigned int i=0; i < signal_outlets; i++) {
			outlet_new((t_object *)this, "signal");
		}
	}
	
	static void maxcpp_destroy(t_object * x) {
		dsp_free((t_pxobject *)x);
		((T *)x)->~T();
	}
	
	static void maxcpp_dsp64(t_object *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags) {
		((T *)x)->dsp(dsp64, count, samplerate, maxvectorsize, flags);
	}
	
	static void maxcpp_perform64(t_object *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
        ((T *)x)->perform(ins, numins, outs, numouts, sampleframes);
    }
	
	// stub functions in case the user doesn't supply them:
	void dsp(t_object * dsp64, short *count, double samplerate, long maxvectorsize, long flags) {
		REGISTER_PERFORM(T, perform);
	}
	void perform(double **ins, long numins, double **outs, long numouts, long sampleframes) {}
	
	// C++ operator overload to treat MaxCpp6 objects as t_objects
	operator t_object & () { return m_ob; }
};


#endif

GVF - PureData object
===

Sources to compile a PureData object using GVF

Compiling
---

Use the provided Makefile (based on the template available here https://puredata.info/docs/developer/MakefileTemplate). Open a Terminal, write:

```
cd <path>/ofxGVF/puredata
make
```

(tested with Mac OS X 10.11)

A GVF object will be generated in a folder `Build/`, for instance on Mac the generated object is called `gvf.pd_darwin` such as:
```
Build/Darwin/gvf.pd_darwin
```

NOTE to LINUX users: PureData include folder is not always consistent. So in the makefile, you might have change the line providing the path to the header file `m_pd.h`. In this version it's line 182:
```
PD_INCLUDE = $(PD_PATH)/include/pdextended
```

Installing
---

Once built, create a gvf folder in your home Library/pd or pdextended. Copy the gvf object and the `gvf-meta.pd` file provided. Then run Pure Data and open the help patch gvf-help.pd to try.


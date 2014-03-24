This repository contains the source code for the High Speed Forward
Chainer (HSFC) and various extension libraries.

The directories:
    - libhsfc: the base HSFC as maintain by Michael S.
    - cpphsfc: a simple C++ wrapper about libhsfc.
    - pyhsfc: a python module for using the HSFC.
    - testing: contains GDL files for regression testing.

To build the libraries, run: 

   make
   make install

The install will place header and library files in include and lib directories.
Then:
	
   source setup.bash

will setup the various paths to point to these directories.


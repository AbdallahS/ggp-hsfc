This repository contains the source code for the High Speed Forward
Chainer (HSFC).

Not sure if we can convince Michael S to maintain it, but either way
we can use this as a place to stage the working versions of the HSFC
and various add-ons.

I'll (Dave) add some CMake build files so that it is easy to build on
a unix machine. Also will use it to hold any interface code (e.g., a
Python API).

Directory structure:
- libhsfc: main directory for the HSFC packaged as a library.
     - /src: directory containing all the source files as copied
             directly from Michael Schofield's home directory.
- python:
     - /src: interface code for the python API.


NOTE: Currently the files for the HSFC are copied directly into
libhsfc/src. Longer term it would be better if the HSFC was more of a
library with a directory for external API header files in their own
subdirectory and its own namespace. Eg. Should be able to:

    #include <hsfc/API.h>

    using namespace hsfc;


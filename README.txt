This repository contains the source code for the High Speed Forward
Chainer (HSFC).

Not sure if we can convince Michael S to maintain it, but either way
we can use this as a place to stage the working versions of the HSFC
and various add-ons.

I'll (Dave) add some CMake build files so that it is easy to build on
a unix machine. Also will use it to hold any interface code (e.g., a
Python API).

Directory structure (proposed):
- libhsfc: main directory for the HSFC packaged as a library.
     - /src: directory for the source files.
     - /hsfc: directory for the externally callable header files.
- python:
     - /src: interface code for the python API.

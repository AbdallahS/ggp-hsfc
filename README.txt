This repository contains the source code for the High Speed Forward
Chainer (HSFC) and various extension libraries.

PACKAGE LAYOUT
--------------
The subdirectories:
  - libhsfc: the base HSFC as maintain by Michael S.
  - cpphsfc: a simple C++ wrapper about libhsfc.
  - pyhsfc: a python module for using the HSFC.
  - testing: contains GDL files for regression testing.
  - examples: contains examples (both for C++ and Python).


INSTALLATION
------------
To build the libraries, run: 

   make
   make install

By default this will install header and library files in the include
and lib sub-directories of the ggp-HSFC repository. It will also
install a shared objects pyhsfc.so in the directory
lib/python{pyversion}/site-package. Then:
	
   source setup.bash

will setup the various paths (including PYTHONPATH) to point to the
these directories. Note: if you want to install some where other than
the repository relative directories then you will have to run cmake
manually and set the CMAKE_INSTALL_PREFIX appropriately. For example
to install relative to /usr/local:
   
   mkdir build
   cd build
   cmake -DCMAKE_INSTALL_PREFIX=/usr/local ../
   make
   make install


USING THE C++ API
-----------------

The include files are in include/hsfc/hsfc.h and the statically linked
library is in lib/libcpphsfc.a. The main classes:

   - Game: an instance of a game. For each loaded game there is only
           one game instance.
   - State: a representation of a game state. From the Game object the
     initial game state can be returned. From this many variant game
     states can be instantiated, by copying, and making moves in a
     state.
   - Player: a game role/player. 
   - Move: a move (independant of a player). 

For an example of using the API see examples/example1.c++. Note:
important limitations of the current HSFC:

    - Now supports loading a GDL from either a file or a string. To load
      from a file need to use the type boost::filesystem::path.
    - The current HSFC doesn't work with "normal" GDL files. Rather it
      takes a preprocessed GDL with special annotations as produced by
      the Gadelac tool. There is currently an optional flag to first 
      process the file with GaDeLac but nothing is currently implemented.

USING THE PYTHON API
--------------------

The python interface is a thin wrapper over the C++ interface. It
provides the same interface, but in some cases in a more pythonic way.

The basics to load a tictactoe gdl and print the moves from the
initial game state are:

      from pyhsfc import *
      game = Game(file="./tictactoe.gdl")
      initstate = State(game)
      for lpm in initstate.Legals():
             print lpm

Important note: the game constructor uses Python keyword arguments. If the
keyword is "file" then the value is assumed to be a file. If the keyword
is "gdl" then it is assumed that the value is a gdl string. The optional
keyword "gadelac" is currently not implemented.

There is some very basic python docstring documentation which you can
query from python with:
   
      python
      >>> import pyhsfc
      >>> help (pyhsfc)

Finally see testing/example.py for a python version of the C++ example.






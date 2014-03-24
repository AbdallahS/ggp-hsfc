#!/usr/bin/env bash

HSFC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#--------------------------------------------------------------------
# Setup GCC include paths
HSFC_INCLUDE=$HSFC_DIR/include 
export CPLUS_INCLUDE_PATH=$HSFC_INCLUDE:$CPLUS_INCLUDE_PATH
export C_INCLUDE_PATH=$HSFC_INCLUDE:$C_INCLUDE_PATH
export CPATH=$HSFC_INCLUDE:$CPATH

#--------------------------------------------------------------------
# Setup library paths
# NOTE: Difference between LD_LIBRARY_PATH and LIBRARY_PATH:
#       LIBRARY_PATH is used by gcc at compile time. LD_LIBRARY_PATH
#       is used at execution time to load shared libraries.    
for subdir in "lib" "lib64" ; do
    dir=$HSFC_DIR/$subdir
    if [ -d "$dir" ] ; then
	export LD_LIBRARY_PATH=$dir:$LD_LIBRARY_PATH
	export LIBRARY_PATH=$dir:$LIBRARY_PATH
    fi
done

#--------------------------------------------------------------------
# Setup locally installed python modules if present
PYTHON_BASE_VERSION=$(python -V 2>&1 | sed -e s'/Python\s*\([0-9]*\.[0-9]*\)\..*$/\1/')
for subdir in "lib" "lib64" ; do
    dir=$HSFC_DIR/$subdir/python$PYTHON_BASE_VERSION/site-packages
    if [ -d "$dir" ] ; then
	export PYTHONPATH=$dir:$PYTHONPATH
    fi
done

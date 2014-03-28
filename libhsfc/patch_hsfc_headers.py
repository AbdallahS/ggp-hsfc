#!/usr/bin/env python

#---------------------------------------------------------------------------------
# Patch the HSFC header files to:
# 1) change: #include "hsfcXXXX.h" => #include "hsfc/impl/hsfcXXXX.h"
# 2) remove "using namespace" references (esp. "using namespace std;")
# 3) Fully qualify type names. Currently: string, vector
#
#---------------------------------------------------------------------------------

import sys
import re

#----------------------------------------------------
# run
#----------------------------------------------------

def run(infile, outfile):
    regex_usng_nmspc = re.compile(r'using\s+namespace\s+(\w+)\s*;')
    regex_gbl_inclde = re.compile(r'^#include\s+<([^>]+)>')
    regex_lcl_inclde = re.compile(r'^#include\s+"([^"]+)"')
    regex_type_string = re.compile(r'(?<!^\w)?string(?!\w)')
    regex_type_vector = re.compile(r'(?<!^\w)?vector(?!\w)')
    
    for line in infile.readlines():
        if regex_usng_nmspc.match(line):
            continue
        if regex_gbl_inclde.match(line):
            outfile.write(line)
            continue
        mtch = regex_lcl_inclde.match(line)
        if mtch:
            outfile.write("#include <hsfc/impl/{0}>".format(mtch.group(1)))
            continue
        if regex_type_string.search(line):
            line = re.sub(r'(?<!^\w)?string(?!^\w)','std::string', line)
        if regex_type_vector.search(line):
            line = re.sub(r'(?<!^\w)?vector(?!^\w)','std::vector', line)
        outfile.write(line)

#----------------------------------------------------
# main
#----------------------------------------------------

def main():
    if len(sys.argv) != 3:
        print "usage: {0} <infilename> <outfilename>".format(sys.argv[0])
        return
    infile = open(sys.argv[1],'r')
    outfile = open(sys.argv[2], 'w')
    run(infile, outfile);
#    run(sys.stdin, sys.stdout)

if __name__ == '__main__':
    main()

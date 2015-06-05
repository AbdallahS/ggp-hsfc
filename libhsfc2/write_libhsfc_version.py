#!/usr/bin/env python

#---------------------------------------------------------------------------------
# Write out a file with a define for the HSFC version
#---------------------------------------------------------------------------------

import sys

#----------------------------------------------------
# main
#----------------------------------------------------

def main():
    if len(sys.argv) != 3:
        print "usage: {0} <outfilename> <version>".format(sys.argv[0])
        return
    outfile = open(sys.argv[1], 'w')
    version = sys.argv[2]
    outfile.write("/* Automatically generated */\n\n");
    outfile.write("#ifndef HSFC_VERSION\n");
    outfile.write("#define HSFC_VERSION {0}\n".format(version));
    outfile.write("#endif\n");

if __name__ == '__main__':
    main()

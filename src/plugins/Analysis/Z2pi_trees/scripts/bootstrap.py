"""
python script to loop over fits to compute uncertainties using the Bootstrap

The following structure is assumed (although easily modified)

parent directory: Z2pi_trees  
- contains all the root trees and analysis of a single fit

current directory: ../Bootstrap 
- contains this file which performs multiple fits with different bootstrap samples
- fit_2pi_primakoff_W_100000_TEMPLATE.cfg has the template for the configuration file. It is a copy of the configuration in the parent directory, except a) uses the ROOTDataReaderBootstrap reader for the data, b) The template has the RANDOM string, which will be replaced with a sequential number for the TRandom2 seed used by the reader.

subdirectory Bootstrap_fit2
- the results of the fits are written to files in the subdirectory Bootstrap_fit2 into files with extensions fit2. 
- The root script bootstrap.C will read the files with the bootstrap fits to obtain bootstrap errors and produced comparison plots between the bootstrap method and fit outputs.
. the output file twopi_primakoff_DSelect_out.fit2 has the same format as the input files, but contains the fitted values from the 00000 file and errors from the bootstrap method. This file can be used with the root script twopi_primakoff.C in the Z2pi_trees parent directory to generate output similar to that of a single fit but with the bootstrap error in the tables.

"""


import subprocess
import numpy
from matplotlib import pyplot as plt
from datetime import datetime, date, time, timedelta
from matplotlib.backends.backend_pdf import PdfPages
import os
import random
from os.path import isfile, join

filenames = []
runs = []

nevents = 10
# outdir = '/work/halld/home/elton/gen_2pi_primakoff_signal_dec2020/Z2pi_trees/Bootstrap/Bootstrap_fit2'
outdir = 'Bootstrap_fit2'

# Output first file without Bootstrap

# outfile = 'Bootstrap_fit2/fit_2pi_primakoff_W_100000_'+id+'.cfg'
outfile = outdir+'/fit_2pi_primakoff_W_100000.cfg'
print "outfile=",outfile

command = 'rm -f '+outfile
print " command0=",command
os.system(command)

command = 'cp fit_2pi_primakoff_W_100000.cfg '+str(outfile)
print " command1=",command
os.system(command)

command = 'fit -c '+outfile
print "command2=",command
os.system(command)
command = 'cp twopi_primakoff.fit twopi_primakoff_DSelect.fit'
print "command3=",command
os.system(command)
command = 'twopi_plotter_primakoff twopi_primakoff.fit -o twopi_primakoff_DSelect.root'
print "command4=",command
os.system(command)
ifile = 0
command = 'mv twopi_fitPars.txt '+outdir+'/twopi_primakoff_DSelect_'+'{0:05d}'.format(ifile)+'.fit2'
print "command5=",command
os.system(command)

# use Bootstrap for files 1-N

for ifile in range(1,nevents):
     seed = random.randint(20,nevents+20-1)

     print "seed=", seed, '{0} {1:05d}'.format(" test seed=",seed)
     id = '{0:05d}'.format(seed)
     # outfile = 'Bootstrap_fit2/fit_2pi_primakoff_W_100000_'+id+'.cfg'
     outfile = outdir+'/fit_2pi_primakoff_W_100000.cfg'
     print "outfile=",outfile

     command = 'rm -f '+outfile
     print " command0=",command
     os.system(command)

     command = 'sed "s/RANDOM/'+str(seed)+'/" fit_2pi_primakoff_W_100000_TEMPLATE.cfg > '+str(outfile)
     print " command1=",command
     os.system(command)


     command = 'fit -c '+outfile
     print "command2=",command
     os.system(command)
     command = 'cp twopi_primakoff.fit twopi_primakoff_DSelect.fit'
     print "command3=",command
     os.system(command)
     command = 'twopi_plotter_primakoff twopi_primakoff.fit -o twopi_primakoff_DSelect.root'
     print "command4=",command
     os.system(command)
     command = 'mv twopi_fitPars.txt '+outdir+'/twopi_primakoff_DSelect_'+'{0:05d}'.format(ifile)+'.fit2'
     print "command5=",command
     os.system(command)
"""





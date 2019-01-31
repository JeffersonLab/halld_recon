"""
python script to loop over root files in specified folder and process Read_bcal_hadronic_eff.C for each run and layers 1-4
"""


import subprocess
import numpy
from matplotlib import pyplot as plt
from datetime import datetime, date, time, timedelta
from matplotlib.backends.backend_pdf import PdfPages
import os
from os.path import isfile, join

filenames = []
runs = []

# minrunno = 40000
minrunno = 41344
# maxrunno = 41343
maxrunno = 42000
minfiles = 1
maxfiles = 100
basedir = "/cache/halld/offline_monitoring/RunPeriod-2018-01/ver11/tree_bcal_hadronic_eff/merged/"

filelist = sorted(os.listdir(basedir))

for file in filelist:
    if ".root" in file:
        for layer in range(1,5):
            filerun = file.replace("tree_bcal_hadronic_eff_","").replace(".root","")
            if (int(filerun) >= minrunno and int(filerun) <= maxrunno):
                # print "filerun=", filerun
                option = '{0} {1:d}'.format(filerun,layer)
                command = "root -b -q "+basedir+file+" 'call_bcal_hadronic_eff.C(\"Read_bcal_hadronic_eff2.C\",\""+option+"\")'"
                print command
                # os.system(command)









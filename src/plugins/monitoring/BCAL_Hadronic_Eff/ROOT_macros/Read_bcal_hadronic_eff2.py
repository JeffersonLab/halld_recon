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
RunPeriod = "RunPeriod-2019-11"
version = "ver12"
runs = []

minrunno = 71350    # Period 1
# maxrunno = 71463 
# minrunno = 71464      # Period 2
# maxrunno = 71591
# minrunno = 71592      # Period 3
# maxrunno = 71727
# minrunno = 71728      # Period 4
# maxrunno = 71855
# minrunno = 71863      # Period 5
# maxrunno = 71942
# minrunno = 71943      # Period 6
# maxrunno = 72067
# minrunno = 72068      # Period 7
# maxrunno = 72163
# minrunno = 72164      # Period 8
# maxrunno = 72325
# minrunno = 72362      # Period 9
maxrunno = 72435
minfiles = 1
maxfiles = 100
# basedir = '/work/halld/home/elton/BCAL_Hadronic_Eff/'+RunPeriod+'/'+version+'/'
basedir = '/cache/halld/offline_monitoring/'+RunPeriod+'/'+version+'/tree_bcal_hadronic_eff/merged/'

print "basdir=", basedir

filelist = sorted(os.listdir(basedir))
for file in filelist:
    print file

for file in filelist:
    if ".root" in file:
        for layer in range(1,5):
            filerun = file.replace("tree_bcal_hadronic_eff_","").replace(".root","")
            print "filerun=", filerun
            if (int(filerun) >= minrunno and int(filerun) <= maxrunno):
                # print "filerun=", filerun
                option = '{0} {1:d}'.format(filerun,layer)
                command = "root -b -q "+basedir+file+" 'call_bcal_hadronic_eff.C(\"Read_bcal_hadronic_eff2.C\",\""+option+"\")'"
                print command
                os.system(command)









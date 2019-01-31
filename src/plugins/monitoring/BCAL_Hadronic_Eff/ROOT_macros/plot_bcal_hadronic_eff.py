"""
python script to loop over dat/ directory produced by Read_bcal_hadronic_eff2.C and create file with a list of all run numbers
At the moment it assumes that layer1 file exists and all other layer files are present.
"""


import subprocess
import numpy
from matplotlib import pyplot as plt
from datetime import datetime, date, time, timedelta
from matplotlib.backends.backend_pdf import PdfPages
import os
from os.path import isfile, join

runlist = []

minrunno = 40000
maxrunno = 45000
maxruns = 20
minfiles = 1
maxfiles = 100


filelist = sorted(os.listdir("/work/halld/home/elton/BCAL_Hadronic_Eff_2018/prod_ver01/dat"))

for file in filelist:
    if ("layer1" in file and ".dat" in file):
        filerun = file.replace("R","").replace("_layer1_cut3","").replace(".dat","")
        runlist.append(int(filerun))
        print "filerun=",filerun

print "length=",len(runlist)

fout = open("plot_bcal_hadronic_eff.list","w")
fout.write(str(len(runlist))+"\n")
for run in runlist:
    fout.write(str(run)+"\n")

fout.close()










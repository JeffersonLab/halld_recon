"""
The SiPM_saturation table was creates with the following ccdb command in the BCAL folder:
/BCAL> mktbl SiPM_saturation -r 8 END LAYER INTEGRAL_TO_PEAK SIPM_NPIXELS PIXEL_PER_COUNT

ccdb_put_SiPM_saturation.py
python script to fill SiPM saturation parameters

At the moment input selections are all specified in this file. The script could be modified to accept command line arguments.
"""
import os
import subprocess
import numpy
from matplotlib import pyplot as plt
from datetime import datetime, date, time, timedelta
from matplotlib.backends.backend_pdf import PdfPages

bdate = "2015-08-10"
edate = "2015-09-10"
system = "/BCAL"

variation = "default"
run1 = 0      # first run of validity
run2 = 0     # last run of validity. 0 -> infinity
constant = "SiPM_saturation"
xmin = -200
xmax = 200
gain = 1.
column = 0

# initialize variables
end = [0,1]
layer = [1,2,3,4]
integral_to_peak = []
sipm_npixels = []
pixel_per_count = []


# create list of file names
filenames = []
runs = [42433]  
figs = []

# generate file with constants

for (ndx,run) in enumerate(runs):
    filenames.append("ccdbfiles"+system+"-"+constant+"-"+str(run)+".dat")
    command =["ccdb","dump",system+"/"+constant+":"+str(run)]
    print " command=", command
    # print "ndx=",ndx,"file=",filenames[ndx]
    fout = open(filenames[ndx],"w")
    subprocess.call(command,stdout=fout)
    fout.close()

# now read file
"""
for (ndx,file) in enumerate(filenames):
    fin = open(file,"r")
    entries = 0

    for line in fin:
        entries += 1
        linew = line.split()
        if entries == 1 or entries == 2:
            for token in linew:
                print "First line dump, num tokens=",len(linew)," token=",token
            continue   # discard first entry
        integral_to_peak = float(linew[column])/gain
        sipm_npixels = float(linew[column+1])/gain
        pixel_per_count = float(linew[column+2])/gain
    entries -= 1
    print "Number of entries=",entries," for ",fin
    fin.close()

# output new files:

print "Run =",runs[0],", integral_to_peak=",integral_to_peak,", sipm_npixels=",sipm_npixels,", pixel_per_count=",pixel_per_count
"""


# fill constants in table
for jend in end:
    for jlayer in layer:
        integral_to_peak.append(12.8)
        sipm_npixels.append(57600.0*jlayer)
        pixel_per_count.append(0.454)

print "end=",end," layer=",layer," integral_to_peak=",integral_to_peak," sipm_npixels=",sipm_npixels," pixel_per_count=",pixel_per_count


ndx=0
filenames = []

filenames.append("ccdbfiles"+system+"-"+constant+"-"+str(run)+"_updated.dat")
# print "ndx=",ndx,"file=",filenames[ndx]
fout = open(filenames[ndx],"w")
fout.write("# initial values independent of end or layer\n")

for jend in end:
    for jlayer in layer:
        fout.write(str(end[jend])+"\t"+str(layer[jlayer-1])+"\t"+str(integral_to_peak[jend*4+jlayer-1])+"\t"+str(sipm_npixels[jend*4+jlayer-1])+"\t"+str(pixel_per_count[jend*4+jlayer-1])+"\n")

fout.close()

if run2 == 0:
    command =["ccdb","add",system+"/"+constant,"-v ",variation," -r ",str(run1)+"-",filenames[ndx]]
else:
    command =["ccdb","add",system+"/"+constant,"-v ",variation," -r ",str(run1)+"-"+str(run2),filenames[ndx]]

print "command=",command
string = ' '.join(command)
print "string=", string
# subprocess.call(command)

"""
# plt.show()
pdfname = "ccdb_get_"+system+"_"+constant+"_col"+str(column)+".pdf"
print "PDF filename=",pdfname
with PdfPages(pdfname) as pdf:
    for fig in figs:
        pdf.savefig(fig)
"""




# This script expects to find the root files loose inside hists or hist_merged, no extra subdirectories
# eg hists/hd_root_071234.root

import os
from shutil import copyfile
import subprocess
import glob
import ccdb
import re


testing = 1  # just process one file

addfilename = "add_files_to_ccdb.sh"   # gets overwritten

dirs = ['eff_tables']
for d in dirs:
    if not os.path.exists(d):
        os.makedirs(d)

rundir = ""

if os.path.exists("hists"):
  histdir = "hists"
elif os.path.exists("hists_merged"):
  histdir = "hists_merged"
else:
  exit("Cannot find hists or hists_merged")


filelist = subprocess.check_output(["ls", histdir]).splitlines()


addf = open(addfilename,"w")
addf.write("#!/bin/sh\n")


veryfirstrun=1

for file in filelist:

    runfile = histdir + "/" + file
#    runfile = histdir + "/" + rundir + "/" + file

#    if not os.path.isdir(runfile):
#        continue

#    if len(os.listdir(runfile)) == 0:
#            fbad = open("emptydir.txt", "a")
#            fbad.write(runfile)
#            fbad.write("\n")
#            fbad.close()
#            continue

#    newlist = subprocess.check_output(["ls", runfile]).splitlines()

    # parse the filename to extract the run
  
    findrunnum = re.findall('\d+',runfile)

    if len(findrunnum) == 0:
        print 'Cannot find run number in filename'
    else:
        run = int(findrunnum[0])

    scriptname = "list_dead_straws.C"

    # root -l -b -q runfile fit_dedx.C(run)
    subprocess.call(["root", "-l", "-b", "-q", runfile,scriptname])

    # check whether the script completed 

    if not os.path.exists("straw_eff.txt") :
        fbad = open("badfiles.txt", "a")
        fbad.write(runfile)
        fbad.write("\n")
        fbad.close()

        continue    


    newcalib=1

    if veryfirstrun == 1:  #copy the new file into eff_tables, remember its name 
        savecalib=1
    else :                         #compare the new file with the previous one
        diffresult = subprocess.call(["diff", "-q", "straw_eff.txt",calibfile])
        if diffresult == 0:
            savecalib=0
        else:
            savecalib=1
      
    if savecalib == 1:
        if veryfirstrun == 0:
            addf.write('ccdb add /CDC/wire_mc_efficiency -v "mc" -r ' + str(firstrun) + '-' + str(run-1) + ' ' + calibfile + '\n')
        print('saving calib file')

        calibfile='eff_tables/eff_' + str(run) + '.txt'
        copyfile('straw_eff.txt',calibfile)
        firstrun=run

    veryfirstrun=0

  # save the last set
addf.write('ccdb add /CDC/wire_mc_efficiency -v "mc" -r ' + str(firstrun) + '-' + str(run) + ' ' + calibfile + '\n')
addf.close()

if os.path.exists("badfiles.txt") :
   print 'bad files are listed in badfiles.txt'


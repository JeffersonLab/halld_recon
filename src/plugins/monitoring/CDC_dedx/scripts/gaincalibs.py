# This script expects to find the root files loose inside hists or hist_merged, no extra subdirectories
# eg hists/hd_root_071234.root

import os
import subprocess
import glob
import ccdb
import re

testing = 1  # just process one file

calcnewgains = 1 # calculate new gains (slower, pulls old ones from ccdb)

addfilename = "add_gains_to_ccdb.sh"   # gets overwritten

# ccdb contains gain files

dirs = ['mip', 'oldgains', 'newgains']
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

filesdone = 0


if calcnewgains:
    addf = open(addfilename,"w")
    addf.write("#!/bin/sh\n")


for file in filelist:

    print(file)

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

    run = 999

    #parse the filename to extract the run
    # hd_calib_verify_Run071350_001.root

 #   run = int(re.findall('\d+',newlist[0])[0])
    findrunnum = re.findall('\d+',runfile)

    if len(findrunnum) == 0:
        print 'Cannot find run number in filename'
    else:
        run = int(findrunnum[0])

    print 'run number ',run

#    runfile = runfile + "/" + newlist[0]

    scriptname = "fit_dedx.C(" + str(run) + ")"


    # look to see how many calib files have been made already
    nfiles = len(os.listdir("mip"))
    #print(nfiles)

    # root -l -b -q runfile fit_dedx.C(run)
    subprocess.call(["root", "-l", "-b", "-q", runfile,scriptname])

    # check whether the script completed & made a new file of calib consts

    #print(len(os.listdir("mip")))

    if len(os.listdir("mip")) == nfiles:
          fbad = open("badfiles.txt", "a")
          fbad.write(runfile)
          fbad.write("\n")
          fbad.close()

          continue    

    filesdone = filesdone + 1

    dedxfile = "mip/run_" + str(run) + ".txt" 
    dedxf = open(dedxfile)
    dedx = float(dedxf.readline())
    dedxf.close()

    print "run ",run,"dedx",dedx


    if calcnewgains:

            oldgainfile = "oldgains/run_" + str(run) + ".txt" 

            print " running ccdb cat CDC/digiscales -r ",str(run),">",oldgainfile

            subprocess.call(["ccdb", "cat", "CDC/digi_scales", "-r", str(run), ">", oldgainfile])

            oldf = open(oldgainfile)

            oldf.readline()
            oldf.readline()
            oldf.readline()
            oldf.readline()
            line = oldf.readline()
            oldf.close()

            oldgain = float(line.split()[1])

            print "old gain ", oldgain


            newdedx = 2.01992   #   // put the 1.5GeV/c cut at this dE/dx

            newgain = newdedx*oldgain/dedx

            print "newgain is ",newgain

            newgainfile = "newgains/run_"+str(run)+".txt"

            newf = open(newgainfile,"w")
            newf.write("%.3f 0.8\n"%newgain)
            newf.close()

            addf.write("ccdb add /CDC/digi_scales -r "+str(run)+"-"+str(run)+" "+newgainfile+"\n")


            if testing == 1:
               addf.close()

    
    if testing == 1:
        break

if calcnewgains:
    addf.close()

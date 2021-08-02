import os
import subprocess
import glob

testing = 0  # just process one file


dirs = ['Before', 'After', 'Combined', 'ccdb', 'Monitoring', 'Proj', 'ResVsT']
for d in dirs:
    if not os.path.exists(d):
        os.makedirs(d)

#rundirlist = subprocess.check_output(["ls", "hists_merged"]).splitlines()
#for rundir in rundirlist:

#    print(rundir)
    rundir = ""

#filelist = subprocess.check_output(["ls"]).splitlines()

if os.path.exists("hists"):
  histdir = "hists"
elif os.path.exists("hists_merged"):
  histdir = "hists_merged"
else:
  exit("Cannot find hists or hists_merged")



#filelist = subprocess.check_output(["ls", "hists_merged/" + rundir]).splitlines()
filelist = subprocess.check_output(["ls", histdir]).splitlines()

filesdone = 0

for file in filelist:

        print(file)

        runfile = histdir + "/" + rundir + "/" + file


        if os.path.isdir(runfile):

          if len(os.listdir(runfile)) == 0:
            fbad = open("emptydir.txt", "a")
            fbad.write(runfile)
            fbad.write("\n")
            fbad.close()
            continue

          newlist = subprocess.check_output(["ls", runfile]).splitlines()

          runfile = runfile + "/" + newlist[0]


        print(runfile)

        scriptname = "ttodfit.C(1,1)"

        print(scriptname)

        # look to see how many calib files have been made already
        nfiles = len(os.listdir("ccdb"))
        print(nfiles)

        # root -l -b -q FitTimeToDistance.C(runfile)"
#        subprocess.call(["root", "-l", "-b", "-q", scriptname])
        subprocess.call(["root", "-l", "-b", "-q", runfile, scriptname])

        # check whether the script completed & made a new file of calib consts

        print(len(os.listdir("ccdb")))

        if len(os.listdir("ccdb")) == nfiles:
          fbad = open("badfiles.txt", "a")
          fbad.write(runfile)
          fbad.write("\n")
          fbad.close()

        filesdone = filesdone + 1

        if testing == 1:
    
           break

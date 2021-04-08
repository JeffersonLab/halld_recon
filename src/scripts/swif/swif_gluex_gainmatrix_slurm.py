#!/usr/bin/env python

##########################################################################################################################
#
# 2015/07/24 Paul Mattione
# Heavily based off of work by Kei Moriya at:
# https://halldsvn.jlab.org/repos/trunk/scripts/monitoring/hdswif/hdswif.py
#
# SWIF DOCUMENTATION:
# https://scicomp.jlab.org/docs/swif
# https://scicomp.jlab.org/docs/swif-cli
# https://scicomp.jlab.org/help/swif/add-job.txt #consider phase!
#
# Nathaly Santiesteban April 8 2021
# Change the DATA_SOURCE_BASE_DIR, RUN_PERIOD, VERSION, DATA_SOURCE_TYPE, DATA_OUTPUT_BASE_DIR
# SCRIPTFILE ENVFILE CONFIG_FILE_PATH CONFIG_FILE_PATH TREE_NAMES CONFIG_DICT["PLUGINS"]
# Example with the BCAL calibration plugin, modify to the plugin you want to run  
##########################################################################################################################
from optparse import OptionParser
import os.path
import os
import sys
import re
import subprocess
import glob
from subprocess import Popen, PIPE
import rcdb
import time
#################################################### GLOBAL VARIABLES ####################################################

# DEBUG
VERBOSE    = False

# PROJECT INFO
PROJECT    = "gluex"          # http://scicomp.jlab.org/scicomp/#/projects
TRACK      = "analysis"		   # https://scicomp.jlab.org/docs/batch_job_tracks

# RESOURCES
NCORES     = "4"               # Number of CPU cores
DISK       = "4GB"            # Max Disk usage
RAM        = "3GB"            # Max RAM usage
TIMELIMIT  = "90minutes"      # Max walltime
OS         = "centos77"        # Specify CentOS65 machines

# SOURCE DATA INFORMATION
# FILES ARE SEARCHED-FOR WITH THE PATH: DATA_SOURCE_BASE_DIR + "/RunPeriod-" + RUN_PERIOD + "/" + VERSION + "/REST/" + FORMATTED_RUN + "/dana_rest_*.hddm"
# Where FORMATTED_RUN is the run number you chose, with leading zeros. 
# /mss/halld/RunPeriod-2018-08/calib/ver01/BCAL_pi0/
# /cache/halld/RunPeriod-2019-01/calib/ver01/BCAL_pi0/
RUN_PERIOD            = "2019-11"
VERSION               = "calib/ver01/BCAL_pi0"
DATA_SOURCE_TYPE      = "mss" #"mss" for tape files, "file" for disk files
DATA_SOURCE_BASE_DIR  = "/mss/halld"
#DATA_SOURCE_TYPE      = "mss" #"mss" or "file"
#DATA_SOURCE_BASE_DIR  = "/cache/halld/offline_monitoring/"

# OUTPUT DATA LOCATION
DATA_OUTPUT_BASE_DIR    = "/work/halld2/home/nathaly/calibrations/BCAL/"

# JOB EXECUTION
#SCRIPTFILE        = "/work/halld2/home/sdobbs/jobs/analysis/script.sh"
SCRIPTFILE        = "/work/halld2/home/nathaly/calibrations/BCAL/scriptGlueX.sh" # location of the script file
ENVFILE           = "/work/halld2/home/nathaly/calibrations/BCAL/env.csh" # location of .cshrc
CONFIG_FILE_PATH  = "/work/halld2/home/nathaly/calibrations/BCAL/analysis.conf" # location of confid file for hd_root
TREE_NAMES        = "gainmatrix"

# CONFIG FILE CONTENTS
CONFIG_DICT = {}
CONFIG_DICT["PLUGINS"] =            "gainmatrixGluex"
CONFIG_DICT["NTHREADS"] =           "4" #Ncores if you have the whole node
CONFIG_DICT["THREAD_TIMEOUT"] =     "300"
CONFIG_DICT["JANA_CALIB_CONTEXT"] = '"variation=default"' #'"variation=,calibtime="'
CONFIG_DICT["OUTDIR_SMALL"] = DATA_OUTPUT_BASE_DIR
#CONFIG_DICT["EVENTS_TO_KEEP"] = "600000"

####################################################### TRY COMMAND ######################################################

def try_command(command, sleeptime = 5):
	# Try an os command and if the exit code is non-zero then return an error
	return_code = -999
	while return_code != 0:
		process = Popen(command.split(), stdout=PIPE)
		output = process.communicate()[0] # is stdout. [1] is stderr
		print output
		return_code = process.returncode

		if return_code == 0:
			break #successful: leave

		# sleep for a few seconds between tries
		print 'sleeping for ' + str(sleeptime) + ' sec...'
		time.sleep(sleeptime)

################################################## GENERATE CONFIG FILE ##################################################

def generate_config():
	config_file = open(CONFIG_FILE_PATH, 'w')
	config_file.truncate() #wipe it out

	for key in CONFIG_DICT:
		value = CONFIG_DICT[key]
		config_file.write(key + " " + value + "\n")

	config_file.close()

####################################################### FIND FILES #######################################################

def find_files(DATA_SOURCE_DIR):

	# CHANGE TO THE DIRECTORY CONTAINING THE INPUT FILES
	current_dir = os.getcwd()
        
	if not os.path.isdir(DATA_SOURCE_DIR):
                return []
	os.chdir(DATA_SOURCE_DIR)
	#print DATA_SOURCE_DIR
	# SEARCH FOR THE FILES
	file_signature = "*.evio"
#	file_signature = "exclusivepi0_*.evio"
	file_list = glob.glob(file_signature)
	#print "filelist is" + str(file_list)
	if(VERBOSE == True):
		print "size of file_list is " + str(len(file_list))

	# CHANGE BACK TO THE PREVIOUS DIRECTORY
	os.chdir(current_dir)
	return file_list

######################################################## ADD JOB #########################################################

def add_job(WORKFLOW, DATA_SOURCE_DIR, FILENAME, RUNNO, FILENO):
	# PREPARE NAMES
	STUBNAME = RUNNO + "_" + FILENO
	JOBNAME = WORKFLOW + "_" + STUBNAME
       # SETUP LOG DIRECTORY FOR SLURM
        if(FILENO != "-1"):
		LOG_DIR = CONFIG_DICT["OUTDIR_SMALL"] + "/log/" + RUNNO
	else:
		LOG_DIR = CONFIG_DICT["OUTDIR_SMALL"] + "/log"
        make_log_dir = "mkdir -p " + LOG_DIR
        try_command(make_log_dir)
        if(VERBOSE == True):
                print "LOG DIRECTORY " + LOG_DIR + " CREATED"


	# CREATE ADD-JOB COMMAND
	# job
	add_command = "swif add-job -workflow " + WORKFLOW + " -name " + JOBNAME
	# project/track
	add_command += " -project " + PROJECT + " -track " + TRACK
	# resources
	add_command += " -cores " + NCORES + " -disk " + DISK + " -ram " + RAM + " -time " + TIMELIMIT + " -os " + OS
	# inputs
	add_command += " -input " + FILENAME + " " + DATA_SOURCE_TYPE + ":" + DATA_SOURCE_DIR + "/" + FILENAME
	# stdout
	add_command += " -stdout " + DATA_OUTPUT_BASE_DIR + "/log/" + RUNNO + "/stdout." + STUBNAME + ".out"
	# stderr
	add_command += " -stderr " + DATA_OUTPUT_BASE_DIR + "/log/" + RUNNO + "/stderr." + STUBNAME + ".err"
	# tags
	add_command += " -tag run_number " + RUNNO
	# tags
	add_command += " -tag file_number " + FILENO
	# command
	add_command += " " + SCRIPTFILE + " " + ENVFILE + " " + FILENAME + " " + CONFIG_FILE_PATH + " " + DATA_OUTPUT_BASE_DIR + " " + RUNNO + " " + FILENO + " " + TREE_NAMES

	if(VERBOSE == True):
		print "job add command is \n" + str(add_command)

	# ADD JOB
	status = subprocess.call(add_command.split(" "))


########################################################## MAIN ##########################################################
	
def main(argv):
	parser_usage = "swif_g13data.py workflow minrun maxrun"
	parser = OptionParser(usage = parser_usage)
	(options, args) = parser.parse_args(argv)

	if(len(args) != 3):
		parser.print_help()
		return

	# GET ARGUMENTS
	WORKFLOW = args[0]
	MINRUN = int(args[1])
	MAXRUN = int(args[2])

	# CREATE WORKFLOW
	status = subprocess.call(["swif", "create", "-workflow", WORKFLOW])

	# GENERATE CONFIG
	generate_config()

	# FIND/ADD JOBS
	for RUN in range(MINRUN, MAXRUN + 1):

		# Format run and file numbers
		FORMATTED_RUN = "Run%06d" % RUN
#		FORMATTED_RUN = "%06d" % RUN

		# Find files for run number
		DATA_SOURCE_DIR = DATA_SOURCE_BASE_DIR + "/RunPeriod-" + RUN_PERIOD + "/" + VERSION + "/" + FORMATTED_RUN
		#DATA_SOURCE_DIR = DATA_SOURCE_BASE_DIR + "/RunPeriod-" + RUN_PERIOD + "/rawdata/" + FORMATTED_RUN
		if(os.path.exists(DATA_SOURCE_DIR)):
			file_list = find_files(DATA_SOURCE_DIR)
		else:
			continue
		if(len(file_list) == 0):
			continue

		# Add jobs to workflow
		for FILENAME in file_list:
			FILENO = FILENAME[-20:-17] #e.g. dana_rest_003185_015.hddm #Cheat!
			add_job(WORKFLOW, DATA_SOURCE_DIR, FILENAME, FORMATTED_RUN, FILENO)
			#print FILENAME
if __name__ == "__main__":
   main(sys.argv[1:])


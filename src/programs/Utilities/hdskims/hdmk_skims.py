#!/usr/bin/env python
#
#  Make trigger skim files from input raw data file.
#
# This will run the hdskims program on a given 20GB raw data file
# writing out blocks of events which contain a FP trigger event.
# A second pass over the produced file using hd_ana with the 
# trigger_skims plugin will produce the standard set of skim files.
# 
# This script takes 2 arguments:
#
#  arg 1: input EVIO file
#  arg 2: output directory for skim files
#
# This will create a temporary directory inside of the output
# directory for working. Once the files are created, they will
# be moved to the output directory and the temporary directory
# will be deleted.
#
# This was developed for the online systems in order to do produce
# skims in the counting house.
#

import os
import sys
import glob
import shutil
import subprocess

# This must be run from gluon computer to write to gluondb1
MYSQL_HOST = 'gluondb1'
MYSQL_DB   = 'HOSS'
MYSQL_USER = 'hoss'
MYSQL_PASSWORD = ''

DELETE_INPUT_FILE = False
RUN_HD_ANA       = True
KEEP_INTERMEDIATE_FILES = False
WRITE_TO_DB = True
FP_IGNORE = []

PLUGINS = ['evio_writer', 'trigger_skims', 'ps_skim', 'TOF_calib']

# Parse command line args
args = []
for arg in sys.argv[1:]:
	if arg == '-d' : DELETE_INPUT_FILE=True
	if arg == '-c' : RUN_HD_ANA=False
	if arg == '-k' : KEEP_INTERMEDIATE_FILES = True
	if arg == '-D' : WRITE_TO_DB = False
	if arg.startswith('-ignore_fp=') : FP_IGNORE = arg.split('-ignore_fp=')[1].split(',')
	if not arg.startswith('-'): args.append(arg)
	
if len(args) != 2:
	print('\nUsage:\n\n   hdmk_skims.py [options] input.evio outputdir\n')
	print('Run hdskims to extract blocks containing a FP triggered event')
	print('and then run hd_root on that with the trigger_skims plugin')
	print('to generate the skim files.')
	print('')
	print(' options:')
	print('     -d   delete the input file when done with it')
	print('     -D   do not write trigger info to DB (default is to write)')
	print('     -c   count only. This will run hdskims but not hd_root')
	print('          so the final skim files won\'t be produced.')
	print('          The trigger counts will be entered into the DB though.')
	print('     -k   Keep intermediate files (for debugging only)')
	print('     -ignore_fp=bits   tell hdskims not to consider any of the')
	print('                       bits specified in the comma separated')
	print('                       list of triggers "bits" when deciding')
	print('                       whether to write out a block')
	print('')
	sys.exit(-1)

infilename = args[0]
outdir     = args[1]
cwd        = os.getcwd()

# If infilename is local then pre-pend cwd so we can access it from
# working directory we create,
if not infilename.startswith('/') : infilename = cwd + '/' + infilename
if not outdir.startswith('/')     : outdir     = cwd + '/' + outdir

workdir = outdir + '/hdskim_work_' + str(os.getpid())

print('--- hdmk_skims.py ----------------')
print(' Inputfile: ' + infilename)
print('Output dir: ' + outdir)
print('  Work dir: ' + workdir)
print('')

# Create workdir and cd to it
print('hdmk_skims.py: Creating working directory: ' + workdir)
os.makedirs(workdir)
os.chdir(workdir)

print('\nhdmk_skims.py: Running hdskims ...')
cmd = ['hdskims', "--sql", infilename]
for trig in FP_IGNORE: cmd.extend(['-fp', str(trig)])
print('hdmk_skims.py: cmd: ' + ' '.join(cmd))
ret = subprocess.call( cmd )

# The skim plugins base their output filenames on input filenames.
# Create a symbolic link with the same name as the input file
# pointing to the block skim file so we can trick the
# plugins into producing the correct names.
# n.b. since we are in our own working directory the original
# input file will not exist here.
infilename_base = os.path.basename(infilename)
block_skim_files = glob.glob('*_skims.evio')
os.symlink( block_skim_files[0], infilename_base )

# Make list of plugins into single string of comma separated values
PLUGINS = ','.join(PLUGINS)

if RUN_HD_ANA:
	print('\nhdmk_skims.py: Running hd_ana ...')
	cmd = ['hd_ana', '-PPLUGINS=' + PLUGINS, '-PNTHREADS=18', '-PEVIO:NTHREADS=28'] + [infilename_base]
	print('hdmk_skims.py: cmd: ' + ' '.join(cmd))
	ret = subprocess.call( cmd )

# Rename the hd_root.root file to hd_root_tofcalib_RUN_FILE.root
nums = infilename_base.split('_')
if len(nums) >= 2:
	RUN  = int(nums[-2])
	FILE = int(nums[-1].split('.')[0])
	tof_fname = 'hd_root_tofcalib_%06d_%03d.root' % (RUN, FILE)
	if os.path.exists( 'hd_root_tofcalib.root' ):
		os.rename( 'hd_root_tofcalib.root', tof_fname )

# Update skiminfo DB with any .sql files found in working directory
sqlfiles = glob.glob('*.sql')
if (len(sqlfiles)>0) and WRITE_TO_DB:

	# We need to import mysql.connector but the RCDB version is
	# not compatible with what is installed on the gluons. Thus,
	# we need to make sure it is not in our path
	mypath = list(sys.path)  # loop over copy so we can change real list within loop
	for p in mypath:
		if '/rcdb/' in p: sys.path.remove(p)
	import mysql.connector
	mydb = mysql.connector.connect( host=MYSQL_HOST, user=MYSQL_USER, password=MYSQL_PASSWORD, database=MYSQL_DB)
	mycursor = mydb.cursor()
	for sqlfile in sqlfiles:
		with open(sqlfile) as f :
			print('\nhdmk_skims.py: Updating skiminfo DB from ' + sqlfile + ' ...')
			for line in f.readlines(): mycursor.execute( line )
	mycursor.close()
	mydb.commit();

print('\nhdmk_skims.py: Removing input file and intermediate files ...')
if DELETE_INPUT_FILE: os.unlink( infilename ) # Remove input file
if not KEEP_INTERMEDIATE_FILES:
	os.unlink( infilename_base )               # Remove symlink
	for f in block_skim_files: os.unlink( f )  # Remove block skim file
	for f in sqlfiles        : os.unlink( f )  # Remove sql file

print('\nhdmk_skims.py: Moving evio skim files ...')
for srcfile in glob.glob('*.evio') + glob.glob('*.root'):
	destfile = outdir + '/' + srcfile
	print('   ' + srcfile + ' -> ' + destfile)
	os.rename( srcfile, destfile )

if not KEEP_INTERMEDIATE_FILES:
	print('hdmk_skims.py: Removing working directory: ' + workdir)
	os.chdir(cwd)
	shutil.rmtree(workdir, True)




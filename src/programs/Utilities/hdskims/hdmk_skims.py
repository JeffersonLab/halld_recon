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

MYSQL_HOST = 'hallddb'
MYSQL_DB   = 'test'
MYSQL_USER = 'test'
MYSQL_PASSWORD = ''

if len(sys.argv) != 3:
	print('\nUsage:\n\n   hdmk_skims.py input.evio outputdir\n')
	print('Run hdskims to extract blocks containing a FP triggered event')
	print('and then run hd_root on that with the trigger_skims plugin')
	print('to generate the skim files')
	sys.exit(-1)

infilename = sys.argv[1]
outdir     = sys.argv[2]
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
os.mkdir(workdir)
os.chdir(workdir)

print('\nhdmk_skims.py: Running hdskims ...')
cmd = ['hdskims', "--sql", infilename]
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

print('\nhdmk_skims.py: Running hd_root ...')
cmd = ['hd_ana', '-PPLUGINS=evio_writer,trigger_skims,ps_skim', '-PNTHREADS=18', '-PEVIO:NTHREADS=28'] + [infilename_base]
print('hdmk_skims.py: cmd: ' + ' '.join(cmd))
ret = subprocess.call( cmd )

# Update skininfo DB with any .sql files found in working directory
sqlfiles = glob.glob('*.sql')
if len(sqlfiles) > 0:
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
os.unlink( infilename )
os.unlink( infilename_base )
for f in block_skim_files: os.unlink( f )  # Remove intermediate files
for f in sqlfiles        : os.unlink( f )  # Remove intermediate files

print('\nhdmk_skims.py: Moving output files ...')
for srcfile in glob.glob('*.evio'):
	destfile = outdir + '/' + srcfile
	print('   ' + srcfile + ' -> ' + destfile)
	os.rename( srcfile, destfile )

print('hdmk_skims.py: Removing working directory: ' + workdir)
os.chdir(cwd)
shutil.rmtree(workdir, True)




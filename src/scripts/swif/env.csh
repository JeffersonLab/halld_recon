#ident  "@(#).cshrc     ver 1.0     Aug 20, 1996"
# Default user .cshrc file.
#
# This file is executed each time a shell is started.
# This includes the execution of shell scripts.


#####
# Source the site-wide syscshrc file.
# The syscshrc file defines some needed aliases (setup amd unsetup)
# and environment variables (PATH and MANPATH).  This line
# should not be deleted.  You do, however, have a choice of
# syscshrc files.  Uncomment the one that you prefer.
#####
source /site/env/syscshrc       # Searches /usr/local/bin first.
#source /site/env/syscshrc.alt   # Searches /usr/local/bin last.


#####
# Set up the shell environment.  You may comment/uncomment
# the following entries to meet your needs.
#####
# Number of commands to save in history list.
set history=50
 
# Number of commands to save in ~/.history upon logout.
set savehist=50

# Notify user of completed jobs right away, instead of waiting
# for the next prompt.
#set notify

# Don't redirect output to an existing file.
# CAD NOTE!  This must be commented out for proper ME10 functionality!!
set noclobber

# Set the file creation mode mask (default permissions for newly created files).
umask 022


#####
# Define your aliases.
#####
#alias       h       history
#alias       d       dirs
#alias       pd      pushd
#alias       pd2     pushd +2
#alias       po      popd
#alias       m       more
#alias       ls      'ls -F'


#####
# Define your default printer.
#####


#####
# User specific additions should be added below.
#####


setenv BUILD_SCRIPTS /group/halld/Software/build_scripts
setenv HALLD_MY /home/ssanties/halld_my
source $BUILD_SCRIPTS/gluex_env_jlab.csh
setenv CCDB_CONNECTION sqlite:////home/ssanties/check_spring_2019.sqlite
setenv CCDB_CONNECTION mysql://ccdb_user@hallddb.jlab.org/ccdb
setenv JANA_CALIB_URL mysql://ccdb_user@hallddb.jlab.org/ccdb
setenv JANA_CALIB_CONTEXT "variation=default"


#!/bin/tcsh
 
# This file was generated by the script "mk_setenv.csh"
#
# Generation date: Mon Sep 26 11:17:41 EDT 2011
# User: gluex
# Host: ifarm1102
# Platform: Linux ifarm1102 2.6.18-128.7.1.el5 #1 SMP Mon Aug 24 08:21:56 EDT 2009 x86_64 x86_64 x86_64 GNU/Linux
# BMS_OSNAME: Linux_CentOS5-x86_64-gcc4.1.2
 
if ( ! $?BMS_OSNAME ) then
   setenv BMS_OSNAME `/group/halld/Software/scripts/osrelease.pl`
endif
 
if ( -e /group/halld/Software/builds/sim-recon/sim-recon-2011-09-23/setenv_${BMS_OSNAME}.csh ) then
    # Try prepending path of cwd used in generating this file
    source /group/halld/Software/builds/sim-recon/sim-recon-2011-09-23/setenv_${BMS_OSNAME}.csh
else if ( -e setenv_${BMS_OSNAME}.csh ) then
    source setenv_${BMS_OSNAME}.csh
endif 
 

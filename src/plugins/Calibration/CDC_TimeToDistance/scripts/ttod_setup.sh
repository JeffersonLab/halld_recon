#!/bin/sh
#
# Script to set up links for CDC_TimeToDistance calibration constant generation
# in current directory
#
# Arg 1 should be the location of the TimeToDistance histograms
#
# eg calibfiledir="/cache/halld/RunPeriod-2018-08/calib/ver01/hists"


if [ $# -ne 1 ]; then
  echo
  echo Usage: ttod_setup.sh calibfiledir
  echo
  echo eg ttod_setup.sh /cache/halld/RunPeriod-2018-08/calib/ver01/hists
  echo
fi

calibfiledir=$1


if [ ! -d $1 ]; then
  echo Cannot find dir $calibfiledir
  exit
fi

echo Linking subdir hists to $1 
`ln -s $1 hists`
echo



cp -iv $HALLD_RECON_HOME/src/plugins/Calibration/CDC_TimeToDistance/scripts/*.py .
cp -iv $HALLD_RECON_HOME/src/plugins/Calibration/CDC_TimeToDistance/scripts/ttodfit.C .




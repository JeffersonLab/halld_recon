#! /usr/bin/python
# coding:utf-8

import os
import sys
import rcdb
import ROOT
import numpy
import tempfile
import subprocess


def main():
  args = sys.argv
  if len(args) != 3:
    print('usage: ./amo.py MINRUN MAXRUN')
    exit(0)

  MINRUN=int(args[1])
  MAXRUN=int(args[2])

  db = rcdb.RCDBProvider(os.environ.get('RCDB_CONNECTION'))
  amo_runs = db.select_runs("@is_dirc_production and status!=0 and event_count > 10000000 and radiator_type==\"4.5x10-4 Al 40um\"", MINRUN, MAXRUN)

  # Create an np array to store the amo runs
  amo_runs_int = numpy.array([])

  for RUN in range(MINRUN, MAXRUN+1):

    # See if is amo run
    rcdb_run_info = db.get_run(int(RUN))
    if(rcdb_run_info not in amo_runs):
      continue

    runnum = int("%06d" % RUN)
    amo_runs_int = numpy.append(amo_runs_int, runnum)

 
  # Create an np array to store the good amo runs
  amo_runs_good = numpy.array([])
  # and a list with file names for their ccdb entries
  tmp_list = []

  for i in range(len(amo_runs_int)):
    runnum = int(amo_runs_int[i])
    
    try:
      file = ROOT.TFile.Open("/work/halld/online_monitoring/root/hdmon_online%d.root" % (runnum))
      # for older run periods:
      #file = ROOT.TFile.Open("/work/halld/data_monitoring/RunPeriod-2017-01/mon_ver62/rootfiles/hd_root_0%d.root" % (runnum))
      if not file or file.IsZombie():
        print("RootSpy file not found!")
        continue
    except OSError as e:
      print(e)
      continue

    beam_energy = file.Get("rootspy/highlevel/BeamEnergy")
    # for older run periods:
    #beam_energy = file.Get("highlevel/BeamEnergy")
    if not beam_energy:
      print("Beam energy histogram not found!")
      continue

    # Get the number of bins
    num_bins = beam_energy.GetNbinsX()

    # Create an array to store the entries
    entries = numpy.zeros(num_bins, dtype=int)

    # Loop over the bins and save the entries
    for i in range(1, num_bins + 1):
      entries[i - 1] = int(beam_energy.GetBinContent(i))

    # Close the ROOT file
    file.Close()

    # Create a temporary text file
    with tempfile.NamedTemporaryFile(delete=False, mode='w') as temp_file:
      # Write the entries to the file in one line
      temp_file.write(' '.join(map(str, entries)))

    amo_runs_good = numpy.append(amo_runs_good, runnum)
    tmp_list.append(temp_file.name)


  # Loop over all runs that have a good histogram
  for i in range(len(amo_runs_good)):
    runnum = int(amo_runs_good[i])
    next = int(amo_runs_good[i + 1] - 1) if i + 1 < len(amo_runs_good) else int(MAXRUN)
    
    subprocess.call(['ccdb', 'add', '/PHOTON_BEAM/amo_norm', '-r', '%s-%s' % (runnum,next), tmp_list[i]])
    subprocess.call(['rm', tmp_list[i]])


if __name__ == '__main__':
  main()

#! /usr/bin/python
# coding:utf-8

import sys
import os
import glob
import subprocess


def main():
  args = sys.argv
  if len(args) != 3:
    print('usage: ./swif2.py input_parameter_file run-list-file')
    exit(0)

  input_par_file = os.path.abspath(args[1])
  run_list_file = args[2]

  par = get_par(input_par_file)

  swif_out_dir = par['path_to_swif2_output_dir'].rstrip('/') + '/'
  script_dir = os.path.dirname(os.path.abspath(args[0])).rstrip('/') + '/'

  with open(run_list_file) as f:
    run_list0 = f.readlines()
  run_list = [int(x.strip()) for x in run_list0]

  work_flow = 'FDC_t0_' + run_list_file.split('/')[-1].replace('.', '_')


  #################################
  # EXPERT ONLY (BELOW THIS LINE) #
  #################################
  subprocess.call(['swif2', 'create', work_flow])
  for runnum in run_list:
    run_period = get_runperiod(runnum)
    evio_path = get_evio_path(run_period, runnum)
    file_name = os.path.basename(evio_path)

    call_list = ['swif2', 'add-job', '-workflow', work_flow, '-account', 'halld', '-partition', 'production']
    call_list += ['-ram', '24g', '-os', 'el9', '-cores', "24", '-time', '4h', '-disk', '30gb']
    call_list += ['-input', file_name, 'mss:' + evio_path]
    call_list += ['-stdout', swif_out_dir + '%s_%06d.out' % (work_flow, runnum)]
    call_list += ['-stderr', swif_out_dir + '%s_%06d.err' % (work_flow, runnum)]
    call_list += [script_dir + 'mille.py %s %d' % (input_par_file, runnum)]
    #print(call_list)
    subprocess.call(call_list)

  subprocess.call(['swif2', 'run', work_flow])


def get_par(input_par_file):
  with open(input_par_file) as f:
    l0 = f.readlines()

  mydict = {}
  for x in l0:
    if x.strip().startswith('#') or len(x.strip().split()) < 2:
      continue
    mydict[x.strip().split()[0]] = x.strip().split()[1]

  return mydict

def get_evio_path(run_period, runnum):
  for br in range(10):
    candidate = '/mss/halld/%s/rawdata/Run%06d/hd_rawdata_%06d_%03d.evio' % (run_period, runnum, runnum, br)
    if os.path.exists(candidate):
      print('Found:', candidate)
      return candidate

  print('[Error] cannot find evio files')
  exit(1)


def get_runperiod(runnum):
  dir_list = glob.glob('/mss/halld/RunPeriod-*/rawdata/Run%06d' % runnum)
  if len(dir_list) != 1:
    print('Error in get_runperiod', dir_list)
    exit(1)
  run_period = ''
  for x in dir_list[0].split('/'):
    if 'RunPeriod' in x:
      run_period = x
  return run_period

if __name__ == '__main__':
  main()

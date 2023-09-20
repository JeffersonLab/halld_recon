#! /apps/bin/python3
# coding:utf-8

import sys
import os
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
    call_list = ['swif2', 'add-job', '-workflow', work_flow, '-account', 'halld', '-partition', 'production', '-ram', '8g']
    call_list += ['-stdout', swif_out_dir + '%s_%06d.out' % (work_flow, runnum)]
    call_list += ['-stderr', swif_out_dir + '%s_%06d.err' % (work_flow, runnum)]
    call_list += [script_dir + 'mille.py %s %d' % (input_par_file, runnum)]
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


if __name__ == '__main__':
  main()

#! /apps/bin/python3
# coding:utf-8

import sys
import os
import glob
import shlex
import subprocess
import rcdb


def main():
  args = sys.argv
  if len(args) != 3:
    print('usage: ./mille.py input_parameter_file runnum')
    exit(0)

  input_par_file = args[1]
  runnum = int(args[2])

  par = get_par(input_par_file)

  ver_xml = par['path_to_version_xml']
  ccdb_path = par['path_to_input_ccdb_sqlite']
  output_dir = par['path_to_output_dir'].rstrip('/') + '/'
  num_of_events = int(par['num_of_events'])

  gxenv(ver_xml)
  run_period = get_runperiod(runnum)
  evio_path = get_evio_path(run_period, runnum)

  # Checks if the output file already exists.
  output_file = output_dir + '%06d.root' % runnum
  if os.path.exists(output_file):
    print('Error: file exists')
    print(output_file)
    exit(0)

  sqlite_str = 'sqlite:///' + ccdb_path
  os.environ['CCDB_CONNECTION'] = sqlite_str
  os.environ['JANA_CALIB_URL'] = sqlite_str

  cmd_list = ['hd_root']
  cmd_list.append('-PPLUGINS=MilleFieldOn,TrackingPulls')
  cmd_list.append('-PTRACKINGPULLS:MAKE_TREE=1')
  cmd_list.append('-PTRACKINGPULLS:TREEFILE=%stree%06d.root' % (output_dir, runnum))
  cmd_list.append('-PTRKFIT:ALIGNMENT=1')
  cmd_list.append('-PNTHEADS=4')
  if num_of_events > 0:
    cmd_list.append('-PEVENTS_TO_KEEP=%d' % num_of_events)
  cmd_list.append(evio_path)
  cmd_list.append('-o')
  cmd_list.append(output_file)
  subprocess.call(cmd_list)


def get_evio_path(run_period, runnum):
  for br in range(10):
    candidate = '/cache/halld/%s/rawdata/Run%06d/hd_rawdata_%06d_%03d.evio' % (run_period, runnum, runnum, br)
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


# copied from https://stackoverflow.com/questions/3503719/emulating-bash-source-in-python
def source(source_command):
  command = shlex.split("env -i bash -c '" + source_command + " && env'")
  proc = subprocess.Popen(command, stdout = subprocess.PIPE)
  for line in proc.stdout:
    str_line = line.decode().strip()
    i = str_line.find('=')
    if i <= 0:
      continue
    key = str_line[:i]
    value = str_line[i + 1:]
    os.environ[key] = value
  proc.communicate()

def gxenv(ver_xml):
  bs_save = os.environ['BUILD_SCRIPTS']
  source('source ' + bs_save + '/gluex_env_clean.sh')
  os.environ['BUILD_SCRIPTS'] = bs_save
  source('source ' + bs_save + '/gluex_env_jlab.sh ' + ver_xml)


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

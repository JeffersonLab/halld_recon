#! /usr/bin/python
# coding:utf-8

import sys
import ccdb
import subprocess


def main():
  # CCDB table list
  table_list = []
  table_list.append('/FDC/package1/wire_timing_offsets')
  table_list.append('/FDC/package2/wire_timing_offsets')
  table_list.append('/FDC/package3/wire_timing_offsets')
  table_list.append('/FDC/package4/wire_timing_offsets')

  args = sys.argv
  if len(args) != 3:
    print('usage: ./update_official_ccdb.py ccdb-sqlite-file run-list-file')
    exit(0)

  ccdb_path = args[1]
  run_list_file = args[2]

  with open(run_list_file) as f:
    run_list = [int(x.strip()) for x in f.readlines()]

  for runnum in run_list:
    dict0 = ccdb2dict(ccdb_path, runnum, table_list)
    dict2ccdb(runnum, dict0)


def dict2ccdb(runnum, dict0):
  rand_id = '9Moh3dYvzI'
  ccdb_sqlite = 'mysql://ccdb_user@hallddb.jlab.org/ccdb'
  for k in dict0:
    tmp_file = k.strip('/').replace('/', '_') + '_%s.txt' % rand_id
    with open(tmp_file, 'w') as f:
      f.writelines(['  '.join(x) + '\n' for x in dict0[k]])
    subprocess.call(['ccdb', '-c', ccdb_sqlite, 'add', k, '-r', '%d-%d' % (runnum, runnum), tmp_file])
    subprocess.call(['rm', tmp_file])


def ccdb2dict(ccdb_path, runnum, table_list):
  provider = ccdb.AlchemyProvider()
  provider.connect('sqlite:///' + ccdb_path)
  dict0 = {}
  for x in table_list:
    assignment = provider.get_assignment(x, runnum, 'default')
    dict0[x] = assignment.constant_set.data_table
  return dict0


if __name__ == '__main__':
  main()

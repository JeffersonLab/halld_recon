#! /usr/bin/python
# coding:utf-8

import sys
import os
import glob
import shlex
import shutil
import subprocess

def main():
  args = sys.argv
  if len(args) != 3:
    print('usage: ./iter.py template_parameter_file runnum')
    exit(1)

  temp_par_file = args[1]
  runnum = args[2]

  niter = 10
  for i in range(niter):
      # prepare input parameter file
      input_par_file = f"{temp_par_file}_{i}"
      shutil.copyfile(temp_par_file,input_par_file)

      # Open the file in read mode
      with open(input_par_file, 'r') as file:
          filedata = file.read()
      # Replace the target string
      filedata = filedata.replace('ITER', str(i))
          
      # Write the file out again
      with open(input_par_file, 'w') as file:
          file.write(filedata)
          
      # run mille
      par = get_par(input_par_file)
      output_dir = par['path_to_output_dir'].rstrip('/') + '/'
      ccdb_in_path = par['path_to_input_ccdb_sqlite']
      if i > 0:
          link = "ln -s " + ccdb_out_path + " " + ccdb_in_path
          os.system(link)
      ccdb_out_path = par['path_to_output_ccdb_sqlite']
      os.mkdir(output_dir)
      mille = "./mille.py " + input_par_file + " " + runnum
      print("=== Running mille ===")
      os.system(mille)
      
      # run pede
      shutil.copyfile(ccdb_in_path,ccdb_out_path)
      pede = "./pede.py " + input_par_file + " " + runnum
      print("=== Running pede ===")
      os.system(pede)

  final_link = "ln -s " + ccdb_out_path + " ccdb_" + str(niter) + ".sqlite"
  os.system(final_link)



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

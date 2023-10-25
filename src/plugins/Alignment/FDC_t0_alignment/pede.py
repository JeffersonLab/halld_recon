#! /apps/bin/python3
# coding:utf-8

import os
import sys
import ccdb
import math
import subprocess


def main():
  args = sys.argv
  if len(args) != 3:
    print('usage: ./pede.py input_parameter_file runnum')
    exit(0)

  input_par_file = args[1]
  runnum = int(args[2])

  par = get_par(input_par_file)

  pede_path = par['path_to_pede']
  in_ccdb_path = par['path_to_input_ccdb_sqlite']
  out_ccdb_path = par['path_to_output_ccdb_sqlite']
  rt_dir = par['path_to_output_dir'].rstrip('/') + '/'
  mil_list = [rt_dir + '%06d.mil' % runnum]

  # CCDB table list
  table_list = []
  table_list.append('/FDC/wire_alignment')
  table_list.append('/FDC/cathode_alignment')
  table_list.append('/FDC/strip_pitches_v2')
  table_list.append('/FDC/cell_offsets')
  table_list.append('/FDC/cell_rotations')
  table_list.append('/FDC/package1/wire_timing_offsets')
  table_list.append('/FDC/package2/wire_timing_offsets')
  table_list.append('/FDC/package3/wire_timing_offsets')
  table_list.append('/FDC/package4/wire_timing_offsets')
  table_list.append('/FDC/drift_function_parms')
  table_list.append('/FDC/drift_function_ext')
  table_list.append('/FDC/base_time_offset')
  table_list.append('/CDC/global_alignment')
  table_list.append('/CDC/wire_alignment')
  table_list.append('/CDC/timing_offsets')

  dict0 = ccdb2dict(in_ccdb_path, runnum, table_list)

  # Runs pede.
  generate_mp2str_txt(mil_list, dict0)
  subprocess.call([pede_path, 'mp2str.txt'])

  add_res_to_dict(dict0)  # Updates dict0.
  if not os.path.exists(out_ccdb_path):
    subprocess.call(['cp', in_ccdb_path, out_ccdb_path])
  dict2ccdb(out_ccdb_path, runnum, dict0)

  # Deletes temporary files.
  for x in ['millepede.res', 'millepede.end', 'millepede.end~', 'millepede.his', 'millepede.log', 'mp2str.txt']:
    subprocess.call(['rm', x])


def generate_mp2str_txt(mil_files_list, dict0):
  trailer = '''\
outlierdownweighting     3         ! number of internal iterations (> 1)
dwfractioncut          0.3         ! 0 < value < 0.5
regularisation         1.0  0.005  ! regularisation factor, pre-sigma
threads                  6
method fullMINRES-QLP    6    2.0
end
'''

  str_txt = par_list()
  # str_txt += fdc_constraints(dict0)  # should be commented out?
  # str_txt += cdc_constraints()  # should be commented out?
  str_txt += 'Cfiles\n'
  for x in mil_files_list:
    str_txt += x + '\n'
  str_txt += trailer

  with open('mp2str.txt', 'w') as f:
    f.write(str_txt)


# This function changes dict0.
def add_res_to_dict(dict0):
  with open('millepede.res') as f:
    l0 = f.readlines()
  for x in l0:
    l1 = x.split()
    if l1[0].startswith('Parameter'):
      continue
    par_id = int(l1[0])
    value = float(l1[1])

    # par_id --> CCDB table name
    if int(par_id / 100000) == 1:  # FDC parameters
      par_id %= 100000
      plane = int(par_id / 1000)
      par_id -= plane * 1000  # 1-5, 100-104, 200-209, 997, 998

      if par_id == 1 or par_id == 100:
        table = '/FDC/cell_offsets'
        row = plane - 1
        col = 0 if par_id == 1 else 1
        dict0[table][row][col] = str(float(dict0[table][row][col]) + value)
      elif 2 <= par_id and par_id <= 4:
        table = '/FDC/cell_rotations'
        row = plane - 1
        col = par_id - 2
        dict0[table][row][col] = str(float(dict0[table][row][col]) + value)
      elif par_id == 5:
        table = '/FDC/wire_alignment'
        row = plane - 1
        col = 2
        dict0[table][row][col] = str(float(dict0[table][row][col]) + value)
      elif 101 <= par_id and par_id <= 104:
        table = '/FDC/cathode_alignment'
        row = plane - 1
        col = [1, 3, 0, 2][par_id - 101]
        dict0[table][row][col] = str(float(dict0[table][row][col]) + value)
      elif 200 <= par_id and par_id <= 209:
        table = '/FDC/strip_pitches_v2'
        row = plane - 1
        col = par_id - 200
        dict0[table][row][col] = str(float(dict0[table][row][col]) + value)
      elif par_id == 997 or par_id == 998:
        table = '/FDC/package%d/wire_timing_offsets' % (int((plane - 1) / 6) + 1)
        row = (plane - 1) % 6  # cell number, 0-based
        for col in [cc + (par_id - 997) * 48 for cc in range(48)]:
          dict0[table][row][col] = str(float(dict0[table][row][col]) - value)
      else:
        print('Error: Unknown parameter', par_id)
    else:  # CDC parameters
      if 1 <= par_id and par_id <= 6:
        table = '/CDC/global_alignment'
        row = 0
        col = par_id - 1
        dict0[table][row][col] = str(float(dict0[table][row][col]) + value)
      elif 1001 <= par_id and par_id <= 15088:
        table = '/CDC/wire_alignment'
        row = int((par_id - 1001) / 4)
        col = (par_id - 1) % 4
        dict0[table][row][col] = str(float(dict0[table][row][col]) + value)
      else:
        print('Error: Non FDC/CDC parameter', par_id)
        exit(1)


def dict2ccdb(ccdb_path, runnum, dict0):
  rand_id = '9Moh3dYvzI'
  ccdb_sqlite = 'sqlite:///' + ccdb_path
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


def par_list():
  fixCDCGlobalX = True
  fixCDCGlobalY = True
  fixCDCGlobalZ = True
  fixCDCGlobalPhiX = True
  fixCDCGlobalPhiY = True
  fixCDCGlobalPhiZ = True
  fixCDCWires = True

  fixFDCCathodeOffsets = True
  fixFDCCathodeAngles = True
  fixFDCCellOffsetsWires = True
  fixFDCCellOffsetsCathodes = True
  fixFDCWireRotationX = True
  fixFDCWireRotationY = True
  fixFDCWireRotationZ = True
  fixFDCZ = True
  fixFDCPitch = True
  fixFDCGap = True
  fixFDCT0 = False

  translationPresigma = 0.0005
  rotationPresigma = 0.0001
  pitchPresigma = 0.001
  t0Presigma = 10.0

  l0 = ['Parameter\n']

  # CDC
  l0.append("%10d  %10.4f  %12.4f\n" % (1, 0.0, -1.0 if fixCDCGlobalX    else translationPresigma))
  l0.append("%10d  %10.4f  %12.4f\n" % (2, 0.0, -1.0 if fixCDCGlobalY    else translationPresigma))
  l0.append("%10d  %10.4f  %12.4f\n" % (3, 0.0, -1.0 if fixCDCGlobalZ    else translationPresigma))
  l0.append("%10d  %10.4f  %12.4f\n" % (4, 0.0, -1.0 if fixCDCGlobalPhiX else    rotationPresigma))
  l0.append("%10d  %10.4f  %12.4f\n" % (5, 0.0, -1.0 if fixCDCGlobalPhiY else    rotationPresigma))
  l0.append("%10d  %10.4f  %12.4f\n" % (6, 0.0, -1.0 if fixCDCGlobalPhiZ else    rotationPresigma))
  for i in range(1001, 15089):
    l0.append("%10d  %10.4f  %12.4f\n" % (i, 0.0, -1.0 if fixCDCWires else translationPresigma))

  # FDC
  for i in range(1, 25):
    label_offset = 100000 + i * 1000
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset +   1, 0.0, -1.0 if fixFDCCellOffsetsWires     else translationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset +   2, 0.0, -1.0 if fixFDCWireRotationX        else    rotationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset +   3, 0.0, -1.0 if fixFDCWireRotationY        else    rotationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset +   4, 0.0, -1.0 if fixFDCWireRotationZ        else    rotationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset +   5, 0.0, -1.0 if fixFDCZ                    else translationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 100, 0.0, -1.0 if fixFDCCellOffsetsCathodes  else translationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 101, 0.0, -1.0 if fixFDCCathodeOffsets       else translationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 102, 0.0, -1.0 if fixFDCCathodeOffsets       else translationPresigma))

    # dPhiU, dPhiV
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 103, 0.0, -1.0 if fixFDCCathodeAngles        else    rotationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 104, 0.0, -1.0 if fixFDCCathodeAngles        else    rotationPresigma))

    # Strip pitch and gap
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 200, 0.0, -1.0 if fixFDCPitch                else       pitchPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 201, 0.0, -1.0 if fixFDCGap                  else translationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 202, 0.0, -1.0 if fixFDCPitch                else       pitchPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 203, 0.0, -1.0 if fixFDCGap                  else translationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 204, 0.0, -1.0 if fixFDCPitch                else       pitchPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 205, 0.0, -1.0 if fixFDCPitch                else       pitchPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 206, 0.0, -1.0 if fixFDCGap                  else translationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 207, 0.0, -1.0 if fixFDCPitch                else       pitchPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 208, 0.0, -1.0 if fixFDCGap                  else translationPresigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 209, 0.0, -1.0 if fixFDCPitch                else       pitchPresigma))

    # t0
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 997, 0.0, -1.0 if fixFDCT0                   else          t0Presigma))
    l0.append("%10d  %10.4f  %12.4f\n" % (label_offset + 998, 0.0, -1.0 if fixFDCT0                   else          t0Presigma))
  return (''.join(l0))


def fdc_constraints(dict0):
  constrainPackageZ = False
  constrainPackageZTogether = False
  constrainWireCathodeAlignment = False
  constraint0 = False

  l0 = []
  if constrainPackageZ:
    for i in range (4):
      l0.append('Constraint 0.0\n')
      for j in range(6):
        indexOffset = 100000 + (i * 6 + j + 1) * 1000
        l0.append('%d 1.0\n' % (indexOffset + 5))

  if constrainPackageZTogether:
    for i in range(4):
      for  j in range(1, 6):
        l0.append('Constraint 0.0\n')
        indexOffset = 100000 + (i * 6 + j + 1) * 1000
        l0.append('%d 1.0 \n' % (100000 + (i * 6 + 1) * 1000 + 5))
        l0.append('%d -1.0\n' % (indexOffset + 5))

  if constraint0:
    # t0
    l0.append('Constraint 0.0\n')
    for i in range(24):
      indexOffset = 100000 + (i + 1) * 1000
      for j in range(901, 997):
        l0.append('%d 1.0 \n' % (indexOffset + j))

  for i in range(24):
    indexOffset = 100000 + (i + 1) * 1000
    angleOffset = (i % 6) * math.pi / 3.0;
    l0.append('Measurement 0.0 0.02\n')
    l0.append('%d %f\n' % (indexOffset + 1, math.sin(angleOffset)))
    l0.append('%d %f\n' % (indexOffset + 100, math.sin(angleOffset + math.pi / 2.0)))
    l0.append('Measurement 0.0 0.02\n')
    l0.append('%d %f\n' % (indexOffset + 1, math.cos(angleOffset)))
    l0.append('%d %f\n' % (indexOffset + 100, math.cos(angleOffset + math.pi / 2.0)))

  # Here we need existing values to write the constraint.
  if constrainWireCathodeAlignment:
    for i in range(24):
      # If this is true the cathode pitch is constrained such that the average
      # pitch is unchanged but each of the sections can float. If this is
      # false, the pitch is adjusted the same ammount for each foil.
      averagePitch = True

      indexOffset = 100000 + (i + 1) * 1000
      phiu = 75.0 * math.pi / 180.0
      phiv = math.pi - phiu

      phiu += float(dict0['/FDC/cathode_alignment'][i][0])
      phiv += float(dict0['/FDC/cathode_alignment'][i][2])

      pu1 = float(dict0['/FDC/strip_pitches_v2'][i][0])
      pu2 = float(dict0['/FDC/strip_pitches_v2'][i][2])
      pu3 = float(dict0['/FDC/strip_pitches_v2'][i][4])

      pv1 = float(dict0['/FDC/strip_pitches_v2'][i][5])
      pv2 = float(dict0['/FDC/strip_pitches_v2'][i][7])
      pv3 = float(dict0['/FDC/strip_pitches_v2'][i][9])

      pu = (48.0 * (pu1 + pu3) + 96.0 * pu2) / 192.0  # Average strip pitch
      pv = (48.0 * (pv1 + pv3) + 96.0 * pv2) / 192.0  # Average strip pitch

      sinphiu = math.sin(phiu)
      sinphiv = math.sin(phiv)
      sinphiumphiv = math.sin(phiu - phiv)
      cosphiumphiv = math.cos(phiu - phiv)

      if averagePitch:
        # Avg = 0
        l0.append('Constraint 0.0\n')
        l0.append('%d 48.0\n' % (indexOffset + 200))
        l0.append('%d 96.0\n' % (indexOffset + 202))
        l0.append('%d 48.0\n' % (indexOffset + 204))
        l0.append('Constraint 0.0\n')
        l0.append('%d 48.0\n' % (indexOffset + 205))
        l0.append('%d 96.0\n' % (indexOffset + 207))
        l0.append('%d 48.0\n' % (indexOffset + 209))
      else:
        # Equal
        l0.append('Constraint 0.0\n')
        l0.append(' 1.0\n' % (indexOffset + 200))
        l0.append('-1.0\n' % (indexOffset + 202))
        l0.append('Constraint 0.0\n')
        l0.append(' 1.0\n' % (indexOffset + 200))
        l0.append('-1.0\n' % (indexOffset + 204))
        l0.append('Constraint 0.0\n')
        l0.append(' 1.0\n' % (indexOffset + 205))
        l0.append('-1.0\n' % (indexOffset + 207))
        l0.append('Constraint 0.0\n')
        l0.append(' 1.0\n' % (indexOffset + 205))
        l0.append('-1.0\n' % (indexOffset + 209))

      # Constrain Wire/cathode alignmnet
      l0.append('Constraint 0.0\n')
      # l0.append('Measurement 0.0 0.00001\n')
      l0.append('%d %f\n' % (indexOffset + 200, sinphiv))
      l0.append('%d %f\n' % (indexOffset + 205, sinphiu))
      l0.append('%d %f\n' % (indexOffset + 103, -1 * (pv + pu * cosphiumphiv) * sinphiv / sinphiumphiv))
      l0.append('%d %f\n' % (indexOffset + 104, (pu + pv * cosphiumphiv) * sinphiu / sinphiumphiv))
      l0.append('Constraint 0.0\n')
      # l0.append('Measurement 0.0 0.00001\n')
      l0.append('%d %f\n' % (indexOffset + 200, sinphiv))
      l0.append('%d %f\n' % (indexOffset + 205, -sinphiu))
      l0.append('%d %f\n' % (indexOffset + 103, (pv - pu * cosphiumphiv) * sinphiv / sinphiumphiv))
      l0.append('%d %f\n' % (indexOffset + 104, (pu - pv * cosphiumphiv) * sinphiu / sinphiumphiv))

  return (''.join(l0))


def cdc_constraints():
  perRing = True

  # Some CDC geometry information
  # straw_offset[29]
  straw_offset = [0, 0, 42, 84, 138, 192, 258, 324, 404, 484, 577, 670, 776, 882, 1005,
                  1128, 1263, 1398, 1544, 1690, 1848, 2006, 2176, 2346, 2528, 2710, 2907, 3104, 3313]
  # Nstraws[28], radius[28], phi[28], phi_ds[28]
  Nstraws = [42, 42, 54, 54, 66, 66, 80, 80, 93, 93, 106, 106, 123, 123,
             135, 135, 146, 146, 158, 158, 170, 170, 182, 182, 197, 197, 209, 209]
  radius = [10.72134, 12.08024, 13.7795, 15.14602, 18.71726, 20.2438, 22.01672, 23.50008, 25.15616, 26.61158, 28.33624, 29.77388, 31.3817, 32.75838,
            34.43478, 35.81146, 38.28542, 39.7002, 41.31564, 42.73042, 44.34078, 45.75302, 47.36084, 48.77054, 50.37582, 51.76012, 53.36286, 54.74716]
  phi = [0, 0.074707844, 0.038166294, 0.096247609, 0.05966371, 0.012001551, 0.040721951, 0.001334527, 0.014963808, 0.048683644, 0.002092645, 0.031681749, 0.040719354, 0.015197341,
         0.006786058, 0.030005892, 0.019704045, -0.001782064, -0.001306618, 0.018592421, 0.003686784, 0.022132975, 0.019600866, 0.002343723, 0.021301449, 0.005348855, 0.005997358, 0.021018761]
  phi_ds = [0.000557611, 0.0764693, 0.0385138, 0.0975182, -0.816345, -0.864077, -0.696401, -0.736506, 0.656304, 0.690227, 0.57023, 0.599326, 0.0410675, 0.0145592,
            0.00729358, 0.0296972, 0.43739, 0.415211, 0.38506, 0.405461, -0.355973, -0.337391, -0.317012, -0.334703, 0.0212654, 0.0058214, 0.005997358, 0.0213175]

  # Four output lists
  ldxu = []
  ldxd = []
  ldyu = []
  ldyd = []
  if not perRing:
    ldxu.append('Constraint 0.0\n')
    ldxd.append('Constraint 0.0\n')
    ldyu.append('Constraint 0.0\n')
    ldyd.append('Constraint 0.0\n')

  # No shrinking or stretching of CDC
  # Loop over the rings

  for iRing in range(28):
    # Get angular spacing between straws
    dPhi = 2 * math.pi / Nstraws[iRing]
    if perRing:
      ldxu.append('Constraint 0.0\n')
      ldxd.append('Constraint 0.0\n')
      ldyu.append('Constraint 0.0\n')
      ldyd.append('Constraint 0.0\n')

    # Loop over straws
    for iStraw in range(Nstraws[iRing]):
      index = straw_offset[iRing + 1] + iStraw
      dxu_index = 1000 + index * 4 + 1
      dyu_index = 1000 + index * 4 + 2
      dxd_index = 1000 + index * 4 + 3
      dyd_index = 1000 + index * 4 + 4

      ConstraintXU = math.cos(phi[iRing] + dPhi * iStraw)
      ConstraintXD = math.cos(phi_ds[iRing] + dPhi * iStraw)
      ConstraintYU = math.sin(phi[iRing] + dPhi * iStraw)
      ConstraintYD = math.sin(phi_ds[iRing] + dPhi * iStraw)

      ldxu.append('%d %lf\n' % (dxu_index, ConstraintXU))
      ldxd.append('%d %lf\n' % (dxd_index, ConstraintXD))
      ldyu.append('%d %lf\n' % (dyu_index, ConstraintYU))
      ldyd.append('%d %lf\n' % (dyd_index, ConstraintYD))

  if not perRing:
    ldxu.append('Constraint 0.0\n')
    ldxd.append('Constraint 0.0\n')
    ldyu.append('Constraint 0.0\n')
    ldyd.append('Constraint 0.0\n')

  # No shrinking or stretching of CDC
  # Loop over the rings

  for iRing in range(28):
    # Get angular spacing between straws
    dPhi = 2 * math.pi / Nstraws[iRing]
    if perRing:
      ldxu.append('Constraint 0.0\n')
      ldxd.append('Constraint 0.0\n')
      ldyu.append('Constraint 0.0\n')
      ldyd.append('Constraint 0.0\n')

    # Loop over straws
    for iStraw in range(Nstraws[iRing]):
      index = straw_offset[iRing + 1] + iStraw
      dxu_index = 1000 + index * 4 + 1
      dyu_index = 1000 + index * 4 + 2
      dxd_index = 1000 + index * 4 + 3
      dyd_index = 1000 + index * 4 + 4

      ConstraintXU = math.sin(phi[iRing] + dPhi * iStraw)
      ConstraintXD = math.sin(phi_ds[iRing] + dPhi * iStraw)
      ConstraintYU = math.cos(phi[iRing] + dPhi * iStraw)
      ConstraintYD = math.cos(phi_ds[iRing] + dPhi * iStraw)

      ldxu.append('%d %lf\n' % (dxu_index, ConstraintXU))
      ldxd.append('%d %lf\n' % (dxd_index, ConstraintXD))
      ldyu.append('%d %lf\n' % (dyu_index, ConstraintYU))
      ldyd.append('%d %lf\n' % (dyd_index, ConstraintYD))

  ldxu.append('Constraint 0.0\n')
  ldxd.append('Constraint 0.0\n')
  ldyu.append('Constraint 0.0\n')
  ldyd.append('Constraint 0.0\n')

  # No global shifts
  for iRing in range(28):
    # Get angular spacing between straws
    dPhi = 2 * math.pi / Nstraws[iRing]
    # Loop over straws
    for iStraw in range(Nstraws[iRing]):
      index = straw_offset[iRing + 1] + iStraw
      dxu_index = 1000 + index * 4 + 1
      dyu_index = 1000 + index * 4 + 2
      dxd_index = 1000 + index * 4 + 3
      dyd_index = 1000 + index * 4 + 4

      ConstraintX = 1
      ConstraintY = 1

      ldxu.append('%d %lf\n' % (dxu_index, ConstraintX))
      ldxd.append('%d %lf\n' % (dxd_index, ConstraintX))
      ldyu.append('%d %lf\n' % (dyu_index, ConstraintY))
      ldyd.append('%d %lf\n' % (dyd_index, ConstraintY))

  ret_str = ''.join(ldxu)
  ret_str += ''.join(ldxd)
  ret_str += ''.join(ldyu)
  ret_str += ''.join(ldyd)
  return ret_str


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

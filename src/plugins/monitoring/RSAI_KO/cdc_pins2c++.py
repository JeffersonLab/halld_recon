#!/usr/bin/env python

import re

cdc_pins_file   = "CDC_pins_spacedelim.txt"
cxx_output_file = "CDC_pins.h"
boards = []
with open( cdc_pins_file, "r") as f:
    for line in f.readlines():
        vals = line.split()
        if len(vals) < 4: continue
        board = vals[1]
        straws = []
        for swire in vals[3:]:
            if swire=="none": continue
            (straw,) = re.findall(r'[0-9]+', swire)  # pull out number part of straw
            (ring_lab,) = re.findall(r'[A-Z]+', swire)  # pull out letter part for ring
            if len(ring_lab)==1:  # for single letter ring names, use ASCII code to convert to number
                ring = ord(ring_lab) - ord('A') + 1
            if ring_lab == 'AA': ring = 27  # double character lables must be handled separately
            if ring_lab == 'BB': ring = 28

            straws.append('{'+str(ring)+','+str(straw)+'}')
        board_str = '{"'+board+'", {' + ','.join(straws) + '}}\n'
        boards.append( board_str )


with open( cxx_output_file, "w" ) as of:
    of.write('''
#include <vector>
#include <string>
#include <map>

std::map<std::string, std::vector<std::pair<int,int> > > CDC_pins = {
    ''')
    of.write( '\t,'.join(boards) + '};\n\n')
    of.close()
    print('Wrote CDC pin info. for ' + str(len(boards)) + ' boards to ' + cxx_output_file)

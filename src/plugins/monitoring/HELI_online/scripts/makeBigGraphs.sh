#!/bin/sh

start="0"
finish="3000000"

if [ $# -eq 0 ] 
then
    echo
    echo "Usage: $0  <input file> <start> <end>"
    echo "This processes the output of the Helicity Plugin for hd_root"
    echo
    echo "First, run the plugin in VERBOSE mode, something like this:"
    echo
    echo "hd_root -PPLUGINS=HELI_online,EPICS_dump -PHELI:VERBOSE=2 -PHELI:LOG=helicity_121121_172.log -PTT:NO_CCDB=1"
    echo "-PTT:XML_FILENAME=/group/halld/Users/jrsteven/TranslationTable/tt_helicity.xml hd_rawdata_121121_172.evio -PEVIO:NTHREADS=4 -PNTHREADS=8 -o hd_root_121121_172.root"
    echo
    echo "Now process the log file (helicity_121121_172.log), for example:"
    echo "$0 helicity_121121_172.log 0 3000000"
    echo
    echo "It should make a root canvas with the timelines of the helicity information for the events".
    echo "Edit this file to modify the output with simple unix tools - see comments in $0"
    exit 1
fi

infile=$1
if [ $# -eq 2 ]
then
    start=$2
fi
if [ $# -eq 3 ]
then
    start=$2
    finish=$3
fi

outfile="temp-heli-event.txt"

#The input file is like this:
#       ..
#       <EventNo>   i1 i2 i3 i4 i5 i6 i7 i8 i9 .... F  Where the i's are ints and F is a flag for Standard or Latest event St/La
#       <EventNo>   i1 i2 i3 i4 i5 i6 i7 i8 i9 .... F 
#       ..
#       #Tag ... other info 
#
# ie mostly lines beginning with an event number and all the helicity states, etc.
# but with some #Tag lines that can be grepped for useful debugging.

#This tools deals with <EventNo> lines, and makes an oscilloscope type plot in ROOT.

#Currently there are 2 sets of plots:
# 1 - With Event number on the x axis
# 2 - With time relative to the first event on the x-axis


#The joys of awk!

#Skip comments and save the events to output in the desired range.
grep -v \# $infile | awk -v start=$start -v finish=$finish '{if(ev>=start)print $0;if(ev>finish)exit;ev++}' > $outfile

echo
echo "Replacing event no with approximate time assuming pattern-sync = (1/4)*30 Hz )"
echo "This takes a few minutes ..... be patient. ROOT TCanvases coming soon."

#In preparation, make a list of the events at sync edges (sort numerically then find events where $3 (3rd field, pattern-sync) goes from 0->1. 
grep -v \# $infile | sort -n  | awk -v start=$start -v finish=$finish '{if(ev==start)prevsyn=-1;if((ev>=start)&&(ev<=finish)&&($3>prevsyn)){if(pe!="")print pe;pe=$1}prevsyn=$3;le=$1;ev++}END{print le;print le+10}' > temp-heli-syncs.txt

#Load in the above table. Then use it to allocate times to events, assuming they are evenly spread between pattern-syncs. Replace eventNo with time.
grep -v "#" $infile | sort -n| awk -v start=$start -v finish=$finish 'BEGIN{while(getline < "temp-heli-syncs.txt")sync[n++]=$1}{if(ev>=start){n=0;while($1>=sync[n])n++;tick=1.0/(sync[n]-sync[n-1]);tt=4/30*(n+(($1-sync[n-1])*tick));t=sprintf("%3.6f",tt);sub($1,t);print $0}if(ev>finish)exit;ev++}' > temp-heli-time.txt

root 'bhtimeline2.C("temp-heli-event.txt", "temp-heli-time.txt")'

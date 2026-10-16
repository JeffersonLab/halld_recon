#!/bin/sh

if [ $# -eq 0 ] 
then
    echo
    echo "Usage: $0  <log file>"
    echo "This processes the output of the Helicity Plugin for hd_root to make a helicity lookup table for 2 files written in parallel"
    echo " ... an even number and the consecutive number: Eg helicity_121121_172.log and helicity_121121_173.log"
    echo
    echo "First, run the plugin in VERBOSE mode, for each file ... something like this:"
    echo
    echo "hd_root -PPLUGINS=HELI_online,EPICS_dump -PHELI:VERBOSE=2 -PHELI:LOG=helicity_121121_172.log -PTT:NO_CCDB=1"
    echo "-PTT:XML_FILENAME=/group/halld/Users/jrsteven/TranslationTable/tt_helicity.xml hd_rawdata_121121_172.evio -PEVIO:NTHREADS=4 -PNTHREADS=8 -o hd_root_121121_172.root"
    echo
    echo
    echo "hd_root -PPLUGINS=HELI_online,EPICS_dump -PHELI:VERBOSE=2 -PHELI:LOG=helicity_121121_173.log -PTT:NO_CCDB=1"
    echo "-PTT:XML_FILENAME=/group/halld/Users/jrsteven/TranslationTable/tt_helicity.xml hd_rawdata_121121_173.evio -PEVIO:NTHREADS=4 -PNTHREADS=8 -o hd_root_121121_173.root"
    echo
    echo "Now process the log files (helicity_121121_172.log helicity_121121_173.log), to make a lookup table, for example:"
    echo "$0 helicity_121121_172.log"
    echo
    echo "It should make files called helicity_121121_172c.table, and helicity_121121_172c.seed"
    exit 1
fi

infile=$1

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
# here we just get the randome seed from the #Seed line

heltable=`echo $infile |  awk '{gsub(".log","c.table");print $0}'`
helseed=`echo $infile  |  awk '{gsub(".log","c.seed");print $0}'`
#infile2=`echo $infile  |  awk 'BEGIN{FS="_"}{t=$0;gsub( ".log","");on=sprintf("%03d.log\n",$NF);nn=sprintf("%03d.log\n",$NF+1);gsub(t,on,nn);print t}'`
infile2=`echo $infile  |  awk 'BEGIN{FS="_"}{t=$0;gsub( ".log","");on=sprintf("%03d.log",$NF);nn=sprintf("%03d.log",$NF+1);gsub(on,nn,t);print t}'`
#echo "$infile $infile2 $heltable $helseed"

/bin/rm -f $heltable
/bin/rm -f $helseed

#Skip comments and save the events to output in the desired range.
grep Seed $infile2 | awk '{print $2}' > $helseed

#skip comments, sort in event order and write events where helicity changes
grep -h -v \# $infile $infile2 | sort -n | awk 'BEGIN{lp = 99}{if($12!=lp){print $1, $12;lp=$12}}' > $heltable


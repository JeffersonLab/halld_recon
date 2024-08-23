#!/bin/sh

if [ $# -ne 3 ]

then
    echo
    echo "Usage: $0  <log file stub> <first> <last>"
    echo
    echo "Eg.    $0 helicity_121121   0 256"
    echo
    echo "This will first combine pairs of runs (n,n+1) into tables using makeDoubleTable.sh"
    echo ".... then concatenate them in order to make a big table"
    echo
    exit
fi

stub=$1
fileno=$2
last=$3
outfile="$stub.table"

/bin/rm -f $outfile

while [ $fileno -le $last ];
do
    filename=`echo "$stub $fileno" |  awk '{printf"%s_%03dc.table",$1,$2}'`
    argument=`echo "$stub $fileno" |  awk '{printf"%s_%03d.log",$1,$2}'` 
    echo "running ./makeDoubleTable $argument, and adding $filename to $outfile"
    ./makeDoubleTable.sh $argument
    cat $filename >> $outfile
    fileno=$((fileno+2))
done

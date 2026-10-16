#!/bin/sh

if [ $# -ne 3 ]

then
    echo
    echo "Usage: $0  <log file stub> <first> <last>"
    echo
    echo "Eg.    $0 helicity_121121   0 253"
    echo
    echo "This will first combine pairs of runs helicity.logs (n,n+1) sorted into event order"
    echo ".... then run a root macro to make a big helicity lookup table"
    echo
    exit
fi

stub=$1
first=$2
last=$3
fileno=$last
sortedlist="$stub.list"
templist="$stub.temp"
touch $templist
outtable="$stub.table"
heltable="$stub.HelTable.txt"
syncfile="$stub.sync"

temphead="$stub.head"   #temp file to hold 1st 100 lines of serted file pair to tag onto the previous file pair 
                        #since a buffers worth of earlier event numbers go into the next file (ie out of sequence).
lastsorted="none"       #mane of the last sorted pair

/bin/rm -f $temphead    #remove and temporary head
/bin/rm -f $sortedlist  #and list of sorted files
/bin/rm -f $templist

while [ $fileno -ge $first ];   #sort pairs in reverse order.
do

    filenoA=`echo $fileno | awk '{printf"%03d",$1-1}'`    #make filenames n, n-1
    filenoB=`echo $fileno | awk '{printf"%03d",$1}'`
    infile1="${stub}_$filenoA.log"                        #add fill path
    infile2="${stub}_$filenoB.log"
    outfile="${stub}_${filenoA}_${filenoB}_sorted.log"    #fill name of sorted pair
    if1=$infile1                                          #temporary copies
    if2=$infile2
    headfile="${outfile}.head"
    if3=""                                                
    if [ -e $temphead ]; then
	if3=$temphead                                     #make this part of the input
	echo "Cutting 20000 lines from start of $lastsorted"
	sed -i -e 1,20000d $lastsorted                      #and cut the corresponding 100 lines from the start of the previous sorted pair.
    fi

    if [ ! -e $infile1 ]; then 
	echo "WARNING: $infile1 is missing"
	if1=""
    fi
    if [ ! -e $infile2 ]; then 
	echo "WARNING: $infile2 is missing"
	if2=""
    fi

    if [ ! -e $infile1 ] && [ ! -e $infile1 ]; then 
	echo "SERIOUS WARNING: $infile1 and  $infile2 are BOTH missing"
	fileno=$((fileno-2))
	continue
    fi

    /bin/rm -f $outfile                                  #remove any previous output 
    echo "sorting $if1, $if2, $if3  -> $outfile"          
    echo
    grep -h -v \#  $if1 $if2 $if3 | sort -n > $outfile   #sort the pair and the temporary header (skipping coments)
    /bin/rm -f $temphead                                 #now remove 

    makehead=`wc -l $outfile | awk '{if ($1>20000){print"y"} else{print "n"}}'`
    if [ $makehead = "y" ]; then                         #if more than 200  lines
	head -n 20000 $outfile > $temphead                 #copy the 1st 200 lines to patch onto the end of the previous file pair
	cp $temphead $headfile
	lastsorted=$outfile                              #save the name of this
    fi
    echo $outfile >> $templist                           #add to the filelist for analysing
    #fileno=$((fileno-2))
    fileno=`echo $fileno | awk '{printf"%03d",$1-2}'`
done

sort $templist > $sortedlist                             #sort the pair files into ascending order

echo "Making $syncfile"
/bin/rm -f $syncfile                                     #save event numbers and timestamps for all pair_sync transitions. 
#For old data before decoder board                          
awk 'BEGIN{ls=-1}{if($5!=ls){print$1,$2;ls=$5;lt=$2}}' $(<$sortedlist) > $syncfile

#For new data after decoder board
#awk 'BEGIN{ls=-1}{if($3!=ls){print$1,$2;ls=$3;lt=$2}}' $(<$sortedlist) > $syncfile
synclen=`wc -l $syncfile | awk '{print $1}' `            #and count the no of lines in the file

echo "Making $outtable"
/bin/rm -f $outtable                                     #remove the old one

rootargs="\"$sortedlist\", \"$outtable\",\"$syncfile\", $synclen"

echo "running root -l -b -q makeBigTableNoSeed.C($rootargs)"
root -l -b -q "makeBigTableNoSeed.C($rootargs)"

#To make an old style .txt .table - fix for script variables.
/bin/rm -f $heltable
echo "Making $heltable"
grep -v "\#" $outtable | awk 'BEGIN{lh=-2}{if($13!=lh)print $1, $13, $7; lh=$13}' > $heltable
#To make a graphable timeline
#grep -v "\#" hel_120400.table | awk '{sub(pev,$1,prev);sub(pt,$2,prev);if(n++>0){print prev};print $0; prev=$0;pev=$1;pt=$2}' > hel_120400.graph
#now make the table

RunNo=$1
OutputDir=OUTPUT_TIME_SHIFT/${RunNo}
InputFile=$2
cp -p ${InputFile} hd_root.root

ccdb dump /PHOTON_BEAM/hodoscope/fadc_time_offsets:${RunNo} >  adc_time_offsets.txt
ccdb dump /PHOTON_BEAM/hodoscope/tdc_time_offsets:${RunNo}  >  tdc_time_offsets.txt

root -b -q 'slice_tagh.C()'

rm -f hd_root.root
mkdir -p ${OutputDir}

mv *.txt ${OutputDir}/.; 

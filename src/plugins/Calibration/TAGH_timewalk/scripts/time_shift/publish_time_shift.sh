RunNo=$1
RunNo_min=$2
RunNo_max=$3
Dir=OUTPUT_TIME_SHIFT/${RunNo}

ccdb add /PHOTON_BEAM/hodoscope/fadc_time_offsets -v default -r ${RunNo_min}-${RunNo_max} ${Dir}/adc_time_offsets_calib.txt '#Correct time shift'
ccdb add /PHOTON_BEAM/hodoscope/tdc_time_offsets  -v default -r ${RunNo_min}-${RunNo_max} ${Dir}/tdc_time_offsets_calib.txt '#Correct time shift'

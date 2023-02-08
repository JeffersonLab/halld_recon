# cdc_scan

This creates a root file containing 3 trees of low-level pulse data and configuration data from the fa125, with the option to emulate the firmware if the raw samples are present (long mode):

- T contains the number of pulse data and trigger time objects found.
- P contains the pulse data, window raw data and configuration data.
- TT contains the trigger times

Use -PCDC_SCAN:EMU=0 to switch off the emulation.  It is switched on by default.

Use -PEVIO:F125_EMULATION_MODE=2 to produce the emulated firmware values using window raw data using the configuration parameters in Df125BORConfig. 
eg
```
hd_root hd_rawdata.evio -PPLUGINS=cdc_scan -PEVIO:F125_EMULATION_MODE=2 
```

The plugin adds the emulated values to its output tree for comparison with the original values from the firmware, which are not replaced.  The comparison is only possible when run with mode=2 as above. The configuration parameters in the Df125BORConfig objects are used for the emulation. If these are not present, then default values are used (and a warning message is printed if EVIO:EMULATION125:VERBOSE=1). 

Different configuration parameters can be supplied, [find the list here](https://github.com/JeffersonLab/halld_recon/blob/master/src/libraries/DAQ/Df125EmulatorAlgorithm_v2.cc), eg -PEMULATION125:FDC_H=60 would set the emulated hit threshold to 60 for all FDC channels.   

The branches in the tree with names starting m_ are emulated quantities. Branches with names starting d_ contain firmware minus emulated. The branch named diffs contains 1 if the firmware and emulated values differ.

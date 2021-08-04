# cdc_scan

This creates a root file containing 3 trees of low-level CDC pulse data from the fa125:

- T contains the number of pulse data and window raw data found.
- P contains the pulse data.
- W contains the window raw data, if present and required.

The command line parameter CDC_SCAN:SHORT_MODE=0 directs the plugin to include the window raw data present in long mode runs.  The default is 1.

 It can easily be modified to include FDC data, at present this is bypassed in line 519.
```sh
 if (wrd->rocid>28) continue; //skip fdc
```

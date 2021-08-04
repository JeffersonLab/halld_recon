# cdc_emu

This emulates the fa125 firmware, using the window raw data from long mode runs.
It creates a root file tree_m.root containing the raw data, the CDC pulse data from the fadc and the pulse data calculated by the plugin. 

The plugin expects to find the RunLog configuration files in a specific location, which should be customised before compiling the plugin.  This is in line 182 of JEventProcessor_cdc_emu.cc.

The plugin reads in the hit thresholds from RunLog configuration files but the timing thresholds TH and TL are hardcoded as they have not yet been changed.  The thresholds are also stored in the BOR records, but those are not always present in the evio files. 

There are two command line parameters:

- CDC:DIFFS_ONLY  Set this to 1 to record only the events where differences between calculated and reported quantities are found. The default is 0.

- CDC:RUN_CONFIG This allows an alternate run number to be specified as a source of configuration files. The default is the current run number. 
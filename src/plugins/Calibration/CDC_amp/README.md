# CDC wire gain calibrations

These use the histograms from the plugin CDC\_amp.  
Approximately 100 evio files are required, from a run taken when the atmospheric pressure was low to average, when RCDB's cdc\_gas\_pressure was 100.1 kPa or below. 
The scripts referred to here are in the subdirectory scripts.

The root script CDC\_gains.C calculates the relative gain for each of the wires in the chamber. This calibration is usually performed close to the start of each run period, after the CDC has been powered off for several months, and again later on if any electronics boards have been replaced. 

The script can also be used to calculate the overall chamber gain for each run, before performing another calibration launch and fine-tuning the gain with the CDC\_dedx plugin, but this is no longer standard practice.

gaincalibs.py creates the following files:
- cdc_amphistos.root Landau fit parameters for each straw
- cdc_new_ascale.txt Overall chamber gain factor
- cdc_new_wiregains.txt Gain factors for individual chamber wires

**To run the gain calibrations:**

1. Make a new directory and cd into it
2. Run the root script CDC_gains.C, eg
```sh
root -b -q /CDC_amp_071344_merged.root $HALLD_RECON_HOME/src/plugins/Calibration/CDC_amp/CDC_gains.C
```
3. Check for error messages, most likely due to insufficient hits in the outer rings. If this happens, run the plugin over more data files. Files from consecutive runs can be combined if there is no change in pressure.
4. Load the new gain constants into ccdb, eg
```sh
ccdb add /CDC/wire_gains -r 71344- new_wiregains.txt
```


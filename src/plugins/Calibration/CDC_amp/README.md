# CDC wire gain calibrations

These use the histograms from the plugin CDC\_amp.  
Approximately 100 evio files are required, from a run taken when the atmospheric pressure was low to average, when RCDB's cdc\_gas\_pressure was 100.1 kPa or below. 
The scripts referred to here are in the subdirectory scripts.

The root script CDC\_gains.C calculates the relative gain for each of the wires in the chamber. This calibration is usually performed close to the start of each run period, after the CDC has been powered off for several months, and again later on if any electronics boards have been replaced. 

The script can also be used to calculate the overall chamber gain for each run, before performing another calibration launch and fine-tuning the gain with the CDC\_dedx plugin, but this is no longer standard practice.

CDC\_gains.C creates the following files:
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



# Labelling dead straws for MC

This uses the histograms from the plugin CDC_amp.  One evio file is sufficient.
The scripts referred to are in the subdirectory scripts_mc.

list\_dead\_straws.C looks through the histogram of tracked hits and compiles a list of straws for which the number of tracked hits is less than 1/4 of the average for their ring of straws.
find\_dead\_straws.py runs list\_dead\_straws.C over all root files in the subdirectory hists.

find\_dead\_straws.py can produce quite lengthy output, as list\_dead\_straws.C produces a report for each file examined. It creates files listing the straw efficiency (as 1 or 0) for the first run encountered and any later runs where the list of dead straws has changed, and a file containing the commands to add the lists to ccdb. These are:
* eff\_tables Directory containing straw efficiency files
* add\_files\_to\_ccdb.sh Script to add the tables to CCDB

**To run the search process:**

1. Make a new directory and cd into it
2. Copy find\_dead\_straws.py and list\_dead\_straws.C into this directory
3. Make a symbolic link from the launch results to a directory hists, eg
```sh
ln -s /work/halld/data_monitoring/RunPeriod-2019-11/mon_ver23/rootfiles/ hists
```
4. Run the find\_dead\_straws script 
```sh
python find_dead_straws.py > searchresults.txt
```
5. Look through the search results and/or check the output files, eg to list the number of bad straws reported each time:
```sh
grep -c 0 eff_tables/*  
```
6. Load the new gain constants into ccdb, eg
```sh
chmod +x add_files_to_ccdb.sh 
./add_files_to_ccdb.sh 
```


# Finding the hit thresholds 

This requires the run configuration files, either in the RunLog tar file stored on tape with the evio files, or on the gluons, in /gluex/CALIB/ALL/fadc125/.
The scripts are in the subdirectory scripts_mc.

geth.C extracts the thresholds from the configuration files and writes them into a simple table suitable for CCDB.
CDC\_straw\_numbers\_run\_3221.txt is a reference table used by geth.C

geth.C creates the following file:
- cdc_h.txt  This is a list of the thresholds, ordered by straw number


**To extract the thresholds:**

1. Make a new directory and cd into it
2. Copy geth.C, CDC\_straw\_numbers\_run\_3221.txt and the configuration files into this directory.  
3. Edit the configuration filename in the script, line 65.
4. Run the script 
```sh
root -q geth.C 
```
5. Load the data into CCDB, eg
```sh
ccdb add /CDC/hit_thresholds -r 71860-72435 cdc_h.txt
```

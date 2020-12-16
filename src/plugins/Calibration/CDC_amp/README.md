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


# Gain vs DOCA corrections.

This procedure was developed for the spring 2018 runs when the gas supply to the CDC failed part way through the run period.  It is *not* in routine use.  
It uses the output of the CDC\_amp plugin. Many evio files are required - it's best to use all of the files for each run.
The script is in the subdirectory scripts.

get_gain_doca_params.C uses the histogram adoca_30 for tracks at theta=30degrees.
It performs a linear fit to amplitude vs doca from 3mm out until the data become less predictable.  The parameters obtained populate the CCDB table /CDC/gain_doca_correction

**To obtain the gain vs DOCA correction parameters:**

1. Make subdirectories rootfiles, ccdb, mpv3mm and plots.
2. Run the script, passing in the run number as an argument - this is used when creating the output files.
```sh
root -q hd_root_072555.root "get_gain_doca_params.C(72555)"
```
3. The files containing the parameters for CCDB should be in the ccdb directory and can be added using the script add_to_ccdb.sh generated in the previous step.



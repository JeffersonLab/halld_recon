# CDC time to distance calibrations

These use the histograms from the plugin CDC\_TimeToDistance.  
One evio file is sufficient.
The scripts referred to here are in the subdirectory scripts.

ttod\_setup.sh copies the scripts into the current directory and creates a link to the calibration histograms
ttodfit.C fits the time to distance parameters used to estimate the track DOCA from drift time. 
runcalibs.py runs ttodfit.C over all root files in the subdirectory hists.

gaincalibs.py creates the following directories, containing one file per run:
- Before - fit function contours for the parameters stored in the root file
- After - fit function contours for the new parameters obtained from the calibration
- Combined - overlay of old and new fit function contours
- ResVsT - 2D plot of track fit residual vs drift time
- Proj - track fit residuals, fitted with a single Gaussian
- Monitoring - all of the above, on one canvas 
- ccdb - file containing new calibration constants to be added to ccdb

It also creates the following files:
- res.txt  - run number and mean and sigma of the fitted Gaussian
- res6.txt - as above, for tracks with FOM 0.6+
- res9.txt - as above, for tracks with FOM 0.9+
- addtoccdb.sh - bash script to add the data to ccdb.
- badfiles.txt - list of files for which calibrations failed (may not be present)

plotres.py generates plots of the mean and sigma of the Gaussian fitted to the track residuals
scanfitresults.py scans the fit results and prints a list of any with mean larger than 8μm 

**To run the calibrations:**

1. Make a new directory and cd into it
2. Run the script ttod_setup.sh and specify the calibration launch results directory on the command line, eg
```sh
$HALLD_RECON_HOME/src/plugins/Calibration/CDC_TimeToDistance/scripts/ttod_setup.sh /work/halld/data_monitoring/RunPeriod-2019-11/mon_ver23/rootfiles/
```
3. Run the calibration script:
```sh
python runcalibs.py
```
4. Inspect the results
5. Load the new gain constants into ccdb:
```sh
chmod +x addtoccdb.sh
./addtoccdb.sh
```

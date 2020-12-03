# CDC dE/dx calibrations

The histograms from the plugin CDC\_dedx are used to fine-tune the gain. 
One evio file is sufficient.
The scripts referred to here are in the subdirectory scripts.

fit\_dedx.C fits gaussians to the dE/dx bands at 0.5 and 1.5 GeV/c
gaincalibs.py runs fit\_dedx.C over all root files in the subdirectory hists.
findoutliers.py scans the output file for dE/dx outliers

gaincalibs.py creates the following directories, containing one file per run:
- mip - mean dE/dx at 1.5 GeV/c
- oldgains - content of ccdb table /CDC/digi\_scales 
- newgains - file containing newly calculated gain to be added to ccdb

It also creates the following files:
- dedx\_mean\_at\_1\_5GeV.txt  - run number and mean dE/dx at 1.5 GeV/c
- dedx\_peak\_scan.txt - run number, mean and sigma of dE/dx at 0.5GeV/c (pi+ and proton) and 1.5GeV/c (combined).
- run\_pi\_mean\_sig\_p\_mean\_sig\_both\_mean\_sig.txt - as dedx\_peak\_scan.txt with the additional text removed
- addtoccdb.sh - bash script to add the data to ccdb.


**To run the gain calibrations:**

1. Make a new directory and cd into it
2. Copy gaincalibs.py and fit\_dedx.C into this directory
3. Make a symbolic link from the launch results to a directory hists, eg
```sh
ln -s /work/halld/data_monitoring/RunPeriod-2019-11/mon_ver23/rootfiles/ hists
```
4. Run the gaincalibs script 
```sh
python gaincalibs.py
```
5. Inspect the results in dedx\_peak\_scan.txt
```sh
python findoutliers.py
```
6. Load the new gain constants into ccdb:
```sh
chmod +x addtoccdb.sh
./addtoccdb.sh
```

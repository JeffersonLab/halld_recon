Scripts for FDC t0 calibration.


# Preparation

## Two input files should be prepared:
input parameter file .. see input_example
run-list file        .. see runlist_example

## Directives in input parameter file (input_example)
path_to_input_ccdb_sqlite   .. absolute path to CCDB SQLite file used for the reconstruction (Mille plugin)
path_to_version_xml         .. absolute path to version XML file for the reconstruction
path_to_output_dir          .. absolute path to ouput directory for .mil files (outputs of Mille plugin)
path_to_swif2_output_dir    .. absolute path to swif2 (standard and error) outputs
path_to_pede                .. absolute path to the executable "pede". See https://www.desy.de/~kleinwrt/MP2/doc/html/index.html to install Millepede (="pede").
path_to_output_ccdb_sqlite  .. absolute path to CCDB SQLite file for output results. This can be the same as path_to_input_ccdb_sqlite.
num_of_events               .. Number of events which will be analyzed by Mille plugin.


# Scripts and how to use

## mille.py
script for Mille plugin.
.mil file is the main output which is necessary for the next "pede" step.
This script is for run-by-run analysis. Use swif2.py to analyze multiple runs.
Make sure raw data files (.evio files) are on the /cache disk before running this script (use "jcache get" command if necessary).

## swif2.py
script for swif2 job which runs multiple mille.py scripts.
Again, make sure raw data files (.evio files) are on the /cache disk before running this script (use "jcache get" command if necessary).

## pede.py
wrapper script for "pede" command.
.mil files are necessary as inputs for this pede.py, so use swif2.py (or mille.py) before this script.
This is for run-by-run fitting (new FDC t0 constants in output CCDB SQLite file will be set run-by-run).

## update_official_ccdb.py
script to update the officiaal CCDB.
This script should be used after pede.py.

How to use pede.py in this directory


# Preparation

## One input file should be prepared.
See `input_example` as an example

## Directives in input parameter file (`input_example`)
`path_to_pede`                absolute path to the executable `pede`. See https://www.desy.de/~kleinwrt/MP2/doc/html/index.html to install Millepede (=`pede`).

`path_to_input_ccdb_sqlite`   absolute path to CCDB SQLite file used for the reconstruction (Mille plugin)

`path_to_output_ccdb_sqlite`  absolute path to CCDB SQLite file for output results. This can be the same as `path_to_input_ccdb_sqlite`.

`runnum`                      Run Number. The CCDB table for this run number in the output CCDB file will be updated.

`use_multiple_mil_files`      Set true if you use multiple .mil files as inputs. false for a single .mil file.

`path_to_mil`                 Set the path to your .mil file if you use a single .mil file as an input. In case you use multiple .mil files, set the path to the directory which contains those .mil files. Note that ALL the .mil files will be used as inputs in this case.


# Scripts and how to use

## pede.py
wrapper script for `pede` command.

.mil files are necessary as inputs for this pede.py, so use `Mille` plugin before this script.

`./pede.py your_input_parameter_file`

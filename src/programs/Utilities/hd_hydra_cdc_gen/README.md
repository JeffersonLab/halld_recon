This program will read in CDC occupancy histograms from 
a list of ROOT files (Produced by RootSpy) and generate
PNG files using the standard CDC occupancy macro. The
histograms will be modified to mimic one or more HV boards
having failed though so as to produce "bad" images that
can be used in Hydra model training.

Important info:

    1. By default it looks for a file called "root_files.txt"
       in the current directory for the list of ROOT files
       to use. One file per line. Empty lines and comment lines
       starting with "#" are allowed. In-line comments are not.

    2. The default output directory will be called "images"
       and will be created by running the "mkdir -p" shell
       command. Thus, it won't error if it already exists.
       One can also specify this with the "RSAI:OUTPUT_DIR"
       config. parameter.

    3. This is hard-coded to produce an image for each HV
       board so that a complete set of single board failure
       images is produced. To generate 2 board failures this
       file will need to be edited and recompiled.

    4. A file "origin_log.txt" will be created in the output
       images directory in order to makea record of where
       each image file came from since they are generically named.


This links in the standard Hall-D DANA libraries so that
the occupancy_online plugin can be attached and the 
CDC_occupancy macro accessed that way. Unlike most JANA
programs though, it does not process events. (Unfortunately
this adds some ugliness in that we create a dummy event
source just to avoid JANA errors/warnings).

Being a JANA program, this also has access to config.
parameters which can be set using the standard command
line options.

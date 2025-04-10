# CDC dE/dx correction procedure for variation with theta

The first iteration of this process is documented in [GlueX-doc-4693](https://halldweb.jlab.org/doc-private/DocDB/ShowDocument?docid=4693).
The scripts are stored in the subdirectory scripts.
The correction matrix is loaded into the CCDB table /CDC/dedx\_theta.

## Prerequisites: 
* 300+ evio files processed with the dedx\_tree plugin, with dE/dx truncation and the dE/dx correction switched off ( -PPID:CDC_CORRECT_DEDX_THETA=0 and -PPID:CDC_TRUNCATE_DEDX=0 ) 

* Simulated data processed similarly. 


## Process overview:

1. makebinnedhistos.C - create many histograms of dedx vs p, for 1 degree bins in theta

2. fitbinnedhistoslg.C - find the peak position for the proton and pion bands in each (p, theta) bin, using theta-specific functions to define the fit regions

3. makesimsinglebinnedhistos.C - as (1), with simulated data
    
4. fitbinnedhistoslg.C - as (2) with simulated data

5. makescalefactors.C - calculate the scale factors to match the real data to the simulated data

6. extendscalefactors.C - extrapolate the scale factors to populate the edges of a rectangular space in (dedx, theta)  (root -b -q dedx_correction_factors.root extendscalefactors.C)

7. smooth_histo.C - run a smoothing algorithm over the scale factors and write the matrix of scale factors into a text file which can be used in the reconstruction code. (root extended_dedx_correction_factors.root smooth_histo.C)

8. checkmatrix.C - use the matrix contained in the text file to correct the data in the root trees and create a set of corrected binned histograms, which can be fitted as before to verify that the correction process produced acceptable results. 


## Plotting scripts
* drawdedxcuts.C - plots functions on top of specified dEdx vs p histogram from histogram file
* drawalldedxcuts.C - loops through all histograms in histo file, saves 2x2 canvases of dedx functions over data
* plotdata.C - reads in fitresults.root or fittedsimresults.root, plots coloured bands of dedx peak position for one particle type 
* plotf.C - reads in dedx_correction_factors.root, plots coloured bands of correction factors for one particle type
* plotfdedx.C - reads in dedx_correction_factors.root, plots dedx vs theta for the data used for the correction factors
* plot_cf_vs_dedx.C - reads in dedx_correction_factors.root, makes plots cf vs theta for 5 degree steps in theta
* dots.C - reads in dedx_correction_factors.root - makes a selection of plots of cf, dedx, theta including a 3D dot plot
* drawsurf2.C - use this with the tree file open, works with dedx_correction_factors.root or extended_dedx_correction_factors.root  - draws surfaces, dots and surface+dots


## Other scripts
* make_histo_into_text_file.C - writes the histogram into a text file without smoothing



A quick 3D plot of plot correction factors vs dedx, theta in colour scale can be made from the open root tree using 
```sh
t->Draw("f:dedx:theta:f","","cont4 col")
```



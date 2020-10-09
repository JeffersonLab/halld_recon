
# CDC dE/dx correction procedure for variation with theta

#Prerequisite: 
300+ evio files processed with the CDC_dedx plugin, with dE/dx truncation and the dE/dx correction switched off ( -PPID:CDC_CORRECT_DEDX_THETA=0 and -PPID:CDC_TRUNCATE_DEDX=0 ) 

Simulated data processed similarly. 


#Overview:

1. makebinnedhistos.C - create many histograms of dedx vs p, for 1 degree bins in theta

1.5 Set up the fit functions to delineate the mass bands

2. fitbinnedhistoslg.C - find the peak position for the proton and pion bands

3. plotdata.C - plot the peak positions for p and pi separately, look for bad fits

3. makesimsinglebinnedhistos.C - as (1), with simulated data
    
4. fitbinnedhistoslg.C - as (2) with simulated data

5. makescalefactors.C - calculate the scale factors to match the real data to the simulated data    This will need quite a bit of alteration for dedx_integral, you should be able to use more of the angular range for both real and sim data.

6. extendscalefactors.C - extrapolate the scale factors to populate the edges of a rectangular space in (dedx, theta)  (root -b -q dedx_correction_factors.root extendscalefactors.C)

7. smooth_histo.C - root extended_dedx_correction_factors.root smooth_histo.C -  run a smoothing algorithm over the scale factors and write them into a text file.  This is the correction matrix used in halld_recon etc.

8. checkmatrix.C - combines the original root trees with the correction matrix to generate a new set of binned and unbinned histograms.  It's much quicker than running the plugin over the data again, using the same correction matrix, and should give identical results.  It does not make new root trees, only histograms.  It could easily be modified to make the new trees, if needed.








Plotting scripts
drawdedxcuts.C - plots dedx cut functions on top of specified histogram from histogram file
drawalldedxcuts.C - loops through all histos in histo file, saves 2x2 canvases of dedx cut functions over data
plotdata.C -  reads in fitresults.root or fittedsimresults.root, plots coloured bands of dedx peak for one particle type 
plotf.C - reads in dedx_correction_factors.root - plots coloured bands of correction factors for one particle type
plotfdedx.C - reads in dedx_correction_factors.root - plots dedx vs theta for the data used for the correction factors
plot_cf_vs_dedx.C - reads in dedx_correction_factors.root - makes plots cf vs theta for 5 degree steps in theta
dots.C - reads in dedx_correction_factors.root - makes a selection of plots of cf, dedx, theta including a 3D dot plot
drawsurf2.C - use this with the tree file open, works with dedx_correction_factors.root or extended_dedx_correction_factors.root  - draws surfaces, dots and surface+dots



More
root -b -q extended_dedx_correction_factors.root make_histo_into_text_file.C
root rebuild_histo_from_text_file.C  (makes the flat colz plot)



For a quick 3D plot of plot correction factors vs dedx, theta  in colour scale t->Draw("f:dedx:theta:f","","cont4 col")
Idk why that works.



 



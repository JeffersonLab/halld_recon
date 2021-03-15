#!/bin/csh -f
set echo
#
# streamline.csh
# Elton Smith. Sep 18, 2017. 
# Streamline instructions to process the output of mcsmear output files through amplitude analysis.
# Update Feb 13, 2018: Modify script to use Paul's new ReactionFilter Plugin (See https://halldweb.jlab.org/doc-private/DocDB/ShowDocument?docid=3407)
# Update Jan 11, 2019: update pippim -> pi0pi0, 2pi -> 2pi0
#
source ~/MC_environment.csh
unset run
unset base
unset maxev

if( $#argv == 0 ) then
    set run = "031000"
    set base = "test"
#    set base = "TEST"
    set maxev = 100000
else if ( $#argv == 1) then
    set run = $1
    set base = ""
    set maxev = 100000
else if ( $#argv == 2) then
    set run = $1
    set base = $2
    set maxev = 100000
else if ( $#argv == 3) then
    set run = $1
    set base = $2
    set maxev = $3
else
    echo "Too many arguments=" $1 $2 $3 $4
endif

cd /work/halld/home/elton/gen_2pi0_primakoff_signal/Z2pi0_trees

echo " run =" $run
echo " base=" $base
echo " maxev=" $maxev
set savebase = $base

# Here are instructions for processing MC smeared output files / or data

#Not Needed: gen_2pi0_primakoff -c gen_2pi0_primakoff_signal.cfg -o tree_gen_2pi0_primakoff_signal_${maxev}.root -hd gen_2pi0_primakoff_signal.hddm -a 5.5 -b 6.0 -p 6.0 -m 11.6 -n ${maxev} -r ${run}

#Optional: hd_root -PPLUGINS=monitoring_hists,ReactionFilter -PReaction1=1_111__m111_8_9 -PNTHREADS=4  -PCOMBO:DEBUG_LEVEL=500 -PKINFIT:DEBUG_LEVEL=500 -PANALYSIS:DEBUG_LEVEL=500 -PVERTEXINFO:DEBUG_LEVEL=500 -PEVENTS_TO_KEEP=${maxev} ../../gen_2pi0_primakoff_${base}/hddm/dana_rest_gen_2pi0_primakoff_${base}_signal_${run}_*.hddm -o hd_root_Z2pi0_trees_${base}_signal_${maxev}.root

# echo "echo:" hd_root -PPLUGINS=monitoring_hists,ReactionFilter -PReaction1=1_111__m111_8_9 -PNTHREADS=4 -PEVENTS_TO_KEEP=${maxev} -PKALMAN:ADD_VERTEX_POINT=1 ../hddm/dana_rest_gen_2pi0_primakoff_${base}_${run}_19*.hddm -o hd_root_Z2pi0_trees_${base}_signal_${maxev}_test.root
# hd_root -PPLUGINS=monitoring_hists,ReactionFilter -PReaction1=1_111__m111_8_9 -PNTHREADS=4 -PEVENTS_TO_KEEP=1000 -PKALMAN:ADD_VERTEX_POINT=1 ../hddm/dana_rest_gen_2pi0_primakoff_${base}_${run}_190.hddm -o hd_root_Z2pi0_trees_${base}_signal_${maxev}_test.root 
#
# Use this option if root trees have already been created using MC wrapper
# rm -f tree_pi0pi0misspb208.root
# hadd tree_pi0pi0misspb208.root ../root/trees/tree_pi0pi0misspb208__B2_gen_2pi0_primakoff_${base}_${run}_*.root
# mv tree_pi0pi0misspb208.root tree_hd_root_Z2pi0_trees_${base}_signal_${maxev}.root
# root -b -q tree_hd_root_Z2pi0_trees_${base}_signal_${maxev}.root 'call_DSelector2.C("DSelector_Z2pi0_trees2.C+")' >! DSelector_Z2pi0_trees_${base}_signal_${maxev}.list
# mv DSelector_Z2pi0_trees2.root DSelector_Z2pi0_trees_${base}_signal_${maxev}.root
# mv treeFlat_DSelector_Z2pi0_trees.root treeFlat_DSelector_Z2pi0_trees_${base}_signal_${maxev}.root
# echo root -l -q  plot_Z2pi_trees.C\(\"DSelector_Z2pi0_trees_${base}_signal_${maxev}\"\)
# root -l -q  plot_Z2pi_trees.C\(\"DSelector_Z2pi0_trees_${base}_signal_${maxev}\"\)
## No Longer needed: tree_to_amptools treeFlat_DSelector_Z2pi0_trees_${base}_signal_${maxev}.root pi0pi0misspb208_TreeFlat
# root -b -q treeFlat_DSelector_Z2pi0_trees_${base}_signal_${maxev}.root 'call_MakeAmpToolsFlat_pi0.C(1)'
# mv AmpToolsInputTree.root treeFlat_DSelector_Z2pi0_trees_${base}_signal_${maxev}_amptools_W.root
# root -b -q treeFlat_DSelector_Z2pi0_trees_${base}_signal_${maxev}.root 'call_MakeAmpToolsFlat_pi0.C(2)'
# mv AmpToolsInputTreeInTime.root treeFlat_DSelector_Z2pi0_trees_${base}_signal_${maxev}_amptools_InTime.root
# root -b -q treeFlat_DSelector_Z2pi0_trees_${base}_signal_${maxev}.root 'call_MakeAmpToolsFlat_pi0.C(3)'
# mv AmpToolsInputTreeOutTime.root treeFlat_DSelector_Z2pi0_trees_${base}_signal_${maxev}_amptools_OutTime.root
# root -b -q treeFlat_DSelector_Z2pi0_trees_${base}_signal_${maxev}.root 'call_MakeAmpToolsFlat_pi0.C(4)'
# mv AmpToolsInputTreeInTimeW.root treeFlat_DSelector_Z2pi0_trees_${base}_signal_${maxev}_amptools_InTimeW.root

# Now repeat for flat distribution. Also need the generated flat distributions.
set base = "test"

#No longer needed: gen_2pi0_primakoff -c gen_2pi0_primakoff_flat.cfg -o tree_gen_2pi0_${base}_primakoff_flat_${maxev}.root -hd gen_2pi0_primakoff_flat.hddm -a 5.5 -b 6.0 -p 6.0 -m 11.6 -n ${maxev} -r ${run}
#No longer needed: root -b -q tree_gen_2pi0_primakoff_${base}_flat_${maxev}_gen.root 'call_MakeAmpToolsFlat_gen.C'
#
# use the following process to obtain generated 'tagged'files
# hd_root -PPLUGINS=monitoring_hists,mcthrown_tree -PNTHREADS=4 -PEVENTS_TO_KEEP=${maxev} ../../gen_2pi0_primakoff_flat/hddm/dana_rest_gen_2pi0_primakoff_${base}_${run}_*.hddm -o hd_root_Z2pi0_trees_${base}_flat_${maxev}_gen.root
# mv tree_thrown.root tree_hd_root_Z2pi0_trees_${base}_flat_${maxev}_gen.root
# root -b -q tree_hd_root_Z2pi0_trees_${base}_flat_${maxev}_gen.root 'call_MakeAmpToolsFlat_mcthrown_pi0.C'
# mv AmpToolsInputTree.root treeFlat_gen_2pi0_primakoff_${base}_flat_${maxev}_amptools.root

#
# Use this option if root trees have already been created using MC wrapper for flat files.
# rm -f tree_pi0pi0misspb208.root
# hadd tree_pi0pi0misspb208.root ../../gen_2pi0_primakoff_flat/root/trees/tree_pi0pi0misspb208__B2_gen_2pi0_primakoff_${base}_${run}_*.root
# mv tree_pi0pi0misspb208.root tree_hd_root_Z2pi0_trees_${base}_flat_${maxev}.root
# root -b -q tree_hd_root_Z2pi0_trees_${base}_flat_${maxev}.root 'call_DSelector2.C("DSelector_Z2pi0_trees2.C+")' >! DSelector_Z2pi0_trees_${base}_flat_${maxev}.list
# mv DSelector_Z2pi0_trees2.root DSelector_Z2pi0_trees_${base}_flat_${maxev}.root
# mv treeFlat_DSelector_Z2pi0_trees.root treeFlat_DSelector_Z2pi0_trees_${base}_flat_${maxev}.root
# root -b -q  plot_Z2pi_trees.C\(\"DSelector_Z2pi0_trees_${base}_flat_${maxev}\"\)
#No longer needed: tree_to_amptools tree_DSelector_Z2pi0_trees_${base}_flat_${maxev}.root pi0pi0misspb208_Tree
# root -b -q treeFlat_DSelector_Z2pi0_trees_${base}_flat_${maxev}.root 'call_MakeAmpToolsFlat_pi0.C(2)'   # avoid any negative weights
# mv AmpToolsInputTreeInTime.root treeFlat_DSelector_Z2pi0_trees_${base}_flat_${maxev}_amptools.root   

# set base = ${savebase}
set tagfit = "W"
set base = ${savebase}_${tagfit}

# fit -c fit_2pi0_primakoff_${tagfit}_${maxev}.cfg  >! twopi_primakoff_DSelect_${base}_${maxev}.list
# cp twopi0_primakoff.fit twopi_primakoff_DSelect_${base}_${maxev}.fit
# twopi_plotter_primakoff twopi_primakoff_DSelect_${base}_${maxev}.fit -o twopi_primakoff_DSelect_${base}_${maxev}.root
mv twopi_fitPars.txt twopi_primakoff_DSelect_${base}_${maxev}.fit2
root -b -q twopi_primakoff.C\(\"twopi_primakoff_DSelect_${base}_${maxev}\",${maxev}\)

unset echo


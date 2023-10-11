


set (analysis_projects
        ReactionFilter
        b1pi_hists
        ReactionEfficiency
        DAQTree
        DAQTreeBCAL
        monitoring_hists
        p2pi_hists
        p3pi_hists
        p4pi_hists
        p2k_hists
        p2gamma_hists
        ppi0gamma_hists
        p2pi0_hists
        fcal_charged
        TPOL_tree
        mcthrown_tree
        trackeff_hists
        imaging
        pid_dirc
        truth_dirc
        lut_dirc
        dirc_hists
        dirc_reactions
        dirc_tree
        TRD_hists
        B3pi_eff_missgamma
        B3pi_eff_misspip
        B3pi_eff_misspim
        B3pi_eff_missprot
        mcthrown_hists
        compton
        # Everything below is optional but hey, it compiles
        bcal_calib
        acceptance_hists
        fcal_hists
        ccal_hits
        ccal_display
        fcal_led
        F250_mode10_pedestal
)

set (optional_but_broken_analysis_projects
        phys_tree
        pedestals
        bcal_calib_cosmic_cdc
        cdc_hists
        dc_alignment
        p2pi_trees
        Z2pi_trees
        )

set (presumably_abandoned_analysis_projects
        bcal_hists
        bcal_shower
        bcalfcaltof_res_tree
        candidate_tree
        cdc_covariance_hists
        DAQ_WRD2PI
        DCdEdxStudy_tree
        eloss_tree
        eta_ntuple
        event_size
        F250_mode8_pedestal
        fdc_covariance_hists
        fdc_hists
        invariant_mass_hists
        level1_trigger
        mc_tree
        photoneff_hists
        pidstudies_tree
        pulls_tree
        radlen_hists
        rho_p_hists
        track_hists
        trackeff_hists2
        trk_profile
        trkfitterLSLM
        trkres_tree
        )


foreach (target_name ${analysis_projects})
    file(GLOB ${target_name}_SOURCES ${target_name}/*.cc)
    add_library(${target_name} SHARED ${${target_name}_SOURCES})
    target_include_directories(${target_name} PRIVATE ../include ../../libraries ../../libraries/include ../../include ${ROOT_INCLUDE_DIRS} ${JANA_INCLUDE_DIR} ${CCDB_INCLUDE_DIRS})
    target_link_libraries(${target_name} halld_recon ${CCDB_LIB})
    set_target_properties(${target_name} PROPERTIES PREFIX "")
    install(TARGETS ${target_name} DESTINATION plugins)
endforeach ()

file(GLOB 1p2pi_SOURCES src-ct/1p2pi/*.cc)
add_library(1p2pi SHARED ${1p2pi_SOURCES})
target_include_directories(1p2pi PRIVATE ../include ../../libraries ../../libraries/include ../../include ${ROOT_INCLUDE_DIRS} ${JANA_INCLUDE_DIR} ${CCDB_INCLUDE_DIRS})
target_link_libraries(1p2pi halld_recon ${CCDB_LIB})
set_target_properties(1p2pi PROPERTIES PREFIX "")
install(TARGETS 1p2pi DESTINATION plugins)

file(GLOB 1p1pi1pi0_SOURCES src-ct/1p1pi1pi0/*.cc)
add_library(1p1pi1pi0 SHARED ${1p1pi1pi0_SOURCES})
target_include_directories(1p1pi1pi0 PRIVATE ../include ../../libraries ../../libraries/include ../../include ${ROOT_INCLUDE_DIRS} ${JANA_INCLUDE_DIR} ${CCDB_INCLUDE_DIRS})
target_link_libraries(1p1pi1pi0 halld_recon ${CCDB_LIB})
set_target_properties(1p1pi1pi0 PROPERTIES PREFIX "")
install(TARGETS 1p1pi1pi0 DESTINATION plugins)

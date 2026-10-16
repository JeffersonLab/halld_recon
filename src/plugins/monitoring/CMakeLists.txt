
set (monitoring_projects
        occupancy_online
        lowlevel_online
        BCAL_online
        BCAL_LEDonline
        BCAL_LED
        BCAL_LED_time
        CCAL_online
        CDC_dedx
        CDC_online
        DAQ_online
        FCAL_online
        FCAL_invmass
        FDC_online
        FDC_Efficiency
        PSC_online
        RF_online
        pedestal_online
        PS_online
        PSPair_online
        PS_flux
        ST_online_lowlevel
        ST_online_tracking
        ST_online_Tresolution
        ST_online_efficiency
        ST_online_multi
        TAGGER_online
        TAGH_online
        TAGM_online
        TAGM_clusters
        TOF_online
        TRIG_online
        TPOL_online
        CDC_expert
        CDC_expert_2
        TOF_TDC_shift
        BCAL_Eff
        BCAL_inv_mass
        CDC_drift
        CDC_PerStrawReco
        CDC_roc_hits
        EPICS_dump
        CDC_Efficiency
        L1_online
        highlevel_online
        BCAL_Hadronic_Eff
        FCAL_Hadronic_Eff
        SC_Eff
        TOF_Eff
        TS_scaler
        TrackingPulls
        TrackingPulls_straight
        timing_online
        DIRC_online
        TRD_online
        RSAI_KO      # TODO: NWB: Revisit once I understand where exactly default tags are used
        BEAM_online  # TODO: NWB: This does not appear to have any locks whatsoever
        # Optional
        TAGH_doubles
        ST_ZEff
        fa125_itrig
        cppFMWPC
        scaler_primex
        lumi_mon
        FMWPC_online
        cppFMWPC_ana
        )

set (optional_monitoring_projects
        TAGH_doubles
        ST_ZEff
        )



foreach (target_name ${monitoring_projects})
    file(GLOB ${target_name}_SOURCES ${target_name}/*.cc)
    add_library(${target_name} SHARED ${${target_name}_SOURCES})
    target_include_directories(${target_name} PRIVATE ../include ../../libraries ../../libraries/include ../../include ${ROOT_INCLUDE_DIRS} ${EVIO_INCLUDE_DIR} ${JANA_INCLUDE_DIR} ${CCDB_INCLUDE_DIRS})
    target_link_libraries(${target_name} halld_recon ${EVIO_LIB} ${CCDB_LIB})
    set_target_properties(${target_name} PROPERTIES PREFIX "")
    install(TARGETS ${target_name} DESTINATION plugins)
endforeach ()


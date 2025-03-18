
set (utilities_plugins
        danarest
        evio_writer
        evio-hddm
        2trackskim
        pi0bcalskim
        pi0fcalskim
        twogamma_fcal_skim
        run_summary
        track_skimmer
        trackeff_missing
        ps_skim
        trigger_skims
        bigevents_skim
        coherent_peak_skim
        exclusivepi0skim
        randomtrigger_skim
        pi0fcaltofskim
        single_neutral_skim
        compton_neutral_skim
        eta2g_primexd_skim
        eta6g_primexd_skim
        etapi0_primexd_skim
        cdcbcal_skim
        EventTagPi0
        es_test
        omega_skim
        cal_high_energy_skim
        syncskim
        Pi0Finder
        dedx_tree
        cdc_scan
        cdc_goodtrack_skim

        # Optional

        danahddm
        dumpcandidates
        dumpthrowns
#        l3bdt # Ported, but broken elsewhere
        merge_rawevents
        DAQ
        TTab
        rawevent

        # Abandoned

#        cdc_emu
#        danaevio
#        eviodana
#        janaded
#        scanf250
#        trackanal
        )


foreach (target_name ${utilities_plugins})
    file(GLOB ${target_name}_SOURCES ${target_name}/*.cc)
    add_library(${target_name} SHARED ${${target_name}_SOURCES})
    target_include_directories(${target_name} PRIVATE . ../include ../../libraries ../../libraries/include ../../include ${ROOT_INCLUDE_DIRS} ${JANA_INCLUDE_DIR} ${EVIO_INCLUDE_DIR} ${CCDB_INCLUDE_DIRS})
    target_link_libraries(${target_name} halld_recon ${EVIO_LIB} ${CCDB_LIB})
    set_target_properties(${target_name} PROPERTIES PREFIX "")
    install(TARGETS ${target_name} DESTINATION plugins)
endforeach()



set (utilities_programs
#        analysis              # No C++ sources
        root_merge
        root2email
#        hddm                  # Needs hddm_x.hpp
        hddm_cull_events
        hddm_merge_files
#        mkplugin              # TODO
#        mkfactory_plugin      # TODO
        hdevio_scan
        hdbeam_current
        hdevio_sample
        hdskims               # TODO: NWB: Commented out all JANA refs because they appear to be no-ops anyway
#        mergeTrees            # No C++ sources
        hd_rsai_gen
        hd_hydra_cdc_gen

        # Optional

#        hdfast_parse          # Doesn't appear to exist!
        hddm2root
        dumpwires
        evio_merge_events
        evio_merge_files
        evio_cull_events
#        evio_check            # Doesn't appear to exist!
        mkMaterialMap
#        material2root         # TODO: Easy
        hddm_select_events
        bfield2root
#        hd_geom_query         # TODO: Easy
        )


foreach (proj ${utilities_programs})
    file(GLOB ${proj}_SOURCES ${proj}/*.cc ${proj}/*.cpp)
    add_executable(${proj} ${${proj}_SOURCES})
    target_include_directories(${proj} PRIVATE . ../include ../../libraries ../../libraries/include ../../include ${JANA_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${EVIO_INCLUDE_DIR} ${CCDB_INCLUDE_DIRS})
    target_link_libraries(${proj} halld_recon ${CCDB_LIB} ${EVIO_LIB})
endforeach ()

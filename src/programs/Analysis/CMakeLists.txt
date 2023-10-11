

set (analysis_programs
        hd_ana
        hd_dump
        hd_eventfilter
        hd_root
#        hdview2   # TODO: NWB: This one will take more work
        )


foreach (project ${analysis_programs})
    file(GLOB ${project}_SOURCES ${project}/*.cc)
    add_executable(${project} ${${project}_SOURCES})
    target_include_directories(${project} PRIVATE . ../include ../../libraries ../../libraries/include ../../include ${CCDB_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${JANA_INCLUDE_DIR} ${EVIO_INCLUDE_DIR})
    target_link_libraries(${project} halld_recon ${EVIO_LIB} ${CCDB_LIB})
endforeach()




set(alignment_projects
        MilleFieldOn
        MilleFieldOff
        MilleKs
        FDCProjectionResiduals
        FDC_InternalAlignment
        )

foreach (target_name ${alignment_plugins})
    file(GLOB ${target_name}_SOURCES ${target_name}/*.cc)
    add_library(${target_name} SHARED ${${target_name}_SOURCES})
    target_include_directories(${target_name} PRIVATE ../include ../../libraries ../../libraries/include ../../include ${ROOT_INCLUDE_DIRS} ${JANA_INCLUDE_DIR})
    target_link_libraries(${target_name} halld_recon)
    set_target_properties(${target_name} PROPERTIES PREFIX "")
    install(TARGETS ${target_name} DESTINATION plugins)
endforeach ()


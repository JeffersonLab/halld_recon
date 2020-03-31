cmake_minimum_required(VERSION 3.15)
project(halld_recon)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(ROOT REQUIRED)
find_package(JANA REQUIRED)

# Only add ET if ETROOT is set
#etroot = os.getenv('ETROOT')
#if(etroot != None) :
list(APPEND CMAKE_CXX_FLAGS -DHAVE_EVIO=1)


if (DEFINED ENV{EVIOROOT})
    message(STATUS "Compiling with EVIOROOT=" $ENV{EVIOROOT})
    set(EVIO_INCLUDE_DIR $ENV{EVIOROOT}/include)
    set(EVIO_LIB_DIR $ENV{EVIOROOT}/lib)
    set(EVIO_LIBS $ENV{EVIOROOT}/lib/evioxx $ENV{EVIOROOT}/lib/evio $ENV{EVIOROOT}/lib/expat)
else()
    message(WARN "Compiling without EVIO")
endif()

file(GLOB DAQ_SOURCES ${PROJECT_SOURCE_DIR}/src/libraries/DAQ/*.cc)
file(GLOB DAQ_HEADERS ${PROJECT_SOURCE_DIR}/src/libraries/DAQ/*.h)

set(HALLD_RECON_SOURCES
        src/libraries/DAQ/JEventSource_EVIO.cc
        src/libraries/DAQ/JEventSource_EVIO.h
    )

add_executable(halld_recon
        src/programs/Analysis/hd_port/hd_port.cc
        ${HALLD_RECON_SOURCES}
        )

target_include_directories(halld_recon PRIVATE src/libraries src/libraries/include ${ROOT_INCLUDE_DIRS} ${JANA_INCLUDE_DIR} ${EVIO_INCLUDE_DIR})
target_link_libraries(halld_recon jana2 ${EVIO_LIBS})



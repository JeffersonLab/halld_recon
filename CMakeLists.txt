cmake_minimum_required(VERSION 3.15)
project(halld_recon)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(JANA REQUIRED)

file(GLOB DAQ_SOURCES ${PROJECT_SOURCE_DIR}/src/libraries/DAQ/*.cc)
file(GLOB DAQ_HEADERS ${PROJECT_SOURCE_DIR}/src/libraries/DAQ/*.h)

set(HALLD_RECON_SOURCES
        ${DAQ_SOURCES}
        ${DAQ_HEADERS}
    )

add_executable(halld_recon
        src/programs/Analysis/hd_port/hd_port.cc
        ${HALLD_RECON_SOURCES}
        )

target_include_directories(halld_recon PRIVATE src/libraries ${JANA_INCLUDE_DIR})
target_link_libraries(halld_recon jana2)



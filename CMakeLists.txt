cmake_minimum_required(VERSION 3.15)
project(halld_recon)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(jana2 REQUIRED)

set(HALLD_RECON_SOURCES
    )
add_executable(halld_recon ${HALLD_RECON_SOURCES})

target_link_libraries(halld_recon jana2)



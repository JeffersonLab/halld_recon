cmake_minimum_required(VERSION 3.14)
project(RSAI_KO)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(${JANA_HOME}/include)

add_executable(RSAI_knockout
        CDC_pins.h
        DCDCDigiHit_factory_KO.cc
        DCDCDigiHit_factory_KO.h
        JFactoryGenerator_RSAI_KO.h
        RSAI_KO.cc)

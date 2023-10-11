
# Copyright 2020, Jefferson Science Associates, LLC.
# Subject to the terms in the LICENSE file found in the top-level directory.

#[=======================================================================[.rst:

Finds the EVIO library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``EVIO::evio``
  The EVIO library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``EVIO_FOUND``
  True if the system has the JANA library.
``EVIO_VERSION``
  The version of the JANA library which was found.
``EVIO_INCLUDE_DIRS``
  Include directories needed to use JANA.
``EVIO_LIBRARIES``
  Libraries needed to link to JANA.

#]=======================================================================]


if (DEFINED EVIOROOT)
    set(EVIO_ROOT_DIR ${EVIOROOT})
    message(STATUS "Using EVIO_ROOT_DIR = ${EVIO_ROOT_DIR} (From CMake EVIOROOT variable)")

elseif (DEFINED ENV{EVIOROOT})
    set(EVIO_ROOT_DIR $ENV{EVIOROOT})
    message(STATUS "Using EVIO_ROOT_DIR = ${EVIO_ROOT_DIR} (From EVIOROOT environment variable)")

else()
    message(FATAL_ERROR "Missing $EVIOROOT")
endif()

set(EVIO_VERSION 4.4.6)

find_path(EVIO_INCLUDE_DIR
        NAMES "evio.h"
        PATHS ${EVIO_ROOT_DIR}/include
        )

find_library(EVIO_LIB
        NAMES evio
        PATHS ${EVIO_ROOT_DIR}/lib
        )

find_library(EVIOXX_LIB
        NAMES evioxx
        PATHS ${EVIO_ROOT_DIR}/lib
        )

set(EVIO_LIBS ${EVIO_LIB} ${EVIOXX_LIB})

set(EVIO_INCLUDE_DIRS ${EVIO_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EVIO
        FOUND_VAR EVIO_FOUND
        VERSION_VAR EVIO_VERSION
        REQUIRED_VARS EVIO_ROOT_DIR EVIO_INCLUDE_DIR EVIO_LIBS
        )
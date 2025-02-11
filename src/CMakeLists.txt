
# To run on macOS:
#DYLD_LIBRARY_PATH=/Users/nbrei/gluex_top/evio/4.4.6/Darwin-x86_64/lib:/Users/nbrei/src/hdds/.build

cmake_minimum_required(VERSION 3.15)
project(halld_recon)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


option(USE_ASAN "Compile with address sanitizer" OFF)
option(USE_TSAN "Compile with thread sanitizer" OFF)

if (${USE_ASAN})
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if (${USE_TSAN})
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

# Set CMake variables according to env variables defined by build_scripts
set(ROOT_DIR $ENV{ROOTSYS}/cmake)
set(XercesC_DIR $ENV{XERCESCROOT})

find_package(ROOT REQUIRED COMPONENTS Geom)
find_package(XercesC REQUIRED)
add_compile_definitions(HAVE_XERCES)
add_compile_definitions(XERCES3)
find_package(JANA REQUIRED)
find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# TODO: ET? From $ETROOT iff set

find_package(EVIO)
if (EVIO_FOUND)
    set(CMAKE_CXX_FLAGS "-DHAVE_EVIO=1 -O0 -fno-omit-frame-pointer")
    add_definitions(-DHAVE_EVIO)
    find_package(EXPAT REQUIRED)
endif()

find_package(MySQL REQUIRED)
add_definitions(-DHAVE_RCDB -DRCDB_MYSQL)

set(CCDB_INCLUDE_DIRS $ENV{CCDB_HOME}/include)
set(CCDB_LIBRARY $ENV{CCDB_HOME}/lib/libccdb.a)
set(SQLITE_LIBRARY -L$ENV{SQLITE_HOME}/lib -lsqlite3)
set(XSTREAM_INCLUDE_DIR external/xstream/include)
set(HDDS_INCLUDE_DIR $ENV{HDDS_HOME}/$ENV{BMS_OSNAME}/src)
set(HDDS_LIBRARY $ENV{HDDS_HOME}/$ENV{BMS_OSNAME}/lib/libhdds.so)
set(RCDB_INCLUDE_DIR $ENV{RCDB_HOME}/cpp/include)

add_definitions(-DHAVE_CCDB)

find_package(ZLIB)
find_package(BZip2)

file(GLOB XSTREAM_SOURCES external/xstream/src/*.cpp)
add_library(xstream ${XSTREAM_SOURCES})
target_include_directories(xstream PUBLIC external/xstream/include)
target_link_libraries(xstream ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})


add_subdirectory(libraries)
add_subdirectory(programs)
add_subdirectory(plugins)

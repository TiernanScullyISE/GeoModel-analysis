
#
# Set build options and C++ standards and options
#
# This file sets up
#
#   CMAKE_BUILD_TYPE
#   CMAKE_CXX_STANDARD
#   CMAKE_CXX_EXTENSIONS
#   CMAKE_CXX_STANDARD_REQUIRED
#
# The options can be overridden at configuration time by using, e.g.:
#    `cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=14 ../GeoModelIO`
# on the command line.
#

# Set default build options.
set( CMAKE_BUILD_TYPE "Release" CACHE STRING "CMake build mode to use" )
set( CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard used for the build" )
set( CMAKE_CXX_EXTENSIONS FALSE CACHE BOOL "(Dis)allow using GNU extensions" )
set( CMAKE_CXX_STANDARD_REQUIRED TRUE CACHE BOOL "Require the specified C++ standard for the build" )

# Setting CMAKE_CXX_FLAGS to avoid "deprecated" warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations" ) # very basic
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic-errors -Wno-deprecated-declarations" ) # good enough for a quick, better check
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic-errors -Wno-deprecated-declarations" ) # better for a thorough check
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic-errors" ) # better for an even more severe check
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Werror -pedantic-errors" ) # not recommended, it warns for really EVERYTHING!

# Enable "warning as errors" globally, for all targets in the GeoModel project
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
if(CMAKE_COMPILE_WARNING_AS_ERROR)
    message(STATUS "${Yellow}INFO: to check the GeoModel code better, we're treating warnings as errors. [If you want to temporarely disable this behaviour in your local build, use the related CMake command-line option: 'cmake --compile-no-warning-as-error'].${ColourReset}")
endif()

# TODO: for Debug and with GCC, do we want to set the flags below by default?
# set( CMAKE_BUILD_TYPE DEBUG )
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O0 -g -gdwarf-2" )

# GeoModel is built as C++20 and inlines weak instantiations of libstdc++'s
# std::__format internals into its shared libraries. libstdc++.so exports no
# strong overrides for these, so the dynamic linker would treat GeoModel's
# copies as authoritative and crash downstream consumers built against a
# different libstdc++ minor version. Localize them with a linker version
# script where the linker supports it. See:
# https://gitlab.cern.ch/GeoModelDev/GeoModel/-/work_items/133
set( GEOMODEL_HIDE_STD_FORMAT_VERSION_SCRIPT
   "${CMAKE_CURRENT_LIST_DIR}/hide-std-format.ver" )
set( GEOMODEL_VERSION_SCRIPT_FLAG
   "-Wl,--version-script=${GEOMODEL_HIDE_STD_FORMAT_VERSION_SCRIPT}" )
# Probe the linker explicitly rather than assuming version-script support:
# ELF/GNU linkers accept it, macOS's ld64 does not.
include( CheckLinkerFlag )
check_linker_flag( CXX "${GEOMODEL_VERSION_SCRIPT_FLAG}"
   GEOMODEL_VERSION_SCRIPT_USABLE )
if( GEOMODEL_VERSION_SCRIPT_USABLE )
   string( APPEND CMAKE_SHARED_LINKER_FLAGS " ${GEOMODEL_VERSION_SCRIPT_FLAG}" )
   string( APPEND CMAKE_MODULE_LINKER_FLAGS " ${GEOMODEL_VERSION_SCRIPT_FLAG}" )
endif()

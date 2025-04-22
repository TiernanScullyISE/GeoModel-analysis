# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: HelloGeoRead
# author: Riccardo Maria BIANCHI @ CERN - Nov, 2018
################################################################################


project(HelloGeoRead)

# Compile with C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Find the needed dependencies, when building individually
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR) 
  find_package( GeoModelCore REQUIRED ) 
  find_package( GeoModelIO REQUIRED )
endif()

# Find includes in current dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Populate a CMake variable with the sources
set(SRCS1 main1.cpp )
set(SRCS2 main2.cpp )
set(SRCS3 main3.cpp )

# Tell CMake to create the helloworld executable
add_executable( hellogeoRead_1 ${SRCS1} )
add_executable( hellogeoRead_2 ${SRCS2} )
add_executable( hellogeoRead_3 ${SRCS3} )

# Link all needed libraries
target_link_libraries( hellogeoRead_1 GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel GeoModelIO::GeoModelIOHelpers GeoModelIO::GeoModelWrite)
target_link_libraries( hellogeoRead_2 GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel GeoModelIO::GeoModelIOHelpers GeoModelIO::GeoModelWrite)
target_link_libraries( hellogeoRead_3 GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel)

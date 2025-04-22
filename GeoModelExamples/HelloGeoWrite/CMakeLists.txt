# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: HelloGeoWrite
# author: Riccardo Maria BIANCHI @ CERN - Nov, 2018
################################################################################


project(HelloGeoWrite)

# Compile with C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR) # when building individually
  find_package( GeoModelCore REQUIRED ) 
  find_package( GeoModelIO REQUIRED ) 
endif()

# Find includes in current dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Populate a CMake variable with the sources
set(SRCS main.cpp )

# Tell CMake to create the helloworld executable
add_executable( hellogeoWrite ${SRCS} )

# Link all needed libraries
target_link_libraries( hellogeoWrite GeoModelCore::GeoModelKernel GeoModelIO::GeoModelWrite GeoModelIO::GeoModelIOHelpers)

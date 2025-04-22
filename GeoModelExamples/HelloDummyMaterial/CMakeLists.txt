# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: HelloDummyMaterial
# author: Riccardo Maria BIANCHI @ CERN - Nov, 2019
################################################################################


#project(HelloDummyMaterial)

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
set(SRCS main.cpp )

# Tell CMake to create the helloworld executable
add_executable( helloDummyMaterial ${SRCS} )

# Link all needed libraries
target_link_libraries( helloDummyMaterial GeoModelCore::GeoModelKernel GeoModelIO::GeoModelWrite )

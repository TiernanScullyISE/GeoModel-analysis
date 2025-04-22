# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: HelloGeo
# author: Riccardo Maria BIANCHI @ CERN - Nov, 2018
################################################################################


# Compile with C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#project(HelloGeo)

# Find the needed dependencies, when building individually
  message (${CMAKE_SOURCE_DIR}) 
  message (${PROJECT_SOURCE_DIR})
  
if ( CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR ) # when buildingindividually
   find_package( GeoModelCore REQUIRED  ) 
endif()

# Find includes in current dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Populate a CMake variable with the sources
set(SRCS main.cpp )

# Tell CMake to create the helloworld executable
add_executable( hellogeo ${SRCS} )

# Link all needed libraries
target_link_libraries( hellogeo GeoModelCore::GeoModelKernel)

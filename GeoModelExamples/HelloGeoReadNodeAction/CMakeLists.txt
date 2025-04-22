# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: HelloGeoReadNodeAction
# author: Riccardo Maria BIANCHI @ CERN - May, 2022
################################################################################


#project(HelloGeoRead)

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
FILE(GLOB SRCS *.cxx)
FILE(GLOB HEADERS *.h)


# Tell CMake to create the helloworld executable
add_executable( hellogeoReadNodeAction ${HEADERS} ${SRCS} )

# Link all needed libraries
target_link_libraries( hellogeoReadNodeAction GeoModelIO::GeoModelRead GeoModelCore::GeoModelKernel)

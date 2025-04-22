# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: HelloToyXML
# author: Riccardo Maria BIANCHI @ CERN - Aug, 2020
################################################################################


project(HelloToyXML)


# Set default build options.
set( CMAKE_BUILD_TYPE "Release" CACHE STRING "CMake build mode to use" )
set( CMAKE_CXX_EXTENSIONS FALSE CACHE BOOL "(Dis)allow using GNU extensions" )
set( CMAKE_CXX_STANDARD_REQUIRED TRUE CACHE BOOL
   "Require the specified C++ standard for the build" )
# Setting CMAKE_CXX_FLAGS to avoid "deprecated" warnings
set(CMAKE_CXX_FLAGS "-Wno-deprecated-declarations" ) # very basic

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR) # when building individually
find_package( GeoModelCore REQUIRED )
find_package( GeoModelIO REQUIRED )
endif()

# this is needed, we use it to access GeoModel input data from XML files
# ==> That's why this example cannot be built together with all the others 
#     when using using the `GEOMODEL_BUILD_EXAMPLES` build option
find_package( GeoModelDataManagers REQUIRED )

# Find includes in current dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Populate a CMake variable with the sources
set(SRCS step1_create_store_geo_and_publish_nodes.cpp )
set(SRCS_READ step2_read_geo_and_published_nodes.cpp )

# Tell CMake to create the 'helloToyXML' executable
add_executable( helloToyXML ${SRCS} )
add_executable( read_helloToyXML_and_publishedNodes ${SRCS_READ} )

# Link all needed libraries
target_link_libraries( helloToyXML GeoModelCore::GeoModelKernel GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelWrite GeoModelDataManagers::GeoXmlInpManager )
target_link_libraries( read_helloToyXML_and_publishedNodes GeoModelCore::GeoModelKernel GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead )

include(GNUInstallDirs)
file(GLOB XML_DATA
  "*.xml"
)
install( FILES ${XML_DATA} DESTINATION ${CMAKE_INSTALL_BINDIR} )
file(COPY ${XML_DATA}
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )


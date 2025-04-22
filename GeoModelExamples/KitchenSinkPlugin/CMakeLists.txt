# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: HelloGeo
# author: Riccardo Maria BIANCHI @ CERN - Nov, 2018
################################################################################


# Compile with C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)


# Find the needed dependencies, when building individually
  message (${CMAKE_SOURCE_DIR})
  message (${PROJECT_SOURCE_DIR})

if ( CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR ) # when buildingindividually
   find_package( GeoModelCore REQUIRED  )
endif()


# Find the header and source files.
file( GLOB SOURCES src/*.cxx )

add_library( KitchenSinkPlugin SHARED ${SOURCES} )
target_link_libraries( KitchenSinkPlugin PUBLIC GeoModelCore::GeoModelKernel)

source_group( "src" FILES ${SOURCES} )

set_target_properties( KitchenSinkPlugin PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR} )

# Install the library.
install( TARGETS KitchenSinkPlugin
   EXPORT ${PROJECT_NAME}-export
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   COMPONENT Runtime
   NAMELINK_SKIP )

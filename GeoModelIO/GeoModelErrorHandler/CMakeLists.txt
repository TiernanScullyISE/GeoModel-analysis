# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Set up how CMake should handle file names given to target_sources().
if( POLICY CMP0076 )
   cmake_policy( SET CMP0076 OLD )
endif()

# Find the header files.
file( GLOB HEADERS GeoModelErrorHandler/*.h )

# Set up the library.
add_library( GeoModelErrorHandler INTERFACE )
source_group( "GeoModelErrorHandler" FILES ${HEADERS} )
target_include_directories( GeoModelErrorHandler INTERFACE
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )
target_link_libraries( GeoModelErrorHandler INTERFACE Qt${QT_VERSION}::Core )
if( "${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR
   "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo" )
   target_compile_definitions( GeoModelErrorHandler INTERFACE
      -DQT_NO_DEBUG_OUTPUT )
endif()

# Set up an alias with the same name that you would get by "finding" a pre-built
# version of the library.
add_library( GeoModelIO::GeoModelErrorHandler ALIAS GeoModelErrorHandler )

# Install the library.
install( TARGETS GeoModelErrorHandler
   EXPORT ${PROJECT_NAME}-export
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   COMPONENT Development )
install( FILES ${HEADERS}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GeoModelErrorHandler
   COMPONENT Development )

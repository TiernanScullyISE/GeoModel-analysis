# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
################################################################################
# Package: GeoModelRead
# author: Riccardo Maria BIANCHI <rbianchi@cern.ch> - 2017
# major updates: rbianchi@cern.ch, 2018
################################################################################

# Find the header and source files.
file( GLOB SOURCES src/*.cpp )
file( GLOB HEADERS GeoModelRead/*.h GeoModelRead/*.tpp )

# Set up the library.
add_library( GeoModelRead SHARED ${HEADERS} ${SOURCES} )
target_link_libraries( GeoModelRead PUBLIC
GeoModelKernel GeoModelDBManager TFPersistification GeoModelHelpers )
target_include_directories( GeoModelRead PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )
source_group( "GeoModelRead" FILES ${HEADERS} )
source_group( "src" FILES ${SOURCES} )
set_target_properties( GeoModelRead PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR} )
 
# Set up an alias with the same name that you would get by "finding" a pre-built
# version of the library.
add_library( GeoModelIO::GeoModelRead ALIAS GeoModelRead )

# Install the library.
install(TARGETS GeoModelRead
    EXPORT ${PROJECT_NAME}-export
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT          Runtime
            NAMELINK_COMPONENT Development   # Requires CMake 3.12
)
install( FILES ${HEADERS}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GeoModelRead
   COMPONENT Development )

if (${GEOMODEL_BUILD_TESTING})
   add_executable( ReadGeoStabilityTest util/ReadStabilityTest.cxx )
   target_link_libraries( ReadGeoStabilityTest PRIVATE 
                                    GeoModelCore::GeoModelKernel
                                    GeoModelIO::GeoModelRead 
                                    GeoModelIO::GeoModelDBManager )
   install( TARGETS ReadGeoStabilityTest
            EXPORT ${PROJECT_NAME}-export
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime )

endif()

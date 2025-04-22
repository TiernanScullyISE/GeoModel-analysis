# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: GeoMaterial2G4
################################################################################
project(GeoMaterial2G4 VERSION ${GeoModel_VERSION} LANGUAGES CXX)

# Use the GNU install directory names.
include( GNUInstallDirs )  # it defines CMAKE_INSTALL_LIBDIR

# Find the header and source files.
file( GLOB SOURCES src/*.cxx )
file( GLOB HEADERS GeoMaterial2G4/*.h )

# Set target and properties
add_library( GeoMaterial2G4 SHARED ${HEADERS} ${SOURCES} )
target_include_directories( GeoMaterial2G4
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )
target_link_libraries( GeoMaterial2G4 PUBLIC GeoModelCore::GeoModelKernel ${Geant4_LIBRARIES} )

# Set installation of library headers
set_property( TARGET GeoMaterial2G4 PROPERTY PUBLIC_HEADER ${HEADERS} )

set_target_properties( GeoMaterial2G4 PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR} )

# new test GeoModelG4
install(TARGETS GeoMaterial2G4
  EXPORT GeoMaterial2G4-export
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GeoMaterial2G4 )

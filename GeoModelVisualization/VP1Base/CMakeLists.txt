# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Find the header and source files.
file( GLOB SOURCES src/*.cxx )
file( GLOB HEADERS VP1Base/*.h )
file( GLOB UIS src/*.ui )
file( GLOB RCS src/*.qrc )

# Add the library.
add_library( GXBase SHARED ${SOURCES} ${HEADERS} ${UIS} ${RCS} )
if (GEOMODEL_USE_QT6)
  target_link_libraries( GXBase
     PUBLIC Coin::Coin Qt${QT_VERSION}::Core Qt${QT_VERSION}::Gui Qt${QT_VERSION}::Widgets Qt${QT_VERSION}::Core5Compat SoQt::SoQt
     PRIVATE Qt${QT_VERSION}::PrintSupport Qt${QT_VERSION}::OpenGL OpenGL::EGL GXHepVis )
else()
  target_link_libraries( GXBase
     PUBLIC Coin::Coin Qt${QT_VERSION}::Core Qt${QT_VERSION}::Gui Qt${QT_VERSION}::Widgets SoQt::SoQt
     PRIVATE Qt${QT_VERSION}::PrintSupport Qt${QT_VERSION}::OpenGL OpenGL::EGL GXHepVis )
endif()

target_include_directories( GXBase PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )
target_compile_definitions( GXBase PUBLIC
   -DQT_NO_DEBUG_OUTPUT -DQT_NO_INFO_OUTPUT )
source_group( "VP1Base" FILES ${HEADERS} )
source_group( "src" FILES ${SOURCES} ${UIS} ${RCS} )
set_target_properties( GXBase PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR} )
if( GEOMODEL_USE_BUILTIN_COIN3D )
   add_dependencies( GXBase Coin3DBuiltIn SoQtBuiltIn )
endif()

# Set up an alias with the same name that you would get by "finding" a pre-built
# version of the library.
add_library( GeoModelVisualization::GXBase ALIAS GXBase )

# Install the library.
install( TARGETS GXBase
   EXPORT ${PROJECT_NAME}-export
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   COMPONENT Runtime
   NAMELINK_COMPONENT Development )
install( FILES ${HEADERS}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VP1Base
   COMPONENT Development )

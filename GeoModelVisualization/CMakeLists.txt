# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# CMake settings
cmake_minimum_required(VERSION 3.26...4.0)
set(CMAKE_CXX_STANDARD 20)
# Dummy call to 'project()', needed to set 'PROJECT_SOURCE_DIR'
project( "GeoModelTools" )

#Set up the project. Check if we build it with GeoModel or individually
project( "GeoModelVisualization" )


# Set the OpenGL flag requested by CMake
set( OpenGL_GL_PREFERENCE GLVND )
if (UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL GLX)
endif()

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    # I am top-level project.
    # Make the root module directory visible to CMake.
    list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake )
    # get global GeoModel version
    include( GeoModel-version ) 
    # set the project, with the version taken from the GeoModel parent project
    project( "GeoModelVisualization" VERSION ${GeoModel_VERSION} LANGUAGES CXX )

    set (QT_VERSION 6)

    # Define color codes for CMake messages
    include( cmake_colors_defs )
    # Warn the users about what they are doing
    message(STATUS "${BoldGreen}Building ${PROJECT_NAME} individually, as a top-level project.${ColourReset}")
    # Set a default build type
    include( BuildType )
    # Set default build and C++ options
    include( configure_cpp_options )
    set( CMAKE_FIND_FRAMEWORK "LAST" CACHE STRING
         "Framework finding behaviour on macOS" )
    # Find the base GeoModel packages, which must be installed on the target system already
    find_package( GeoModelCore REQUIRED ${GeoModel_VERSION} )
    find_package( GeoModelIO REQUIRED ${GeoModel_VERSION} )
    # Print Build Info on screen
    include( PrintBuildInfo )
else()
    # I am called from other project with add_subdirectory().
    message( STATUS "Building ${PROJECT_NAME} as part of the root GeoModel project.")
    # Set the project
    project( "GeoModelVisualization" VERSION ${GeoModel_VERSION} LANGUAGES CXX )
endif()

# Find the dependencies of the project.

find_package( Qt${QT_VERSION} COMPONENTS Core Gui Widgets Sql PrintSupport OpenGL Core5Compat REQUIRED )
find_package( OpenGL REQUIRED )
find_package( HDF5 REQUIRED COMPONENTS CXX )
# Set up how the project should handle some of its dependenices. Either by
# picking them up from the environment, or building them itself.
include( SetupCoin3D )
include( SetupJSON )

# Qt${QT_VERSION} settings.
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTOUIC ON )
set( CMAKE_AUTORCC ON )

# Use the GNU install directory names.
include( GNUInstallDirs )

# Set sub-packages to build.
add_subdirectory(VP1GeometryPlugin)
add_subdirectory(VP1GeometrySystems)
add_subdirectory(GXClashPointSystems)
add_subdirectory(GeoModelExplorer)
add_subdirectory(VP1Gui)
add_subdirectory(VP1Base)
add_subdirectory(GXHepVis)
add_subdirectory(VP1GuideLineSystems)
add_subdirectory(GXHitDisplaySystems)
add_subdirectory(GXTrackDisplaySystems)
add_subdirectory(GmGraphBrowser)

# Create and install the version description of the project.
include( WriteBasicConfigVersionFile )
write_basic_config_version_file(
   ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}ConfigVersion.cmake
   VERSION ${PROJECT_VERSION}
   COMPATIBILITY SameMajorVersion )
install(
   FILES
   ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}ConfigVersion.cmake
   COMPONENT Development
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} )

# Create and install the description of the libraries.
export( EXPORT ${PROJECT_NAME}-export
   NAMESPACE "${PROJECT_NAME}::"
   FILE ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Targets.cmake )
install( EXPORT ${PROJECT_NAME}-export
   FILE ${PROJECT_NAME}Targets.cmake
   COMPONENT Development
   NAMESPACE "${PROJECT_NAME}::"
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} )

# Install the hand-written project configuration.
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
   ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake
   @ONLY )
install(
   FILES
   ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}Config.cmake
   COMPONENT Development
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME} )

# Set up the packaging of the project using CPack.
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )
include( GeoModelVisualizationPackaging )

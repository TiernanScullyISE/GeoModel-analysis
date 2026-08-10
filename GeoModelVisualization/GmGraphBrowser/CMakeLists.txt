# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

cmake_minimum_required(VERSION 3.26...4.0)

project(gmgraphbrowser VERSION 0.1 LANGUAGES CXX)

file( GLOB MANPAGES man/man1/* )

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    # I am top-level project.
    # Make the root module directory visible to CMake.
    list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake )
    # get global GeoModel version
    include( GeoModel-version ) 
    # set the project, with the version taken from the GeoModel parent project
    project( "gmgraphbrowser" VERSION ${GeoModel_VERSION} LANGUAGES CXX )

    set (QT_VERSION 6)

    # Define color codes for CMake messages
    include( cmake_colors_defs )
    # Warn the users about what they are doing
    message(STATUS "${BoldGreen}Building ${PROJECT_NAME} individually, as a top-level project.${ColourReset}")
    # Set default build and C++ options
    include( configure_cpp_options )
    set( CMAKE_FIND_FRAMEWORK "LAST" CACHE STRING
         "Framework finding behaviour on macOS" )
    # Set up how the project handle some of its dependenices. Either by picking them
    # up from the environment, or building them itself.
    find_package (GeoModelIO REQUIRED)
else()
    # I am called from other project with add_subdirectory().
    message( STATUS "Building ${PROJECT_NAME} as part of the root GeoModel project.")
    # Set the project
    project( "gmgraphbrowser" VERSION ${GeoModel_VERSION} LANGUAGES CXX )
endif()


set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find the dependencies of the project.
find_package( Qt${QT_VERSION} COMPONENTS Core Gui Widgets REQUIRED )

# Qt${QT_VERSION} settings.
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Use the GNU install directory names.
include( GNUInstallDirs )


file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB uis ${PROJECT_SOURCE_DIR}/src/*.ui)
set(PROJECT_SOURCES ${sources} ${headers}  ${uis} )


add_executable(gmgraphbrowser ${PROJECT_SOURCES} )
target_link_libraries(gmgraphbrowser PRIVATE Qt${QT_VERSION}::Widgets GeoModelIO::GeoModelIOHelpers GeoModelCore::GeoModelKernel)

# Install the executable.
install( TARGETS gmgraphbrowser
   EXPORT ${PROJECT_NAME}-export
   LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
   COMPONENT Runtime )


install( FILES ${MANPAGES}
   DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
   COMPONENT Runtime )

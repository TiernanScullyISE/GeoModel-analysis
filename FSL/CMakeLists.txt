# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

cmake_minimum_required(VERSION 3.16...4.0)

project(fsl VERSION 0.1 LANGUAGES CXX)

file( GLOB MANPAGES man/man1/* )

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    # I am top-level project.
    # Make the root module directory visible to CMake.
    list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake )
    # get global GeoModel version
    include( GeoModel-version ) 
    # set the project, with the version taken from the GeoModel parent project
    project( "fsl" VERSION ${GeoModel_VERSION} LANGUAGES CXX )

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
    include( SetupJSON )
    find_package (FullSimLight REQUIRED)
    add_definitions (-DFSLPLUGINROOT="${FullSimLight_DIR}/../../../${CMAKE_INSTALL_DATAROOTDIR}" )
    message  ( STATUS "FSL plugin files sought here: ${FullSimLight_DIR}/../../../${CMAKE_INSTALL_DATAROOTDIR}" )
    if(FullSimLight_HAS_HEPMC3)
	    add_compile_definitions( USE_HEPMC3 )
	    message (STATUS "HEPMC3 Enabled in FullSimLight")
    else()
        message (STATUS "HEPMC3 Disabled in FullSimLight")
    endif()
    if(FullSimLight_HAS_Pythia)
	    add_compile_definitions( USE_Pythia )
	    message (STATUS "Pythia Enabled in FullSimLight")
    else()
        message (STATUS "Pythia Disabled in FullSimLight")
    endif()	
else()
    # I am called from other project with add_subdirectory().
    message( STATUS "Building ${PROJECT_NAME} as part of the root GeoModel project.")
    # Set the project
    project( "fsl" VERSION ${GeoModel_VERSION} LANGUAGES CXX )
    add_definitions (-DFSLPLUGINROOT="${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}" )
    if(GEOMODEL_USE_HEPMC3)
	    add_compile_definitions( USE_HEPMC3 )
    endif()
    if(GEOMODEL_USE_PYTHIA)
	    add_compile_definitions( USE_Pythia )
    endif()
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find the dependencies of the project.
find_package( Qt${QT_VERSION} COMPONENTS Core Gui Widgets Sql PrintSupport OpenGL REQUIRED )

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


add_executable(fsl ${PROJECT_SOURCES} )
target_link_libraries(fsl PRIVATE Qt${QT_VERSION}::Widgets nlohmann_json::nlohmann_json)

# Install the executable.
install( TARGETS fsl
   EXPORT ${PROJECT_NAME}-export
   LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
   COMPONENT Runtime )


install( FILES ${MANPAGES}
   DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
   COMPONENT Runtime )

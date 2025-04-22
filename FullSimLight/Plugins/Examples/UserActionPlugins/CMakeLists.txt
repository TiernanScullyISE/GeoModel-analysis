# (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Set up the project.
project( "UserActionsPlugins" )

#Set up the project. Check if we build it with GeoModel or individually
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    # I am built as a top-level project.
    # Make the root module directory visible to CMake.
    list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../cmake )
    # get global GeoModel version
    include( GeoModel-version ) 
    # set the project, with the version taken from the GeoModel parent project
    project( "FSLUserActionsPlugins" VERSION ${GeoModel_VERSION} LANGUAGES CXX )
    # Define color codes for CMake messages
    include( cmake_colors_defs )
    # Warn the users about what they are doing
    message(STATUS "${BoldGreen}Building ${PROJECT_NAME} individually, as a top-level project.${ColourReset}")
    # Set default build and C++ options
    include( configure_cpp_options )
    set( CMAKE_FIND_FRAMEWORK "LAST" CACHE STRING
         "Framework finding behaviour on macOS" )
    # Set up how the project handle some of its dependencies. Either by picking them
    # up from the environment, or building them itself
    find_package( GeoModelCore REQUIRED )
    find_package( FullSimLight REQUIRED )
else()
    # I am called from other project with add_subdirectory().
    message( STATUS "Building ${PROJECT_NAME} as part of the root GeoModel project.")
    # Set the project
    project( "FSLUserActionsPlugins" VERSION ${GeoModel_VERSION} LANGUAGES CXX )
endif()


# Project's dependencies.
# GeoModel dependencies

# Use the GNU install directory names.
include( GNUInstallDirs )

# Set sub-packages to build.
add_subdirectory( DummyUserActionPlugin )


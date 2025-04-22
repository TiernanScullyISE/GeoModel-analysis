# Set up the project.


project( "GenerateTracksPlugin" )

#Set up the project. Check if we build it with GeoModel or individually
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    # I am built as a top-level project.
    # Make the root module directory visible to CMake.
    list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )
    # get global GeoModel version
    #include( GeoModelATLAS-version ) 
    # set the project, with the version taken from the GeoModel parent project
    project( "GenerateTracksPlugin" VERSION 1.0 LANGUAGES CXX )
    # Define color codes for CMake messages
    include( cmake_colors_defs )
    # Use the GNU install directory names.
    include( GNUInstallDirs )
    # Set a default build type
    include( BuildType )
    # Set default build and C++ options
    include( configure_cpp_options )
    # Print Build Info on screen
    include( PrintBuildInfo )
    # Warn the users about what they are doing
    message(STATUS "${BoldGreen}Building ${PROJECT_NAME} individually, as a top-level project.${ColourReset}")
    # Set default build and C++ options
    include( configure_cpp_options )
    set( CMAKE_FIND_FRAMEWORK "LAST" CACHE STRING
         "Framework finding behaviour on macOS" )
    # GeoModel dependencies
    find_package( GeoModelCore REQUIRED )
    find_package( FullSimLight REQUIRED )
else()
    # I am called from other project with add_subdirectory().
    message( STATUS "Building ${PROJECT_NAME} as part of the root project.")
    # Set the project
    project( "GenerateTracksPlugin" VERSION 1.0 LANGUAGES CXX )
endif()



# Find the header and source files.
file( GLOB HEADERS include/*.hh)
file( GLOB SOURCES src/*.cc )

set(PROJECT_SOURCES ${HEADERS} ${SOURCES} TracksPlugin.cc)

# Set up the library.
add_library(GenerateTracksPlugin SHARED ${HEADERS} ${SOURCES} TracksPlugin.cc)

find_package(Geant4 REQUIRED)
find_package( HDF5 REQUIRED COMPONENTS CXX )

message( STATUS "Found Geant4: ${Geant4_INCLUDE_DIR}")

# Use the GNU install directory names.
include( GNUInstallDirs )

# Add the FullSimLight headers dir.
target_include_directories( GenerateTracksPlugin PUBLIC ${HDF5_CXX_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries ( GenerateTracksPlugin PUBLIC FullSimLight::FullSimLight ${CMAKE_DL_LIBS} ${Geant4_LIBRARIES} ${HDF5_CXX_LIBRARIES})

set_target_properties( GenerateTracksPlugin PROPERTIES
		       VERSION ${PROJECT_VERSION}
		       SOVERSION ${PROJECT_VERSION_MAJOR} )

install( TARGETS GenerateTracksPlugin
	 LIBRARY DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/FullSimLight/UserActionPlugins
	 COMPONENT Runtime
	 NAMELINK_COMPONENT Development 
	 )




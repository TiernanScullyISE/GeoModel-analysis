# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
################################################################################
# Package: GeoModelIOHelpers
# author: Riccardo Maria BIANCHI <rbianchi@cern.ch> - 2023 Jan
################################################################################

# Find the header and source files.
file( GLOB SOURCES src/*.cpp )
file( GLOB HEADERS GeoModelIOHelpers/*.h )

# Set up the library.
add_library( GeoModelIOHelpers SHARED ${HEADERS} ${SOURCES} )
target_link_libraries( GeoModelIOHelpers PUBLIC
    GeoModelRead GeoModelWrite )
# We link those to carry on the needed libs when including GeoModelIOHelpers,
# even if the latter is headers only
target_include_directories( GeoModelIOHelpers PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )
source_group( "GeoModelIOHelpers" FILES ${HEADERS} )
source_group( "src" FILES ${SOURCES} )
set_target_properties( GeoModelIOHelpers PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR} )
set_target_properties(GeoModelIOHelpers PROPERTIES LINKER_LANGUAGE CXX)

# Set up custom build flags for the library.
foreach( _flag GEOMODEL_IO_READ_DEBUG GEOMODEL_IO_DEBUG_VERBOSE
   GEOMODEL_IO_READ_TIMING )
   if( ${${_flag}} )
      target_compile_definitions( GeoModelIOHelpers PRIVATE ${_flag}=true )
   endif()
endforeach()

# Set up an alias with the same name that you would get by "finding" a pre-built
# version of the library.
add_library( GeoModelIO::GeoModelIOHelpers ALIAS GeoModelIOHelpers )

# Install the library.
install(TARGETS GeoModelIOHelpers
    EXPORT ${PROJECT_NAME}-export
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT          Runtime
            NAMELINK_COMPONENT Development   # Requires CMake 3.12
)
install( FILES ${HEADERS}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GeoModelIOHelpers
   COMPONENT Development )

include(CheckCXXCompilerFlag)
if( BUILD_TESTING AND GEOMODEL_BUILD_TESTING )

    # add "-Wno-character-conversion" if supported by the compiler
    check_cxx_compiler_flag(
        "-Wno-character-conversion"
        GEOMODEL_HAS_WNO_CHARACTER_CONVERSION
    )
    if(GEOMODEL_HAS_WNO_CHARACTER_CONVERSION)
        add_compile_options(-Wno-character-conversion)
    endif()

    # global I/O tests
      file(GLOB_RECURSE files "tests/*.cxx")
    foreach(_exeFile ${files})
      get_filename_component(_theExec ${_exeFile} NAME_WE)
      get_filename_component(_theLoc ${_exeFile} DIRECTORY)
  
    if(${_theLoc} MATCHES "DoNotBuild")
      continue()
    endif() 
  
    add_executable(${_theExec} ${_exeFile})
    target_link_libraries( ${_theExec} GeoModelIO::GeoModelDBManager GeoModelCore::GeoModelHelpers GeoModelCore::GeoModelKernel GeoModelIO::GeoModelIOHelpers GTest::GTest GTest::Main pthread )
    add_test(NAME ${_theExec}
             COMMAND ${_theExec})

    endforeach()
endif()

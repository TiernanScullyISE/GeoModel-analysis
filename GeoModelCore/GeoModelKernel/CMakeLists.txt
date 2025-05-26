# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Find the header and source files.
file( GLOB SOURCES src/*.cxx )
file( GLOB HEADERS GeoModelKernel/*.h GeoModelKernel/*.tpp GeoModelKernel/*.icc )

# Create the library.
add_library( GeoModelKernel SHARED ${HEADERS} ${SOURCES} )
target_link_libraries( GeoModelKernel PUBLIC Eigen3::Eigen GeoGenericFunctions
   ${CMAKE_DL_LIBS} )
target_include_directories( GeoModelKernel PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )
source_group( "GeoModelKernel" FILES ${HEADERS} )
source_group( "src" FILES ${SOURCES} )
set_target_properties( GeoModelKernel PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR} )

# Code in this file makes heavy use of eigen and runs orders of magnitude
# more slowly without optimization.  So force this to be optimized even
# in debug builds.  If you need to debug it you might want to change this.
# Specifying optimization via an attribute on the particular
# function didn't work, because that still didn't allow inlining.
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/GeoXF.cxx
     ${CMAKE_CURRENT_SOURCE_DIR}/src/GeoAlignableTransform.cxx
     ${CMAKE_CURRENT_SOURCE_DIR}/src/GeoShapeShift.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
     COMPILE_DEFINITIONS "FLATTEN" )
endif()

# Set up an alias with the same name that you would get by "finding" a pre-built
# version of the library.
add_library( GeoModelCore::GeoModelKernel ALIAS GeoModelKernel )

# Install the library.
install(TARGETS GeoModelKernel
    EXPORT ${PROJECT_NAME}-export
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT          Runtime
            NAMELINK_COMPONENT Development   # Requires CMake 3.12
)
install( FILES ${HEADERS}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GeoModelKernel
   COMPONENT Development )


if( BUILD_TESTING AND GEOMODEL_BUILD_TESTING )
    file(GLOB_RECURSE files "tests/*.cxx")
    foreach(_exeFile ${files})
      get_filename_component(_theExec ${_exeFile} NAME_WE)
      get_filename_component(_theLoc ${_exeFile} DIRECTORY)

    if(${_theLoc} MATCHES "DoNotBuild")
      continue()
    endif()

    add_executable(${_theExec} ${_exeFile})
    target_link_libraries( ${_theExec} GeoModelKernel GTest::GTest GTest::Main)
    add_test(NAME ${_theExec}
	     COMMAND ${_theExec})
    endforeach()
    include(GoogleTest)
    gtest_discover_tests(testRCBase)
    gtest_discover_tests(testGeoGraphNode)
    gtest_discover_tests(testGeoMaterial)
    gtest_discover_tests(testGeoElement)
    gtest_discover_tests(testGeoVolumeAction)
endif()

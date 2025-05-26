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

if( BUILD_TESTING AND GEOMODEL_BUILD_TESTING )
    # global I/O tests
    add_executable(test_io_shared_serialtransformers tests/test_io_shared_serialtransformers.cpp)
    target_link_libraries( test_io_shared_serialtransformers GeoModelIO::GeoModelDBManager GeoModelCore::GeoModelHelpers GeoModelCore::GeoModelKernel GeoModelIO::GeoModelIOHelpers)
    add_test(NAME test_IO_SharedSerialTransformers
    	     COMMAND test_io_shared_serialtransformers)

    # I/O Unit tests
    add_executable(test_io_shapes_unidentifiedshape tests/test_io_UnidentifiedShape.cpp)
    target_link_libraries( test_io_shapes_unidentifiedshape GeoModelIO::GeoModelDBManager GeoModelCore::GeoModelHelpers GeoModelCore::GeoModelKernel GeoModelIO::GeoModelIOHelpers)
    add_test(NAME test_IO_Shapes_UnidentifiedShape
             COMMAND test_io_shapes_unidentifiedshape)

    add_executable(test_io_shapes_ellipticaltube tests/test_io_shape_EllipticalTube.cpp)
    target_link_libraries( test_io_shapes_ellipticaltube GeoModelIO::GeoModelDBManager GeoModelCore::GeoModelHelpers GeoModelCore::GeoModelKernel GeoModelIO::GeoModelIOHelpers)
    add_test(NAME test_IO_Shapes_EllipticalTube
             COMMAND test_io_shapes_ellipticaltube)
endif()

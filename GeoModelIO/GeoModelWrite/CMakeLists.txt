# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Find the header and source files.
file( GLOB SOURCES src/*.cpp )
file( GLOB HEADERS GeoModelWrite/*.h)

# Set up the library.
add_library( GeoModelWrite SHARED ${HEADERS} ${SOURCES} )
target_link_libraries( GeoModelWrite PUBLIC
   GeoModelKernel GeoModelDBManager TFPersistification GeoModelHelpers)
target_include_directories( GeoModelWrite PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )
source_group( "GeoModelWrite" FILES ${HEADERS} )
source_group( "src" FILES ${SOURCES} )
set_target_properties( GeoModelWrite PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR} )

# Set up an alias with the same name that you would get by "finding" a pre-built
# version of the library.
add_library( GeoModelIO::GeoModelWrite ALIAS GeoModelWrite )

# Install the library.
install(TARGETS GeoModelWrite
    EXPORT ${PROJECT_NAME}-export
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT          Runtime
            NAMELINK_COMPONENT Development   # Requires CMake 3.12
)
install( FILES ${HEADERS}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GeoModelWrite
   COMPONENT Development )

if( BUILD_TESTING AND GEOMODEL_BUILD_TESTING)
    add_compile_options(-Wno-character-conversion)
    add_executable(testWriteGeoModel 
    	tests/testWriteGeoModel.cxx 
    )
    target_link_libraries( testWriteGeoModel GeoModelWrite GTest::GTest GTest::Main)
    add_test(NAME testWriteGeoModel 
    	COMMAND testWriteGeoModel
    )

    include(GoogleTest)
    gtest_discover_tests(testWriteGeoModel)
endif()

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
################################################################################
# Package: GeoModelDBManager
# author: Riccardo Maria BIANCHI @ CERN - 2017
# major updates:
# - R.M.Bianchi, 2018
# - R.M.Bianchi, Nov 2020
# - R.M.Bianchi, Apr 2024 -- Add CTest tests
# - R.M.Bianchi, Feb 2025 -- Add GoogleTest support
################################################################################

# Find the header and source files.
file( GLOB SOURCES src/*.cpp )
file( GLOB HEADERS GeoModelDBManager/*.h )

# Set up the library.
add_library( GeoModelDBManager SHARED ${HEADERS} ${SOURCES} )
target_link_libraries( GeoModelDBManager PRIVATE SQLite::SQLite3 GeoModelHelpers GeoModelKernel )
target_include_directories( GeoModelDBManager PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )
source_group( "GeoModelDBManager" FILES ${HEADERS} )
source_group( "src" FILES ${SOURCES} )
set_target_properties( GeoModelDBManager PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR} )

# Set up an alias with the same name that you would get by "finding" a pre-built
# version of the library.
add_library( GeoModelIO::GeoModelDBManager ALIAS GeoModelDBManager )

# Install the library.
install(TARGETS GeoModelDBManager
    EXPORT ${PROJECT_NAME}-export
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT          Runtime
            NAMELINK_COMPONENT Development   # Requires CMake 3.12
)
install( FILES ${HEADERS}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GeoModelDBManager
   COMPONENT Development )


### --- Add Tests ---
if( BUILD_TESTING AND GEOMODEL_BUILD_TESTING )
    #++ Create a 'simple' unit test
    add_executable(test_create_db_file tests/test_create_db_file.cpp)
    target_link_libraries( test_create_db_file GeoModelIO::GeoModelDBManager)
    add_test(NAME testCreateDBFile
    	     COMMAND test_create_db_file)

    # Create test executable
    add_executable(testGMDBManager tests/testGMDBManager.cpp)
    target_link_libraries(testGMDBManager GeoModelDBManager  GTest::GTest GTest::Main GeoModelKernel)
    add_test(NAME testGMDBManager COMMAND testGMDBManager)
    #
    add_executable(testGMDBManagerQuery tests/testGMDBManagerQuery.cpp)
    target_link_libraries(testGMDBManagerQuery GeoModelDBManager  GTest::GTest GTest::Main GeoModelKernel)
    add_test(NAME testGMDBManagerQuery COMMAND testGMDBManagerQuery)
    #
    add_executable(testGMSQLiteConnection tests/testGMSQLiteConnection.cpp)
    target_link_libraries(testGMSQLiteConnection GeoModelDBManager GTest::GTest GTest::Main SQLite::SQLite3 GeoModelKernel )
    add_test(NAME testGMSQLiteConnection COMMAND testGMSQLiteConnection)
    #
    add_executable(testGMSQLiteConnectionQuery tests/testGMSQLiteConnectionQuery.cpp)
    target_link_libraries(testGMSQLiteConnectionQuery GeoModelDBManager GTest::GTest GTest::Main SQLite::SQLite3 GeoModelKernel )
    add_test(NAME testGMSQLiteConnectionQuery COMMAND testGMSQLiteConnectionQuery)
endif()

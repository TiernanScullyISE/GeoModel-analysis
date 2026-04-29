# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# === Preamble ===
cmake_minimum_required(VERSION 3.16...4.0)


file( GLOB MANPAGES man/man1/* )

find_package( SQLite3 3.7.17 )
if(NOT TARGET SQLite3::SQLite3) # CMake < 4.3
  add_library(SQLite3::SQLite3 ALIAS SQLite::SQLite3)
endif()

# Declare the package's executable.
add_executable( gmintegritycheck src/gmintegritycheck.cxx  src/GeoIntegrityCheckAction.cxx )
target_link_libraries( gmintegritycheck PRIVATE SQLite3::SQLite3
		       			    GeoModelCore::GeoModelKernel
                                            GeoModelIO::GeoModelRead 
                                            GeoModelIO::GeoModelWrite
					    GeoModelIO::GeoModelIOHelpers
                                            GeoModelIO::GeoModelDBManager
                                            GeoModelCore::GeoModelHelpers)

# Tweak how debug information should be attached to the executable, in Debug
# builds.
if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" AND
   "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" )
   target_compile_options( gmintegritycheck PRIVATE "-gdwarf-2" )
endif()

# Install the executable.
install( TARGETS gmintegritycheck
   EXPORT ${PROJECT_NAME}-export
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   COMPONENT Runtime )


install( FILES ${MANPAGES}
   DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
   COMPONENT Runtime )

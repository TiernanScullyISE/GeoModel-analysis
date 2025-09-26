# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration



file( GLOB MANPAGES man/man1/* )

# Declare the package's executable.
add_executable( gmstatistics src/gmstatistics.cxx  src/GeoInventoryGraphAction.cxx )
target_link_libraries( gmstatistics PRIVATE GeoModelCore::GeoModelKernel
                                            GeoModelIO::GeoModelRead 
                                            GeoModelIO::GeoModelWrite
                                            GeoModelIO::GeoModelDBManager
                                            GeoModelCore::GeoModelHelpers )

# Tweak how debug information should be attached to the executable, in Debug
# builds.
if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" AND
   "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" )
   target_compile_options( gmstatistics PRIVATE "-gdwarf-2" )
endif()

# Install the executable.
install( TARGETS gmstatistics
   EXPORT ${PROJECT_NAME}-export
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   COMPONENT Runtime )


install( FILES ${MANPAGES}
   DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
   COMPONENT Runtime )

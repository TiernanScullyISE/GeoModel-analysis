# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration


file( GLOB MANPAGES man/man1/* )

# Declare the package's executable.
add_executable( gmcat src/gmcat.cxx src/publishMetaData.cpp)
target_link_libraries( gmcat PRIVATE GeoModelCore::GeoModelKernel
                                     GeoModelCore::GeoModelHelpers
                                     GeoModelIO::GeoModelRead 
                                     GeoModelIO::GeoModelWrite
                                     GeoModelIO::GeoModelDBManager )

# Tweak how debug information should be attached to the executable, in Debug
# builds.
if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" AND
   "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" )
   target_compile_options( gmcat PRIVATE "-gdwarf-2" )
endif()
target_compile_options( gmcat PRIVATE "-DGMVERSION=${GeoModel_VERSION}" )

# Install the executable.
install( TARGETS gmcat
   EXPORT ${PROJECT_NAME}-export
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   COMPONENT Runtime )

install( FILES ${MANPAGES}
   DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
   COMPONENT Runtime )

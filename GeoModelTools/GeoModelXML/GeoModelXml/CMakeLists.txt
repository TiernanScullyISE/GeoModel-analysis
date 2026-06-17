# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Find the header and source files.
file( GLOB SOURCES src/*.cxx )
file( GLOB HEADERS GeoModelXml/*.h )
file( GLOB SHAPEHEADERS GeoModelXml/shape/*.h )
file( GLOB DATA data/*.dtd )

# External dependencies
find_package( ZLIB REQUIRED )
find_package( XercesC  REQUIRED ) 

# Create the library.
add_library( GeoModelXml SHARED ${HEADERS} ${SOURCES} )
# link libraries
target_link_libraries( GeoModelXml PUBLIC GeoModelCore::GeoModelKernel 
                                          GeoModelCore::GeoModelHelpers 
                                          GeoModelTools::ExpressionEvaluator 
                                          XercesC::XercesC 
                                   PRIVATE ZLIB::ZLIB  )
target_include_directories( GeoModelXml PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )
source_group( "GeoModelXml" FILES ${HEADERS} )
source_group( "src" FILES ${SOURCES} )
set_target_properties( GeoModelXml PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR} )

# Set up an alias with the same name that you would get by "finding" a pre-built
# version of the library.
add_library( GeoModelTools::GeoModelXml ALIAS GeoModelXml )

# Install the library.
install(TARGETS GeoModelXml
    EXPORT ${PROJECT_NAME}-export
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT          Runtime
            NAMELINK_COMPONENT Development   # Requires CMake 3.12
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT Development
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GeoModelXml
            COMPONENT Development
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

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


    file(GLOB_RECURSE files "tests/*.cxx")
    foreach(_exeFile ${files})
      get_filename_component(_theExec ${_exeFile} NAME_WE)
      get_filename_component(_theLoc ${_exeFile} DIRECTORY)

    if(${_theLoc} MATCHES "DoNotBuild")
      continue()
    endif()

    add_executable(${_theExec} ${_exeFile})
    
    target_link_libraries( ${_theExec} GeoModelXml GTest::GTest GTest::Main)
    
    add_test(NAME ${_theExec}
	     COMMAND ${_theExec})
    endforeach()
endif()


install( FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GeoModelXml
    COMPONENT Development )
install( FILES ${SHAPEHEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/GeoModelXml/shape)
install( FILES ${DATA} DESTINATION ${CMAKE_INSTALL_DATADIR}/GeoModelXml)

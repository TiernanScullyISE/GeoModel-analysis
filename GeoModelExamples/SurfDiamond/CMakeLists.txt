
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Find the needed dependencies, when building individually
if ( CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR ) # when buildingindividually
   find_package( GeoModelCore REQUIRED  )
endif()

# Find the header and source files.
file( GLOB SOURCES src/*.cxx )

add_library( SurfDiamond SHARED ${SOURCES} )
target_link_libraries( SurfDiamond PUBLIC GeoModelCore::GeoModelKernel GeoModelCore::GeoModelHelpers)

source_group( "src" FILES ${SOURCES} )

set_target_properties( SurfDiamond PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR} )

# Install the library.
install( TARGETS SurfDiamond
   EXPORT ${PROJECT_NAME}-export
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   COMPONENT Runtime
   NAMELINK_SKIP )

# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: HelloGeoWriteReadWrite
# author: Riccardo Maria BIANCHI @ CERN - May, 2022
################################################################################


project(SharedNodes)

# Compile with C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR) # when building individually
  find_package( GeoModelCore REQUIRED ) 
  find_package( GeoModelIO REQUIRED ) 
endif()

# Find includes in current dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Populate a CMake variable with the sources
set(HDRS world.h)

add_executable( test_shared_shapes shared_shapes.cpp ${HDRS} )
target_link_libraries( test_shared_shapes GeoModelCore::GeoModelKernel GeoModelIO::GeoModelWrite GeoModelIO::GeoModelRead)

add_executable( test_shared_logvols shared_logvols.cpp ${HDRS} )
target_link_libraries( test_shared_logvols GeoModelCore::GeoModelKernel GeoModelIO::GeoModelWrite GeoModelIO::GeoModelRead)

add_executable( test_shared_serialtransformers shared_serialtransformers.cpp ${HDRS} )
target_link_libraries( test_shared_serialtransformers GeoModelCore::GeoModelKernel GeoModelIO::GeoModelWrite GeoModelIO::GeoModelRead)

add_executable( test_shared_serialtransformers_2 shared_serialtransformers_2.cpp ${HDRS} )
target_link_libraries( test_shared_serialtransformers_2 GeoModelCore::GeoModelKernel GeoModelIO::GeoModelWrite GeoModelIO::GeoModelRead)

add_executable( test_shared_serialtransformers_3 shared_serialtransformers_3.cpp ${HDRS} )
target_link_libraries( test_shared_serialtransformers_3 GeoModelCore::GeoModelKernel GeoModelIO::GeoModelWrite GeoModelIO::GeoModelRead)

add_executable( test_shared_physvols shared_physvols.cpp ${HDRS} )
target_link_libraries( test_shared_physvols GeoModelCore::GeoModelKernel GeoModelIO::GeoModelWrite GeoModelIO::GeoModelRead)

add_executable( test_shared_nametags shared_nametags.cpp ${HDRS} )
target_link_libraries( test_shared_nametags GeoModelCore::GeoModelKernel GeoModelIO::GeoModelWrite GeoModelIO::GeoModelRead)


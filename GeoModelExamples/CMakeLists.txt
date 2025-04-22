# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: GeoModelExamples
# author: Riccardo Maria BIANCHI @ CERN - Nov, 2018
################################################################################

# Set up the project.
project( "GeoModelExamples" VERSION 4.1.0 LANGUAGES CXX )

# Getting-started examples
add_subdirectory( KitchenSinkPlugin )
add_subdirectory( MinimalPlugin )
add_subdirectory( SurfaceTestPlugin )
add_subdirectory( SurfAlignTestPlugin )
add_subdirectory( SurfAnnulusDemo )
add_subdirectory( SiliconSystemExample )
add_subdirectory( SurfDiamond )

add_subdirectory( HelloGeo )
# add_subdirectory( HelloGeoWrite )
# add_subdirectory( HelloGeoWriteReadWrite )
# add_subdirectory( HelloGeoRead )
# add_subdirectory( HelloGeoReadNodeAction )
# add_subdirectory( SharedNodes )
add_subdirectory( HelloToyWrite )

#add_subdirectory( HelloDummyMaterial )
add_subdirectory( HelloToy )
#add_subdirectory( HelloToyDetectorFactory )
#add_subdirectory( HelloToyXML ) # this should be moved to GeoModelATLAS, because it uses GeoModelDataManagers

# GeoModel reference examples
#add_subdirectory( GeoActions )
#add_subdirectory( GeoShapeExamples )
#add_subdirectory( GeoShiftUnion )
#add_subdirectory( GeoTessellatedSolidExamples )
#add_subdirectory( GeoFullPhysVol )

# GeoModel <--> Geant4 examples are built on explicit request only
if(GEOMODEL_BUILD_EXAMPLES_W_GEANT4)
  add_subdirectory( HelloGeoRead2G4 )
  # list( APPEND BUILT_PACKAGES "HelloGeoRead2G4")
endif()

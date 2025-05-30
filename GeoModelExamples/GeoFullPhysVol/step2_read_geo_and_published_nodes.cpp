// Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

/*
 * HelloGeo.cpp
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: Nov, 2018
 *
 */

// GeoModel includes
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoUtilFunctions.h"

// C++ includes
#include <iostream>
#include <map>
#include <fstream>
#include <cstdlib> // EXIT_FAILURE


// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits // so we will get, e.g., 'GeoModelKernelUnits::cm'



int main(int argc, char *argv[])
{
  // QCoreApplication app(argc, argv);

  // GET GEOMETRY FROM LOCAL DB
  // Set valid db path before first run
  const std::string path = "./geometry.db";
  std::cout << "Using this DB file:" << path << std::endl;

  // check if DB file exists. If not, return.
  // FIXME: TODO: this check should go in the 'GMDBManager' constructor.
  std::ifstream infile(path.c_str());
    if ( ! infile.good() ) {
      std::cout << "\n\tERROR!! The '" << path << "' input file does not exist! Please, check.\n";
      exit(EXIT_FAILURE);
  }
  infile.close();


  // open the DB
  auto db = std::make_unique<GMDBManager>(path);
  /* Open database */
  if (db->checkIsDBOpen()) {
    std::cout << "OK! Database is open!\n";
  }
  else {
    std::cout << "Database is not open!\n";
    // return;
    throw;
  }

  
  /* setup the GeoModel reader */
  GeoModelIO::ReadGeoModel readInGeo = GeoModelIO::ReadGeoModel(db);
  std::cout << "OK! ReadGeoModel is set." << std::endl;


  /* build the GeoModel tree */
  PVConstLink world = readInGeo.buildGeoModel(); // builds the whole GeoModel tree in memory
  std::cout << "ReadGeoModel::buildGeoModel() done." << std::endl;

  
  // --- testing the imported Geometry

  // get number of children volumes
  unsigned int nChil = world->getNChildVols();
  std:: cout << "world's number of children: " << nChil << std::endl;

  // loop over all children nodes
  std::cout << "Looping over all 'volume' children (i.e., GeoPhysVol and GeoFullPhysVol)..." << std::endl;
  for (unsigned int idx=0; idx<nChil; ++idx) {
	  PVConstLink nodeLink = world->getChildVol(idx);

	  if ( dynamic_cast<const GeoVPhysVol*>( &(*( nodeLink ))) ) {

		  std::cout << "\t" << "the child n. " << idx << " ";
		  const GeoVPhysVol *childVolV = &(*( nodeLink ));

		  if ( dynamic_cast<const GeoPhysVol*>(childVolV) ) {
			  const GeoPhysVol* childVol = dynamic_cast<const GeoPhysVol*>(childVolV);
			  std::cout << "is a GeoPhysVol, whose GeoLogVol's name is: " << childVol->getLogVol()->getName();
			  std::cout<< " and it has  "<<childVol->getNChildVols()<<" child volumes" << std::endl;
		  }
		  else if ( dynamic_cast<const GeoFullPhysVol*>(childVolV) ) {
			  const GeoFullPhysVol* childVol = dynamic_cast<const GeoFullPhysVol*>(childVolV);
			  std::cout << childVol << " is a GeoFullPhysVol, whose GeoLogVol's name is: " << childVol->getLogVol()->getName();
			  std::cout<< " and it has  "<<childVol->getNChildVols()<<" child volumes" << std::endl;
              GeoUtilFunctions::printTrf(childVol->getAbsoluteTransform());
		  }
	  }
  }

  std::cout << "We now read back from the DB the lists of published FullPhysVol and AlignableTransform nodes...\n";
  
  // Note: In the Step1 of this example, 
  //       we have published FullPhysVols and AlignableTransforms 
  //       into the DB as:
  //       - <integer-based key, GeoFullPhysVol*> 
  //       - <string-based key, GeoAlignableTransform*>
  //       Thus, we now get them back from the DB with the same key types we originally used:
  std::map<unsigned int, GeoFullPhysVol*> mapFPV = readInGeo.getPublishedNodes<unsigned int, GeoFullPhysVol*>("HelloToyExample");
  std::map<std::string, GeoAlignableTransform*> mapAXF = readInGeo.getPublishedNodes<std::string, GeoAlignableTransform*>("HelloToyExample");

  std::cout << "Published FullPhysVols from the DB...\n";
  unsigned int ii=0;
  for ( auto const& [key, vol] : mapFPV ) 
  {
      GeoTrf::Transform3D xf = vol->getAbsoluteTransform();

      if(0==ii) std::cout << " [key type (compiler's code): '" << typeid(key).name() << "']\n";
      std::cout << "--> key: " << key 
                << " - GeoFullPhysVol*: " << vol 
                    //<< " - AbsTransf: " << xf
                    //<< " , " << vol->getAbsoluteTransform() 
                << std::endl;
      GeoUtilFunctions::printTrf(vol->getAbsoluteTransform());
      ++ii;
  }
 
  /*
  ii=0;
  std::cout << "Published AlignableTransforms from the DB...\n";
  for ( auto const& [key, xf] : mapAXF ) 
  {
      if(0==ii) std::cout << " [key type (compiler's code): '" << typeid(key).name() << "']\n";
      std::cout << "--> key: " << key 
                << " - AlignableTransform*: " << xf 
                << std::endl;
      ++ii;
   }
*/
  std::cout << "Everything done." << std::endl;

  return 0;
}

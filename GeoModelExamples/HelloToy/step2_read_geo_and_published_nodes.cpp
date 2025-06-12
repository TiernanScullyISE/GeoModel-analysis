// Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

/*
 * HelloGeo.cpp
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: Nov, 2018
 *
 */

// GeoModel includes
#include "GeoModelHelpers/defineWorld.h"
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelHelpers/TransformToStringConverter.h"

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
      std::cout << "\n\tERROR!! The '" << path << "' file does not exists already!! Please, check.\n";
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
  GeoModelIO::ReadGeoModel readInGeo{db.get()};
  std::cout << "OK! ReadGeoModel is set." << std::endl;


  /* build the GeoModel tree */
  PVConstLink world = readInGeo.buildGeoModel(); // builds the whole GeoModel tree in memory
  std::cout << "ReadGeoModel::buildGeoModel() done." << std::endl;

  
  // --- testing the imported Geometry

  // get number of children volumes
  unsigned int nChil = world->getNChildVols();
  std:: cout << "world's number of children: " << nChil << std::endl;


  std::cout << "We now read back from the DB the lists of published FullPhysVol and AlignableTransform nodes...\n";
  
  // Note: In the Step1 of this example, 
  //       we have published FullPhysVols and AlignableTransforms 
  //       into the DB as:
  //       - <integer-based key, GeoFullPhysVol*> 
  //       - <string-based key, GeoAlignableTransform*>
  //       Thus, we now get them back from the DB with the same key types we originally used:
  std::map<unsigned int, GeoFullPhysVol*> mapFPV = readInGeo.getPublishedNodes<unsigned int, GeoFullPhysVol*>("HelloToyExample");
  std::map<std::string, GeoAlignableTransform*> mapAXF = readInGeo.getPublishedNodes<std::string, GeoAlignableTransform*>("HelloToyExample");

  //check for a table we know doesn't exist
  if (db->checkTableFromDB("PublishedFullPhysVols_HelloToyExample")) std::cout<<"We find the table that we expected - good!"<<std::endl;
  else std::cout<<"Uh oh, we don't find the expected table - bad!"<<std::endl;
  if(!db->checkTableFromDB("PublishedFullPhysVols_ByeByeToyExample")) std::cout<<"We don't find the table that we didn't expect - good!"<<std::endl;
  else std::cout<<"Uh oh, we found a table which doesn't exist - bad!"<<std::endl;  

  //Now test via the specific accessors with additional checks
  std::map<unsigned int, GeoFullPhysVol*> mapFPV_test = readInGeo.getPublishedNodes<unsigned int, GeoFullPhysVol*>("ByeByeToyExample",true);
  std::map<std::string, GeoAlignableTransform*> mapAXF_test = readInGeo.getPublishedNodes<std::string, GeoAlignableTransform*>("ByeByeToyExample",true);
  if(mapFPV_test.size()==0) std::cout<<"Returned an empty FPV map when using checks for a non-existing table - good!"<<std::endl;
  if(mapAXF_test.size()==0) std::cout<<"Returned an empty AXF map when using checks for a non-existing table - good!"<<std::endl;

  unsigned int ii=0;
  std::cout << "\n\nPublished AlignableTransforms from the DB...\n";
  std::cout << "['xf' is the output of 'getTransform()']\n";
  std::cout << "(Printing the first 3 only)" << std::endl;
  for ( auto const& [key, xf] : mapAXF ) 
  {
    if (0 == ii)
      std::cout << "[key type (compiler's code): '" << typeid(key).name() << "']\n";
    if (ii < 3)
    {

      std::cout << "\n\t--> key: " << key
                << " - AlignableTransform*: " << xf
                << std::endl;
      std::cout << "\txf:: "<<GeoTrf::toString(xf->getTransform());
    }
    ++ii;
  }

  ii=0; // reset the counter
  std::cout << "\n\nPublished FullPhysVols from the DB...\n";
  std::cout << "['xf' is the output of 'getAbsoluteTransform()']\n";
  std::cout << "(Printing the first 3 only)" << std::endl;
  for ( auto const& [key, vol] : mapFPV ) 
  {
      // GeoTrf::Transform3D xf = vol->getAbsoluteTransform(); // crashes

      if (0 == ii)
        std::cout << "[key type (compiler's code): '" << typeid(key).name() << "']\n";
      if (ii < 3)
      {
        std::cout << "\n\t--> key: " << key
                  << " - GeoFullPhysVol*: " << vol
                  << std::endl;
      }
      ++ii;
  }

  std::cout << "Everything done." << std::endl;

  return 0;
}

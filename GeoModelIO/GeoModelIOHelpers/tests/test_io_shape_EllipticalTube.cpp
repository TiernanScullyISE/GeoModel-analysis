// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/*
 * This example tests the I/O of GeoEllipticalTube.
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: Oct, 2024
 */

// GeoModel includes
#include "GeoModelIOHelpers/GMTests_IO.h"
#include "GeoModelKernel/GeoEllipticalTube.h"
#include "GeoModelHelpers/defineWorld.h"

// C++ includes
#include <iostream>

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS \
    GeoModelKernelUnits // so we will get, e.g., 'GeoModelKernelUnits::cm'

int main(int argc, char *argv[])
{
// Define the units
#define gr SYSTEM_OF_UNITS::gram
#define mole SYSTEM_OF_UNITS::mole

    // get the World volume,
    // we build it outside the class for convenience only
    PVLink world{createGeoWorld()};

    // Define elements used in this example:
    GeoElement *elAluminum = new GeoElement("Aluminum", "Al", 13, 26 * gr / mole);

    // Define materials used in this example:
    double densityOfAluminum = 2.7; // g/cm^3
    GeoMaterial *matAluminum = new GeoMaterial("Aluminum", densityOfAluminum);
    matAluminum->add(elAluminum, 1.0);
    matAluminum->lock();


      // Add a test GeoEllipticalTube shape
      const double halfX = 5.0 * SYSTEM_OF_UNITS::cm;
      const double halfY = 30.0 * SYSTEM_OF_UNITS::cm;
      const double halfZ = 30.0 * SYSTEM_OF_UNITS::cm;
  GeoEllipticalTube *sh = new GeoEllipticalTube(halfX, halfY, halfZ);                                                                          
  GeoLogVol *lV = new GeoLogVol("ellipticaltube", sh, matAluminum);                    
  GeoPhysVol *pV = new GeoPhysVol(lV);                                           
  GeoNameTag *nT = new GeoNameTag("Shape-EllipticalTube");                                    
  world->add(nT);                                                                           
  world->add(pV);

    // write to the test DB
    std::string testDB = "test_io_shape_EllipticalTube.db";
    unsigned loglevel = 2;
    GeoModelIO::IO::saveToDB(world, testDB, loglevel, true);

    // load from the test DB
    PVConstLink world2 = GeoModelIO::IO::loadDB(testDB);

    // get the child volume, then the shape from its logVol
    PVConstLink childVol = world2->getChildVol(0);
    const GeoEllipticalTube *shape = dynamic_cast<const GeoEllipticalTube *>(childVol->getLogVol()->getShape());


    // data to test
    std::vector<double> test_data_stored;
    std::vector<double> test_data_restored;
    std::vector<std::string> test_label;

    test_data_stored.push_back(halfX);
    test_data_restored.push_back(shape->getXHalfLength());
    test_label.push_back("shape->getXHalfLength()");

    bool test = true;
    unsigned int ii{0};
    for (const auto& num : test_data_stored) {
        if (test_data_stored != test_data_restored)
        {
            std::cout << "\nERROR!!! The restored '" << test_label[ii] << "' is different from the stored version!!!" << std::endl;
            std::cout << "Stored data: " << num << std::endl;
            std::cout << "Restored data: " << test_data_restored[ii] << std::endl;
            test = false;
        }
    }



    if (shape->getXHalfLength() != halfX)
    {
        std::cout << "\nERROR!!! The restored 'getXHalfLength' is different from the stored version!!!" << std::endl;
        std::cout << "Stored data: " << halfX << std::endl;
        std::cout << "Restored data: " << shape->getXHalfLength() << std::endl;
        test = false;
    }
    if (shape->getYHalfLength() != halfY)
    {
        std::cout << "\nERROR!!! The restored 'getYHalfLength' is different from the stored version!!!" << std::endl;
        std::cout << "Stored data: " << halfY << std::endl;
        std::cout << "Restored data: " << shape->getYHalfLength() << std::endl;
        test = false;
    }
    if (shape->getZHalfLength() != halfZ)
    {
        std::cout << "\nERROR!!! The restored 'getZHalfLength' is different from the stored version!!!" << std::endl;
        std::cout << "Stored data: " << halfZ << std::endl;
        std::cout << "Restored data: " << shape->getZHalfLength() << std::endl;
        test = false;
    }
    

    // Return 0 if all OK! :-)
    // Return 1 otherwise... :-(
    if (test)
        return 0;
    return 1;
    // ----------------
}

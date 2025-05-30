// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/*
 * This example tests the I/O of GeoUnidentifiedShape.
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: July, 2024
 */

// GeoModel includes
#include "GeoModelIOHelpers/GMTests_IO.h"
#include "GeoModelKernel/GeoUnidentifiedShape.h"
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
    GeoIntrusivePtr<GeoPhysVol> world{createGeoWorld()};

    // Define elements used in this example:
    GeoElement *elAluminum = new GeoElement("Aluminum", "Al", 13, 26 * gr / mole);

    // Define materials used in this example:
    double densityOfAluminum = 2.7; // g/cm^3
    GeoMaterial *matAluminum = new GeoMaterial("Aluminum", densityOfAluminum);
    matAluminum->add(elAluminum, 1.0);
    matAluminum->lock();

    // Add a test "UnidentifiedShape" shape node
    const std::string shapeDataAscii = "LAr::Example";
    const std::string shapeDataName = "LArCustomShape";
    GeoUnidentifiedShape *sUnidentifiedShape = new GeoUnidentifiedShape(shapeDataName, shapeDataAscii);
    const GeoLogVol *lUnidentifiedShape = new GeoLogVol("UnidentifiedShape", sUnidentifiedShape, matAluminum);
    GeoPhysVol *pUnidentifiedShape = new GeoPhysVol(lUnidentifiedShape);
    GeoNameTag *nUnidentifiedShape = new GeoNameTag("UnidentifiedShape");
    world->add(nUnidentifiedShape);
    world->add(pUnidentifiedShape);

    // write to the test DB
    std::string testDB = "test_io_unidentifiedshape.db";
    unsigned loglevel = 2;
    const bool forceDelete = true;
    GeoModelIO::IO::saveToDB(world, testDB, loglevel, forceDelete);

    // load from the test DB
    PVConstLink world2 = GeoModelIO::IO::loadDB(testDB);

    // get the child volume, then the shape from its logVol
    GeoIntrusivePtr<const GeoVPhysVol> childVol = world2->getChildVol(0);
    const GeoUnidentifiedShape *shape = dynamic_cast<const GeoUnidentifiedShape *>(childVol->getLogVol()->getShape());
    const std::string restoredShapeDataAscii = shape->asciiData();
    const std::string restoredShapeDataName = shape->name();

    bool test = true;
    if (restoredShapeDataAscii != shapeDataAscii)
    {
        std::cout << "\nERROR!!! The restored 'AsciiData' is different from the stored version!!!" << std::endl;
        std::cout << "Stored 'AsciiData': " << shapeDataAscii << std::endl;
        std::cout << "Restored 'AsciiData': " << restoredShapeDataAscii << std::endl;
        test = false;
    }
    if (restoredShapeDataName != shapeDataName)
    {
        std::cout << "\nERROR!!! The restored 'name' is different from the stored version!!!" << std::endl;
        std::cout << "\t==> Stored 'name': " << shapeDataName << std::endl;
        std::cout << "\t==> Restored 'name': " << restoredShapeDataName << std::endl;
        test = false;
    }

    // Return 0 if all OK! :-)
    // Return 1 otherwise... :-(
    if (test)
        return 0;
    return 1;
    // ----------------
}

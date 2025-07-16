// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/*
 * This example tests shared GeoSerialTransformer and Function nodes.
 *
 * It creates one single LogVol, with one Material and one Shape.
 * Then it creates one single PhysVol volumes using the above LogVol.
 * Then it creates two SerialTrasnformers with two different placing formulas,
 * but which share the same PhysVol defined above.
 *
 * It is meant to how how to share GeoModel nodes to reduce the memory footprint
 * and to test the GeoModel I/O packages and methods.
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: Apr, 2024
 *
 */

// GeoModel includes
#include "GeoGenericFunctions/AbsFunction.h"
#include "GeoGenericFunctions/Cos.h"
#include "GeoGenericFunctions/Sin.h"
#include "GeoGenericFunctions/Variable.h"
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelIOHelpers/GMIO.h"
#include "GeoModelIOHelpers/GMTests_IO.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoSerialDenominator.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelHelpers/defineWorld.h"

// C++ includes
#include <cstdlib>  // EXIT_FAILURE
#include <fstream>
#include <iostream>

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS \
    GeoModelKernelUnits  // so we will get, e.g., 'GeoModelKernelUnits::cm'

using namespace GeoGenfun;
using namespace GeoXF;

int main(int argc, char *argv[]) {
// Define the units
#define gr SYSTEM_OF_UNITS::gram
#define mole SYSTEM_OF_UNITS::mole
#define cm3 SYSTEM_OF_UNITS::cm3
#define degree SYSTEM_OF_UNITS::degree

    // get the World volume,
    // we build it outside the class for convenience only
    GeoIntrusivePtr<GeoPhysVol> world{createGeoWorld()};

    // Define elements used in this example:
    GeoElement *aluminum = new GeoElement("Aluminum", "Al", 13, 26 * gr / mole);
    GeoElement *iron = new GeoElement("Iron", "Fe", 26.0, 55.847 * gr / mole);

    // Define materials:
    double densityOfAluminum = 2.7;  // g/cm^3
    double densityOfIron = 7.87;     // g/cm^3
    GeoMaterial *Aluminum = new GeoMaterial("Aluminum", densityOfAluminum);
    Aluminum->add(aluminum, 1.0);
    Aluminum->lock();
    GeoMaterial *Iron = new GeoMaterial("Iron", densityOfIron);
    Iron->add(iron, 1.0);
    Iron->lock();

    // define a container box
    const GeoBox *toyBox =
        new GeoBox(1200 * SYSTEM_OF_UNITS::cm, 1200 * SYSTEM_OF_UNITS::cm,
                   1200 * SYSTEM_OF_UNITS::cm);
    const GeoLogVol *toyLog = new GeoLogVol("ToyLog", toyBox, Aluminum);  //
    GeoPhysVol *toyPhys = new GeoPhysVol(toyLog);

    //--------------------------------------//
    // Define some daughters and put them in the toy volume
    //--------------------------------------//
    const GeoTube *ringTube =
        new GeoTube(500 * SYSTEM_OF_UNITS::cm, 1000 * SYSTEM_OF_UNITS::cm,
                    5.0 * SYSTEM_OF_UNITS::cm);
    const GeoLogVol *ringLog = new GeoLogVol("RingLog", ringTube, Aluminum);
    // define a global name for the daughters
    GeoSerialDenominator *ringName = new GeoSerialDenominator("RING");
    toyPhys->add(ringName);
    // Make 100 of these              //
    // within the volume of the toy:  //
    for (int i = 0; i < 100; i++) {
        GeoFullPhysVol *ringPhys = new GeoFullPhysVol(ringLog);
        GeoAlignableTransform *xform = new GeoAlignableTransform(
            GeoTrf::TranslateZ3D((i - 50) * 20 * SYSTEM_OF_UNITS::cm));
        toyPhys->add(xform);
        toyPhys->add(ringPhys);
    }

    //--------------------------------------//
    //  Now, in addition to active daughters,
    // add some passive material.
    // This is done here using
    // the "SerialTransformer",
    // our way of parameterizing volumes.
    // It does not need to be done this way,
    // but we want to provide an example of
    // parametrizations in the Toy
    //--------------------------------------//

    GeoBox *sPass =
        new GeoBox(5.0 * SYSTEM_OF_UNITS::cm, 30 * SYSTEM_OF_UNITS::cm,
                   30 * SYSTEM_OF_UNITS::cm);
    GeoLogVol *lPass = new GeoLogVol("Passive", sPass, Iron);
    GeoPhysVol *pPass = new GeoPhysVol(lPass);

    //--------------------------------------//
    // Now, define two mathematical functions,
    // which are used to parametrically place the volumes
    // in space.
    //--------------------------------------//
    const unsigned int NPLATES = 100;
    Variable i;
    Sin sin;
    GENFUNCTION f = 360 * SYSTEM_OF_UNITS::deg / NPLATES * i;
    GENFUNCTION g = sin(4 * f);
    GENFUNCTION h = -g;
    TRANSFUNCTION t1 = Pow(GeoTrf::RotateZ3D(1.0), f) *
                       GeoTrf::TranslateX3D(1100 * SYSTEM_OF_UNITS::cm) *
                       Pow(GeoTrf::TranslateZ3D(800 * SYSTEM_OF_UNITS::cm), g);
    TRANSFUNCTION t2 = Pow(GeoTrf::RotateZ3D(1.0), f) *
                       GeoTrf::TranslateX3D(1100 * SYSTEM_OF_UNITS::cm) *
                       Pow(GeoTrf::TranslateZ3D(800 * SYSTEM_OF_UNITS::cm), h);

    //--------------------------------------//
    // Inside, by the way, the serial transformer
    // will evaluate the functions:
    // HepTransform3D xf = t1(i), for i=1,NPLATES....
    //--------------------------------------//

    GeoSerialDenominator *pass1Name = new GeoSerialDenominator("PASSIVE-1-");
    GeoSerialTransformer *s1 = new GeoSerialTransformer(pPass, &t1, NPLATES);
    toyPhys->add(pass1Name);
    toyPhys->add(s1);

    GeoSerialDenominator *pass2Name = new GeoSerialDenominator("PASSIVE-2-");
    GeoSerialTransformer *s2 = new GeoSerialTransformer(pPass, &t2, NPLATES);
    toyPhys->add(pass2Name);
    toyPhys->add(s2);

    world->add(toyPhys);

    // --- I/O TESTS ---
    std::cout << "\nTests:\n";
    unsigned loglevel = 2;
    unsigned printtests = 1;
    std::pair<bool, std::map<std::string, bool>> tests = GeoModelIO::TestIO::runAllTests(world, loglevel, printtests);
    bool testall = tests.first;
    
    // Return 0 if all OK! :-) 
    // Return 1 otherwise... :-(
    if (testall) return 0;
    return 1;
    // ----------------
}

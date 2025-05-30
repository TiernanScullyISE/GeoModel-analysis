// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/*
 * This example tests shared NameTag and SerialDenominator nodes.
 *
 * It creates one single LogVol, with one Material and one Shape.
 * Then it creates three PhysVol volumes using the above LogVol.
 * Then, one of those PhysVols is used to create children of the above volumes.
 *
 * This small application is meant to test the GeoModel I/O packages and methods
 * on different nodes.
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: Feb, 2023
 *
 */

// local includes
#include "world.h"

// GeoModel includes
#include "GeoModelIOHelpers/GMIO.h"
#include "GeoModelIOHelpers/GMTests_IO.h"

// GeoModelKernel includes
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoSerialDenominator.h"

// C++ includes
#include <cstdlib>  // EXIT_FAILURE
#include <fstream>
#include <iostream>

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS \
    GeoModelKernelUnits  // so we will get, e.g., 'GeoModelKernelUnits::cm'

std::vector<std::string> testNTSD(const GeoVPhysVol *vol, std::string volname) {
    std::vector<std::string> names;
    std::cout << "Child volumes of '" << volname << "':\n";
    for (size_t id = 0; id < vol->getNChildVols(); ++id) {
        std::string name = vol->getNameOfChildVol(id);
        std::cout << "child [" << id << "] name: " << name << std::endl;
        names.push_back(name);
    }
    return names;
}

int main(int argc, char *argv[]) {
// Define the units
#define gr SYSTEM_OF_UNITS::gram
#define mole SYSTEM_OF_UNITS::mole
#define cm3 SYSTEM_OF_UNITS::cm3

    // get the World volume,
    // we build it outside the class for convenience only
    GeoPhysVol *world = GeoModel::World().getPhysVol();

    // Define elements used in this example:
    GeoElement *aluminium =
        new GeoElement("Aluminium", "Al", 13, 26 * gr / mole);

    // Define materials:
    double densityOfAluminium = 2.7;  // g/cm^3
    GeoMaterial *Aluminium = new GeoMaterial("Aluminium", densityOfAluminium);
    Aluminium->add(aluminium, 1.0);
    Aluminium->lock();

    // define two shape nodes
    const GeoBox *sh1 = new GeoBox(5.0, 5.0, 5.0);
    const GeoBox *sh2 = new GeoBox(1.0, 1.0, 1.0);

    // define two LogVol nodes
    const GeoLogVol *lv1 = new GeoLogVol("LVParent", sh1, Aluminium);
    const GeoLogVol *lv2 = new GeoLogVol("LVChild", sh2, Aluminium);

    // build two PhysVol volumes with the two LogVol nodes
    GeoPhysVol *pv1 = new GeoPhysVol(lv1);  // parent volume
    GeoPhysVol *pv2 = new GeoPhysVol(lv1);
    GeoPhysVol *pv3 = new GeoPhysVol(lv1);
    GeoPhysVol *pv4 = new GeoPhysVol(lv2);  // child volume

    // create the transforms to place the parent volumes
    GeoTransform *tr1 = new GeoTransform(GeoTrf::Translate3D(-15.0, 0, 0));
    GeoTransform *tr2 = new GeoTransform(GeoTrf::Translate3D(15.0, 0, 0));
    // create the transforms to place the child volumes inside the parent volume
    GeoTransform *tr3 = new GeoTransform(GeoTrf::Translate3D(-3.0, 0, 0));
    GeoTransform *tr4 = new GeoTransform(GeoTrf::Translate3D(3.0, 0, 0));

    // create a NameTag to share among the parent volumes
    GeoNameTag *nt1 = new GeoNameTag("ParentVol");
    // create a SerialDenominator to incrementally name all child volumes of PV1
    // and PV3
    GeoSerialDenominator *sd1 = new GeoSerialDenominator("Child-");

    // add the NameTag for the 1st parent volume to the world
    world->add(nt1);
    // add the first vol to the world, at 0,0,0
    world->add(pv1);

    // add the same vol to the world, but this time at -15,0,0
    world->add(tr1);
    world->add(nt1);  // NameTag for the 2nd parent vol
    world->add(pv2);

    // add the same vol to the world, but this time at 15,0,0
    world->add(tr2);
    world->add(nt1);  // NameTag for the 3rd parent vol
    world->add(pv3);

    // add the SerialDenominator to the parent volume,
    // to name all children incrementally
    pv1->add(sd1);

    // add the child volume to the PV1 volume, at 0,0,0
    pv1->add(pv4);

    // add the same child volume to the PV1 volume, at -3,0,0
    pv1->add(tr3);
    pv1->add(pv4);

    // add the same child volume to the PV1 volume, at 3,0,0
    pv1->add(tr4);
    pv1->add(pv4);

    // --- I/O TESTS ---
    std::cout << "\nGeneral I/O Tests:\n";
    bool testall = GeoModelIO::TestIO::runAllTests(world, 0, 1, true).first; // we replace the existing .db file
    // ----------------

    // --- Specific NameTags and SerialDenominator tests ---
    std::cout << "\nSpecific NameTags and SerialDenominator tests:\n";
    const std::string db1_name = "geometry_db_test_1.db";
    GeoModelIO::IO::saveToDB(world, db1_name, 0, true); // we write to a .db file replacing the existing file
    PVConstLink world2 = GeoModelIO::IO::loadDB(db1_name, 0); // we reload it 
    std::vector<std::string> t1 = testNTSD(world, "world");
    std::vector<std::string> t2 = testNTSD(pv1, "pv1");
    std::vector<std::string> t3 = testNTSD(world2, "world2");
    std::vector<std::string> t4 =
        testNTSD(dynamic_cast<const GeoPhysVol *>(&(*(world2->getChildVol(0)))),
                 "pv1_2");
    bool tt1 = (t1 == t3);
    bool tt2 = (t2 == t4);
    testall = (testall && tt1 && tt2);
    if (testall)
        std::cout << "OK! Test of names passed.\n";
    else
        std::cout << "ERROR! Test of names FAILED!!\n";
    // ----------------

    // return 0 if all tests passed, returns 1 otherwise.
    if (testall) return 0;
    return 1;
}

// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/*
 *  HelloToyXML.cpp
 *
 *  Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *  Created on: Dec, 2020
 *  Updates:
 *  - Jan, 2023 -- Added read of custom tables.
 *
 */

// GeoModel includes
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoUtilFunctions.h"
#include "GeoModelRead/ReadGeoModel.h"

// C++ includes
#include <cstdlib>  // EXIT_FAILURE
#include <fstream>
#include <iostream>
#include <map>

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS \
    GeoModelKernelUnits  // so we will get, e.g., 'GeoModelKernelUnits::cm'

GeoPhysVol* createTheWorld(GeoPhysVol* world) {
    if (world == nullptr) {
        //-----------------------------------------------------------------------------------//
// Define the materials that we shall use. //
// ----------------------------------------------------------------------------------//

// Define the units
#define gr SYSTEM_OF_UNITS::gram
#define mole SYSTEM_OF_UNITS::mole
#define cm3 SYSTEM_OF_UNITS::cm3

        // Define the chemical elements
        GeoElement* Nitrogen =
            new GeoElement("Nitrogen", "N", 7.0, 14.0067 * gr / mole);
        GeoElement* Oxygen =
            new GeoElement("Oxygen", "O", 8.0, 15.9995 * gr / mole);
        GeoElement* Argon =
            new GeoElement("Argon", "Ar", 18.0, 39.948 * gr / mole);
        GeoElement* Hydrogen =
            new GeoElement("Hydrogen", "H", 1.0, 1.00797 * gr / mole);

        // Define the materials
        double densityOfAir = 0.001214 * gr / cm3;
        GeoMaterial* air = new GeoMaterial("Air", densityOfAir);
        air->add(Nitrogen, 0.7494);
        air->add(Oxygen, 0.2369);
        air->add(Argon, 0.0129);
        air->add(Hydrogen, 0.0008);
        air->lock();

        const GeoBox* worldBox =
            new GeoBox(1000 * SYSTEM_OF_UNITS::cm, 1000 * SYSTEM_OF_UNITS::cm,
                       1000 * SYSTEM_OF_UNITS::cm);
        const GeoLogVol* worldLog = new GeoLogVol("WorldLog", worldBox, air);
        world = new GeoPhysVol(worldLog);
    }
    return world;
}

int main(int argc, char* argv[]) {
    // QCoreApplication app(argc, argv);

    // GET GEOMETRY FROM LOCAL DB
    // Set valid db path before first run
    const std::string path = "./geometry.db";
    std::cout << "Using this DB file:" << path << std::endl;

    // check if DB file exists. If not, return.
    // FIXME: TODO: this check should go in the 'GMDBManager' constructor.
    std::ifstream infile(path.c_str());
    if (!infile.good()) {
        std::cout << "\n\tERROR!! The '" << path
                  << "' file does not exists already!! Please, check.\n";
        exit(EXIT_FAILURE);
    }
    infile.close();

    // open the DB
    auto db = std::make_unique<GMDBManager>(path);
    /* Open database */
    if (db->checkIsDBOpen()) {
        std::cout << "OK! Database is open!\n";
    } else {
        std::cout << "Database is not open!\n";
        // return;
        throw;
    }

    /* setup the GeoModel reader */
    GeoModelIO::ReadGeoModel readInGeo{db.get()};
    std::cout << "OK! ReadGeoModel is set." << std::endl;

    /* build the GeoModel tree */
    PVConstLink world = readInGeo.buildGeoModel();  // builds the whole GeoModel tree in memory
    std::cout << "ReadGeoModel::buildGeoModel() done." << std::endl;

    // --- testing the imported Geometry

    // get number of children volumes
    unsigned int nChil = world->getNChildVols();
    std::cout << "world's number of children: " << nChil << std::endl;

    // loop over all children nodes
    std::cout << "Looping over all 'volume' children (i.e., GeoPhysVol and "
                 "GeoFullPhysVol)..."
              << std::endl;
    for (unsigned int idx = 0; idx < nChil; ++idx) {
        PVConstLink nodeLink = world->getChildVol(idx);

        if (dynamic_cast<const GeoVPhysVol*>(&(*(nodeLink)))) {
            std::cout << "\t"
                      << "the child n. " << idx << " ";
            const GeoVPhysVol* childVolV = &(*(nodeLink));

            if (dynamic_cast<const GeoPhysVol*>(childVolV)) {
                const GeoPhysVol* childVol =
                    dynamic_cast<const GeoPhysVol*>(childVolV);
                std::cout << "is a GeoPhysVol, whose GeoLogVol's name is: "
                          << childVol->getLogVol()->getName();
                std::cout << " and it has  " << childVol->getNChildVols()
                          << " child volumes" << std::endl;
            } else if (dynamic_cast<const GeoFullPhysVol*>(childVolV)) {
                const GeoFullPhysVol* childVol =
                    dynamic_cast<const GeoFullPhysVol*>(childVolV);
                std::cout << "is a GeoFullPhysVol, whose GeoLogVol's name is: "
                          << childVol->getLogVol()->getName();
                std::cout << " and it has  " << childVol->getNChildVols()
                          << " child volumes" << std::endl;
                std::cout << "\txf:";
                GeoUtilFunctions::printTrf(childVol->getAbsoluteTransform());
            }
        }
    }

    std::cout << "We now read back from the DB the lists of published "
                 "FullPhysVol and AlignableTransform nodes...\n";

    // Note: In the Step1 of this example,
    //       we have published FullPhysVols and AlignableTransforms
    //       into the DB as:
    //       - <integer-based key, GeoFullPhysVol*>
    //       - <string-based key, GeoAlignableTransform*>
    //       Thus, we now get them back from the DB with the same key types we
    //       originally used:
    std::map<unsigned int, GeoFullPhysVol*> mapFPV =
        readInGeo.getPublishedNodes<unsigned int, GeoFullPhysVol*>(
            "HelloToyXML");
    std::map<std::string, GeoAlignableTransform*> mapAXF =
        readInGeo.getPublishedNodes<std::string, GeoAlignableTransform*>(
            "HelloToyXML");

    unsigned int ii = 0;
    std::cout << "\n\nPublished AlignableTransforms from the DB...\n";
    std::cout << "['xf' is the output of 'getTransform()']\n";
    for (auto const& [key, xf] : mapAXF) {
        if (0 == ii)
            std::cout << "[key type (compiler's code): '" << typeid(key).name()
                      << "']\n";
        std::cout << "\n\t--> key: " << key << " - AlignableTransform*: " << xf
                  << std::endl;
        std::cout << "\txf:: ";
        GeoUtilFunctions::printTrf(xf->getTransform());
        ++ii;
    }

    ii = 0;  // reset the counter
    std::cout << "\n\nPublished FullPhysVols from the DB...\n";
    std::cout << "['xf' is the output of 'getAbsoluteTransform()']\n";
    for (auto const& [key, vol] : mapFPV) {
        GeoTrf::Transform3D xf = vol->getAbsoluteTransform();

        if (0 == ii)
            std::cout << "[key type (compiler's code): '" << typeid(key).name()
                      << "']\n";
        std::cout << "\n\t--> key: " << key << " - GeoFullPhysVol*: " << vol
                  << std::endl;
        std::cout << "\txf:";
        GeoUtilFunctions::printTrf(vol->getAbsoluteTransform());
        ++ii;
    }

    std::cout << "\n\nWe now list all available tables.\n\n";
    readInGeo.printAllDBTables();
    readInGeo.printDBTable("dbversion");

    std::cout
        << "\n\nWe now print the content of the tables containing auxiliary "
           "data, which have been stored in the data file as additional custom "
           "tables\n\n";
    readInGeo.printDBTable("HELLOBOXES");
    readInGeo.printDBTable("HELLOCABLES");
    readInGeo.printDBTable("HelloTest");

    std::cout
        << "\n\nWe now read back the tables containing auxiliary data, which "
           "have been stored in the data file as additional custom tables\n\n";
    std::vector<std::vector<std::string>> hellotable =
        readInGeo.getTableFromTableName("HelloTest");
    for (auto& row : hellotable) {
        for (auto& rec : row) {
            std::cout << " record: " << rec;
        }
        std::cout << std::endl;
    }
    std::cout << "Everything done." << std::endl;

    return 0;
}

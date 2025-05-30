// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/*
 * HelloGeoRead_1
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: Nov, 2018
 *  Updated on: Oct, 2023
 *
 */

// GeoModel includes
#include "GeoModelIOHelpers/GMIO.h"

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelKernel/throwExcept.h"

// C++ includes
#include <cstdlib>  // EXIT_FAILURE
#include <fstream>
#include <iostream>


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "\nERROR!\nUsage: %s <geometry.db>\n\n", argv[0]);
        return 1;
    }

    // Get from command-line the input SQLite '.db' file containing the geometry
    std::string fileName;
    fileName = argv[1];
    std::cout << "Using this SQLite '.db' file:" << fileName << std::endl;


    // Get the 'world' volume from the GeoModel DB
    std::cout << "Picking the 'World' volume from the geometry DB file..."
              << std::endl;
    PVConstLink world = GeoModelIO::IO::loadDB(fileName);
    if(!world) {
       THROW_EXCEPTION("---ERROR! 'World' is a 'nullptr'! exiting...\n\n");
    } else {
        std::cout << "'World' volume loaded." << std::endl;
    }

    // --- Reading the properties of the 'world' volume retrieved from the .db file
    std::cout << "Getting the GeoLogVol used by the 'world' volume"
              << std::endl;
    const GeoLogVol* logVol = world->getLogVol();
    std::cout << "'world' GeoLogVol name: " << logVol->getName() << std::endl;
    std::cout << "'world' GeoMaterial name: "
              << logVol->getMaterial()->getName() << std::endl;

    // --- Reading the imported Geometry

    // get number of children volumes
    unsigned int nChil = world->getNChildVols();
    std::cout << "'world' number of children: " << nChil << std::endl;

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
                std::cout << "is a GeoPhysVol, whose GeoLogVol name is: "
                          << childVol->getLogVol()->getName();
                std::cout << " and it has  " << childVol->getNChildVols()
                          << " child volumes" << std::endl;
            } else if (dynamic_cast<const GeoFullPhysVol*>(childVolV)) {
                const GeoFullPhysVol* childVol =
                    dynamic_cast<const GeoFullPhysVol*>(childVolV);
                std::cout << "is a GeoFullPhysVol, whose GeoLogVol name is: "
                          << childVol->getLogVol()->getName();
                std::cout << " and it has  " << childVol->getNChildVols()
                          << " child volumes" << std::endl;
            }
        } else if (dynamic_cast<const GeoNameTag*>(&(*(nodeLink)))) {
            std::cout << "\t"
                      << "the child n. " << idx << " is a GeoNameTag"
                      << std::endl;
            const GeoNameTag* childVol =
                dynamic_cast<const GeoNameTag*>(&(*(nodeLink)));
            std::cout << "\t\t GeoNameTag's name: " << childVol->getName()
                      << std::endl;
        } else if (dynamic_cast<const GeoMaterial*>(&(*(nodeLink)))) {
            std::cout << "\t"
                      << "the child n. " << idx << " is a GeoMaterial"
                      << std::endl;
            const GeoMaterial* childVol =
                dynamic_cast<const GeoMaterial*>(&(*(nodeLink)));
            std::cout << "\t\t GeoMaterial's name: " << childVol->getName()
                      << std::endl;
            std::cout << "\t\t GeoMaterial's number of elements: "
                      << childVol->getNumElements() << std::endl;
        }
    }

    std::cout << "Everything done." << std::endl;

    // return app.exec();
    return 0;
}

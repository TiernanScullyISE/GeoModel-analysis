
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/*
 * This header file provides helper functions to write and read a GeoModel SQLite file. 
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: Feb, 2023
 *
 */


#ifndef GMIO_H
#define GMIO_H

// C++ includes
#include <cstdlib>  // EXIT_FAILURE
#include <fstream>
#include <string>

#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelWrite/WriteGeoModel.h"

#include "GeoModelKernel/throwExcept.h"

namespace GeoModelIO {

class IO {
   public:

    // dummy constructor
    IO(){};

    static GMDBManager saveToDB(const GeoVPhysVol* world, const std::string path,
                                unsigned loglevel = 0, const bool forceDelete = false ) {
        // Check if the output DB file exists already. 
        // - If yes and the 'forceDelete' option is set to 'true' by the user, 
        // then delete it before trying to create the new one; that is, 
        // the 'forceDelete' option replaces the existing '.db' file.
        // - If yes and the 'forceDelete' option is set to 'false', then throw
        // and error and exits. 
        std::ifstream inputfile(path.c_str());
        if (inputfile.good()) {
            if (true == forceDelete) {
                std::cout << "GeoModelIO -- INFO: you set the 'forceDelete' option to 'true', so we replace the existing .db file: '" << path << "'." << std::endl;
                std::remove(path.c_str());  // delete file
            } else {
            std::cerr << "\n*** ERROR! The output '" << path << "' file exists already! If you want to replace it, set the 'forceDelete' option to 'true'. Otherwise, rename the existing file, or move it to another place, and retry. ***\n\n{"
                  << __func__ << " ["
                  << __PRETTY_FUNCTION__ 
                  << "]}\n\n";
                throw;
            }
        }
        inputfile.close();

        // open the DB connection
        GMDBManager db(path);

        // check the DB connection
        if (!db.checkIsDBOpen()) {
            std::cout << "Database ERROR!! Exiting..." << std::endl;
            THROW_EXCEPTION("It was not possible to open the DB correctly!");
        }

        // Dump the tree volumes to a local file
        std::cout << "Dumping the GeoModel geometry to the DB file..."
                  << std::endl;
        // init the GeoModel node action
        GeoModelIO::WriteGeoModel dumpGeoModelGraph(db);
        // set loglevel of write action, if > 0
        if (loglevel > 0) {
            dumpGeoModelGraph.setLogLevel(loglevel);
        }
        world->exec(&dumpGeoModelGraph);  // visit all GeoModel nodes
        dumpGeoModelGraph.saveToDB();     // save to the SQlite DB file
        std::cout << "DONE. Geometry saved." << std::endl;
        return db;
    }

    static PVConstlink loadDB(const std::string path, unsigned loglevel = 0) {
        // check if DB file exists. 
        // If not, print a warning message and return a nullptr.
        std::ifstream inputfile(path.c_str());
        bool failed = false;
        if (!inputfile.good()) {
            std::cerr << "\n*** WARNING! The input .db file does not exist! Check the path of the input file. Returning a nullptr... ***\n{"
                  << __func__ << " ["
                  << __PRETTY_FUNCTION__ 
                  << "]}\n\n";
            failed = true;
        }
        inputfile.close();
        if(failed) {
            return nullptr;
        }

        // open the DB
       auto db = std::make_unique<GMDBManager>(path);
        if (!db->checkIsDBOpen()) {
            std::cout << "ERROR!! -- Database is not open!\n";
            THROW_EXCEPTION("It was not possible to open the DB correctly!");
        }

        /* setup the GeoModel reader */
        GeoModelIO::ReadGeoModel geoReader{db.get()};
        // set loglevel of read action, if > 0
        if (loglevel > 0) {
            geoReader.setLogLevel(loglevel);
        }

        /* build the GeoModel geometry */
        // builds the whole GeoModel tree in memory
        PVConstLink rootVolume = geoReader.buildGeoModel();

        return rootVolume;  // FIXME: See if you can pass a smart ptr
    }

    static GeoModelIO::ReadGeoModel getReaderDB(const std::string path, unsigned loglevel = 0) {
        // check if DB file exists. 
        // If not, print a warning message and exit.
        std::ifstream inputfile(path.c_str());
        if (!inputfile.good()) {
            std::cerr << "\n*** ERROR! The input .db file does not exist! Check the path of the input file. Exiting...\n"
                  << "{ " << __func__ << " "
                  << "[" << __PRETTY_FUNCTION__ 
                  << "]}.\n\n";
            exit(EXIT_FAILURE);
        }
        inputfile.close();

        // open the DB
        auto db = std::make_unique<GMDBManager>(path);
        if (!db->checkIsDBOpen()) {
            std::cout << "ERROR!! -- Database is not open!\n";
            THROW_EXCEPTION("It was not possible to open the DB correctly!");
        }

        /* setup the GeoModel reader */
        GeoModelIO::ReadGeoModel geoReader{db.release()};
        // set loglevel of read action, if > 0
        if (loglevel > 0) {
            geoReader.setLogLevel(loglevel);
        }

        return geoReader;  // FIXME: See if you can pass a smart ptr
    }

    static std::map<std::string, unsigned long> countNodesFromDB(
        GMDBManager& db) {
        // map to populate and return
        std::map<std::string, unsigned long> mmap;
        // get the number of all nodes in the DB from the DB manager
        unsigned long nphysvols = db.getTableFromNodeType_String("GeoPhysVol").size();
        unsigned long nfullphysvols = db.getTableFromNodeType_String("GeoFullPhysVol").size();
        unsigned long nlogvols = db.getTableFromNodeType_VecVecData("GeoLogVol").size();
        unsigned long nelements = db.getTableFromNodeType_VecVecData("GeoElement").size();
        unsigned long nmaterials = db.getTableFromNodeType_VecVecData("GeoMaterial").size();
        unsigned long nalignables = db.getTableFromNodeType_String("GeoAlignableTransform").size();
        unsigned long nfunctions = db.getTableFromNodeType_VecVecData("Function").size();
        unsigned long nserialtransformers = db.getTableFromNodeType_String("GeoSerialTransformer").size();
        unsigned long nserialdenominators = db.getTableFromNodeType_String("GeoSerialDenominator").size();
        unsigned long ntransforms = db.getTableFromNodeType_VecVecData("GeoTransform").size();
        unsigned long nserialidentifiers = db.getTableFromNodeType_VecVecData("GeoSerialIdentifier").size();
        unsigned long nidentifiertags = db.getTableFromNodeType_VecVecData("GeoIdentifierTag").size();
        unsigned long nnametags = db.getTableFromNodeType_VecVecData("GeoNameTag").size();
        // get shapes
        unsigned long nshapes = db.getTableFromNodeType_String("GoShape").size(); // TODO: to be removed later
        unsigned long nshapes_box = db.getTableFromNodeType_VecVecData("GeoBox").size();
        unsigned long nshapes_cons = db.getTableFromNodeType_VecVecData("GeoCons").size();
        unsigned long nshapes_para = db.getTableFromNodeType_VecVecData("GeoPara").size();
        unsigned long nshapes_pcon = db.getTableFromNodeType_VecVecData("GeoPcon").size();
        unsigned long nshapes_pgon = db.getTableFromNodeType_VecVecData("GeoPgon").size();
        unsigned long nshapes_simplepol = db.getTableFromNodeType_VecVecData("GeoSimplePolygonBrep").size();
        unsigned long nshapes_trap = db.getTableFromNodeType_VecVecData("GeoTrap").size();
        unsigned long nshapes_trd = db.getTableFromNodeType_VecVecData("GeoTrd").size();
        unsigned long nshapes_tube = db.getTableFromNodeType_VecVecData("GeoTube").size();
        unsigned long nshapes_tubs = db.getTableFromNodeType_VecVecData("GeoTubs").size();
        unsigned long nshapes_twisted = db.getTableFromNodeType_VecVecData("GeoTwistedTrap").size();
        unsigned long nshapes_unid = db.getTableFromNodeType_VecVecData("GeoUnidentifiedShape").size();
        unsigned long nshapes_shift = db.getTableFromNodeType_VecVecData("GeoShapeShift").size();
        unsigned long nshapes_union = db.getTableFromNodeType_VecVecData("GeoShapeUnion").size();
        unsigned long nshapes_intersection = db.getTableFromNodeType_VecVecData("GeoShapeIntersection").size();
        unsigned long nshapes_subtraction = db.getTableFromNodeType_VecVecData("GeoShapeSubtraction").size();
        // get metadata
        unsigned long nchildrenconnections = db.getChildrenTable().size();

        mmap["PhysVol"] = nphysvols;
        mmap["FullPhysVol"] = nfullphysvols;
        mmap["LogVol"] = nlogvols;
        mmap["Element"] = nelements;
        mmap["Material"] = nmaterials;
        mmap["Alignable"] = nalignables;
        mmap["Function"] = nfunctions;
        mmap["SerialTransformer"] = nserialtransformers;
        mmap["SerialDenominator"] = nserialdenominators;
        mmap["ChildrenConnections"] = nchildrenconnections;
        mmap["Transform"] = ntransforms;
        mmap["SerialIdentifier"] = nserialidentifiers;
        mmap["IdentifierTag"] = nidentifiertags;
        mmap["NameTag"] = nnametags;
        mmap["Shape"] = nshapes;
        mmap["Shape_Box"] = nshapes_box;
        mmap["Shape_Cons"] = nshapes_cons;
        mmap["Shape_Para"] = nshapes_para;
        mmap["Shape_Pcon"] = nshapes_pcon;
        mmap["Shape_Pgon"] = nshapes_pgon;
        mmap["Shape_SimplePolygonBrep"] = nshapes_simplepol;
        mmap["Shape_Trap"] = nshapes_trap;
        mmap["Shape_Trd"] = nshapes_trd;
        mmap["Shape_Tube"] = nshapes_tube;
        mmap["Shape_Tubs"] = nshapes_tubs;
        mmap["Shape_TwistedTrap"] = nshapes_twisted;
        mmap["Shape_UnidentifiedShape"] = nshapes_unid;
        mmap["Shape_Shift"] = nshapes_shift;
        mmap["Shape_Union"] = nshapes_union;
        mmap["Shape_Intersection"] = nshapes_intersection;
        mmap["Shape_Subtraction"] = nshapes_subtraction;

        return mmap;
    }

    static std::map<std::string, unsigned long> countTreeMemoryNodesFromVolume(
        const GeoVPhysVol* world, unsigned loglevel = 0) {
        std::map<std::string, unsigned long> mmap;

        // init the graph action to count all nodes in the in-memory tree
        GeoModelIO::WriteGeoModel dump;
        // set loglevel of write action, if > 0
        if (loglevel > 0) {
            dump.setLogLevel(loglevel);
        }
        world->exec(&dump);  // visit all GeoModel nodes of the 'world' volume

        unsigned long nphysvols = dump.getNPhysVols();
        unsigned long nfullphysvols = dump.getNFullPhysVols();
        unsigned long nlogvols = dump.getNLogVols();
        unsigned long nelements = dump.getNElements();
        unsigned long nmaterials = dump.getNMaterials();
        unsigned long nalignables = dump.getNAlignableTransforms();
        unsigned long nfunctions = dump.getNFunctions();
        unsigned long nserialtransformers = dump.getNSerialTransformers();
        unsigned long nserialdenominators = dump.getNSerialDenominators();
        unsigned long nchildrenconnections = dump.getNChildrenConnections();
        // unsigned nrootvolume = dump.getNRootVolume();
        unsigned long ntransforms = dump.getNTransforms();
        unsigned long nserialidentifiers = dump.getNSerialIdentifiers();
        unsigned long nidentifiertags = dump.getNIdentifierTags();
        unsigned long nnametags = dump.getNNameTags();
         // get shapes
        unsigned long nshapes = dump.getNShapes(); // TODO: to be removed later
        unsigned long nshapes_box = dump.getNShapes_Box();
        unsigned long nshapes_cons = dump.getNShapes_Cons();
        unsigned long nshapes_para = dump.getNShapes_Para();
        unsigned long nshapes_pcon = dump.getNShapes_Pcon();
        unsigned long nshapes_pgon = dump.getNShapes_Pgon();
        unsigned long nshapes_simplepol = dump.getNShapes_SimplePolygonBrep();
        unsigned long nshapes_trap = dump.getNShapes_Trap();
        unsigned long nshapes_trd = dump.getNShapes_Trd();
        unsigned long nshapes_tube = dump.getNShapes_Tube();
        unsigned long nshapes_tubs = dump.getNShapes_Tubs();
        unsigned long nshapes_twisted = dump.getNShapes_TwistedTrap();
        unsigned long nshapes_unid = dump.getNShapes_UnidentifiedShape();
        unsigned long nshapes_shift = dump.getNShapes_Shift();
        unsigned long nshapes_union = dump.getNShapes_Union();
        unsigned long nshapes_intersection = dump.getNShapes_Intersection();
        unsigned long nshapes_subtraction = dump.getNShapes_Subtraction();

        mmap["PhysVol"] = nphysvols;
        mmap["FullPhysVol"] = nfullphysvols;
        mmap["LogVol"] = nlogvols;
        mmap["Element"] = nelements;
        mmap["Material"] = nmaterials;
        mmap["Alignable"] = nalignables;
        mmap["Function"] = nfunctions;
        mmap["SerialTransformer"] = nserialtransformers;
        mmap["Shape"] = nshapes;
        mmap["SerialDenominator"] = nserialdenominators;
        mmap["ChildrenConnections"] = nchildrenconnections;
        mmap["Transform"] = ntransforms;
        mmap["SerialIdentifier"] = nserialidentifiers;
        mmap["IdentifierTag"] = nidentifiertags;
        mmap["NameTag"] = nnametags;
        mmap["Shape"] = nshapes;
        mmap["Shape_Box"] = nshapes_box;
        mmap["Shape_Cons"] = nshapes_cons;
        mmap["Shape_Para"] = nshapes_para;
        mmap["Shape_Pcon"] = nshapes_pcon;
        mmap["Shape_Pgon"] = nshapes_pgon;
        mmap["Shape_SimplePolygonBrep"] = nshapes_simplepol;
        mmap["Shape_Trap"] = nshapes_trap;
        mmap["Shape_Trd"] = nshapes_trd;
        mmap["Shape_Tube"] = nshapes_tube;
        mmap["Shape_Tubs"] = nshapes_tubs;
        mmap["Shape_TwistedTrap"] = nshapes_twisted;
        mmap["Shape_UnidentifiedShape"] = nshapes_unid;
        mmap["Shape_Shift"] = nshapes_shift;
        mmap["Shape_Union"] = nshapes_union;
        mmap["Shape_Intersection"] = nshapes_intersection;
        mmap["Shape_Subtraction"] = nshapes_subtraction;

        return mmap;
    }
    static std::map<std::string, unsigned long> countLoadedNodesFromReadAction(
        GeoModelIO::ReadGeoModel& read) {
        std::map<std::string, unsigned long> mmap;

        unsigned long nphysvols = read.getNPhysVols();
        unsigned long nfullphysvols = read.getNFullPhysVols();
        unsigned long nlogvols = read.getNLogVols();
        unsigned long nelements = read.getNElements();
        unsigned long nmaterials = read.getNMaterials();
        unsigned long nalignables = read.getNAlignableTransforms();
        unsigned long nfunctions = read.getNFunctions();
        unsigned long nserialtransformers = read.getNSerialTransformers();
        unsigned long nserialdenominators = read.getNSerialDenominators();
        unsigned long nchildrenconnections = read.getNChildrenConnections();
        // unsigned nrootvolume = read.getNRootVolume();
        unsigned long ntransforms = read.getNTransforms();
        unsigned long nserialidentifiers = read.getNSerialIdentifiers();
        unsigned long nidentifiertags = read.getNIdentifierTags();
        unsigned long nnametags = read.getNNameTags();
        // get shapes
        unsigned long nshapes = read.getNShapes(); // TODO: to be removed later
        unsigned long nshapes_box = read.getNShapes_Box();
        unsigned long nshapes_cons = read.getNShapes_Cons();
        unsigned long nshapes_para = read.getNShapes_Para();
        unsigned long nshapes_pcon = read.getNShapes_Pcon();
        unsigned long nshapes_pgon = read.getNShapes_Pgon();
        unsigned long nshapes_simplepol = read.getNShapes_SimplePolygonBrep();
        unsigned long nshapes_trap = read.getNShapes_Trap();
        unsigned long nshapes_trd = read.getNShapes_Trd();
        unsigned long nshapes_tube = read.getNShapes_Tube();
        unsigned long nshapes_tubs = read.getNShapes_Tubs();
        unsigned long nshapes_twisted = read.getNShapes_TwistedTrap();
        unsigned long nshapes_unid = read.getNShapes_UnidentifiedShape();
        unsigned long nshapes_shift = read.getNShapes_Shift();
        unsigned long nshapes_union = read.getNShapes_Union();
        unsigned long nshapes_intersection = read.getNShapes_Intersection();
        unsigned long nshapes_subtraction = read.getNShapes_Subtraction();

        mmap["PhysVol"] = nphysvols;
        mmap["FullPhysVol"] = nfullphysvols;
        mmap["LogVol"] = nlogvols;
        mmap["Element"] = nelements;
        mmap["Material"] = nmaterials;
        mmap["Alignable"] = nalignables;
        mmap["Function"] = nfunctions;
        mmap["SerialTransformer"] = nserialtransformers;
        mmap["Shape"] = nshapes;
        mmap["SerialDenominator"] = nserialdenominators;
        mmap["ChildrenConnections"] = nchildrenconnections;
        mmap["Transform"] = ntransforms;
        mmap["SerialIdentifier"] = nserialidentifiers;
        mmap["IdentifierTag"] = nidentifiertags;
        mmap["NameTag"] = nnametags;
        mmap["Shape"] = nshapes;
        mmap["Shape_Box"] = nshapes_box;
        mmap["Shape_Cons"] = nshapes_cons;
        mmap["Shape_Para"] = nshapes_para;
        mmap["Shape_Pcon"] = nshapes_pcon;
        mmap["Shape_Pgon"] = nshapes_pgon;
        mmap["Shape_SimplePolygonBrep"] = nshapes_simplepol;
        mmap["Shape_Trap"] = nshapes_trap;
        mmap["Shape_Trd"] = nshapes_trd;
        mmap["Shape_Tube"] = nshapes_tube;
        mmap["Shape_Tubs"] = nshapes_tubs;
        mmap["Shape_TwistedTrap"] = nshapes_twisted;
        mmap["Shape_UnidentifiedShape"] = nshapes_unid;
        mmap["Shape_Shift"] = nshapes_shift;
        mmap["Shape_Union"] = nshapes_union;
        mmap["Shape_Intersection"] = nshapes_intersection;
        mmap["Shape_Subtraction"] = nshapes_subtraction;

        return mmap;
    }

    static void printNodesMap(std::map<std::string, unsigned long> mmap) {
        for (auto& node : mmap) {
            std::cout << node.first << ": " << node.second << std::endl;
        }
    }

    static void printKeyMaps(std::string key,
                             std::map<std::string, unsigned long> m1,
                             std::map<std::string, unsigned long> m2) {
        std::cout << "number of " << key << " : " << m1[key] << " -- "
                  << m2[key] << std::endl;
    }

    static std::vector<std::string> getMapKeys()
    {
        std::vector<std::string> keys{"PhysVol",
                                      "FullPhysVol",
                                      "LogVol",
                                      "Element",
                                      "Material",
                                      "Alignable",
                                      "Function",
                                      "SerialTransformer",
                                      "Transform",
                                      "SerialIdentifier",
                                      "IdentifierTag",
                                      "NameTag"
                                      "SerialDenominator",
                                      "ChildrenConnections",
                                      "Shape",
                                      "Shape_Box",
                                      "Shape_Cons",
                                      "Shape_Para",
                                      "Shape_Pcon",
                                      "Shape_Pgon",
                                      "Shape_SimplePolygonBrep",
                                      "Shape_Trap",
                                      "Shape_Trd",
                                      "Shape_Tube",
                                      "Shape_Tubs",
                                      "Shape_TwistedTrap",
                                      "Shape_UnidentifiedShape",
                                      "Shape_Shift",
                                      "Shape_Union",
                                      "Shape_Intersection",
                                      "Shape_Subtraction"};
        return keys;
    }
    static void printCompareTwoNodesMaps(
        std::map<std::string, unsigned long> m1,
        std::map<std::string, unsigned long> m2) {
        std::vector<std::string> keys = getMapKeys();
        for (auto& key : keys) {
            printKeyMaps(key, m1, m2);
        }
    }

    static void printDifferencesBetweenTwoNodesMaps(
        std::map<std::string, unsigned long> m1,
        std::map<std::string, unsigned long> m2)
    {
        for (const auto &entry : m1)
        {
            const std::string key = entry.first;
            const unsigned i1 = m1[key];
            const unsigned i2 = m2[key];
            if (i1 != i2)
            {
                std::cout << "ERROR! ==> the number of '" << key << "' are different! " << i1 << " <-> " << i2 << std::endl;
            }
        }
    }

    static std::map<std::string, unsigned long> initNodesMap() {
        std::map<std::string, unsigned long> nmap;
        std::vector<std::string> keys = getMapKeys();
        // init the map
        for (auto& key : keys) {
            nmap[key] = 0;
        }
        return nmap;
    }

    static void checkTwoNodesMapsSameSize(
        std::map<std::string, unsigned long> m1,
        std::map<std::string, unsigned long> m2)
    {
        if (m1.size() != m2.size())
        {
            THROW_EXCEPTION("ERROR! Maps are not of the same size!");
        }
    }
};

}  // namespace GeoModelIO

#endif

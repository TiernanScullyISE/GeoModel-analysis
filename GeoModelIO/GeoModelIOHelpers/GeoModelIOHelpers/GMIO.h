
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

#include "filesystem"

#include "GeoModelKernel/throwExcept.h"

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoEllipticalTube.h"
#include "GeoModelKernel/GeoGenericTrap.h"
#include "GeoModelKernel/GeoPara.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelKernel/GeoShapeIntersection.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoSimplePolygonBrep.h"
#include "GeoModelKernel/GeoTessellatedSolid.h"
#include "GeoModelKernel/GeoTorus.h"
#include "GeoModelKernel/GeoTrap.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoTwistedTrap.h"
#include "GeoModelKernel/GeoUnidentifiedShape.h"




namespace GeoModelIO {

class IO {
   public:

    using StatMap_t = std::map<std::string, std::size_t>;

    static 
        std::shared_ptr<GMDBManager> saveToDB(const PVConstLink world, 
                                              const std::string& path,
                                              unsigned loglevel = 0,
                                              bool forceDelete = false) {
        // Check if the output DB file exists already. 
        // - If yes and the 'forceDelete' option is set to 'true' by the user, 
        // then delete it before trying to create the new one; that is, 
        // the 'forceDelete' option replaces the existing '.db' file.
        if (std::filesystem::exists(path)) {
            if (forceDelete) {
                std::cout<<"GeoModelIO - INFO: Delete already existing "<<path<<" object "<<std::endl;
                std::filesystem::remove(path);
            } else {
                THROW_EXCEPTION("The database '"<<path<<"' already exist. Don't want to clobber with existing object");
            }
        }

        // open the DB connection
        auto db = std::make_unique<GMDBManager>(path);

        // check the DB connection
        if (!db->checkIsDBOpen()) {
            THROW_EXCEPTION("It was not possible to open the DB correctly!");
        }

        // Dump the tree volumes to a local file
        std::cout << "Dumping the GeoModel geometry to the DB file..." << std::endl;
        // init the GeoModel node action
        GeoModelIO::WriteGeoModel dumpGeoModelGraph(*db);
        // set loglevel of write action, if > 0
        if (loglevel > 0) {
            dumpGeoModelGraph.setLogLevel(loglevel);
        }
        world->exec(&dumpGeoModelGraph);  // visit all GeoModel nodes
        dumpGeoModelGraph.saveToDB();     // save to the SQlite DB file
        std::cout << "DONE. Geometry saved." << std::endl;
        return db;
    }


    static GeoModelIO::ReadGeoModel getReaderDB(const std::string path, unsigned loglevel = 0) {
        // open the DB
        auto db = std::make_unique<GMDBManager>(path);
        if (!db->checkIsDBOpen()) {
            THROW_EXCEPTION("It was not possible to open the DB correctly!");
        }
        /* setup the GeoModel reader */
        GeoModelIO::ReadGeoModel geoReader{std::move(db)};
        /// Ensure that the intermediate maps are not cleaned
        geoReader.m_autoClean = false;
        // set loglevel of read action, if > 0
        if (loglevel > 0) {
            geoReader.setLogLevel(loglevel);
        }
        return geoReader;
    }
    
    static PVConstLink loadDB(const std::string& path, unsigned loglevel = 0) {
            return getReaderDB(path, loglevel).buildGeoModel();
    }

    static std::vector<std::string> supportedShapes() {
        return {
            GeoBox::getClassType(),
            GeoCons::getClassType(),
            GeoEllipticalTube::getClassType(),
            GeoGenericTrap::getClassType(),
            GeoPara::getClassType(),
            GeoPcon::getClassType(),
            GeoPgon::getClassType(),
            GeoShapeIntersection::getClassType(),
            GeoShapeShift::getClassType(),
            GeoShapeSubtraction::getClassType(),
            GeoShapeUnion::getClassType(),
            GeoSimplePolygonBrep::getClassType(),
            GeoTessellatedSolid::getClassType(),
            GeoTorus::getClassType(),
            GeoTrap::getClassType(),
            GeoTrd::getClassType(),
            GeoTube::getClassType(),
            GeoTubs::getClassType(),
            GeoTwistedTrap::getClassType(),
            GeoUnidentifiedShape::getClassType()
        };
    }
    static StatMap_t countNodesFromDB(GMDBManager& db) {
        // map to populate and return
        StatMap_t mmap;
        // get the number of all nodes in the DB from the DB manager
        mmap["PhysVol"]                 = db.getTableFromNodeType_String("GeoPhysVol").size();
        mmap["FullPhysVol"]             = db.getTableFromNodeType_String("GeoFullPhysVol").size();
        mmap["LogVol"]                  = db.getTableFromNodeType_VecVecData("GeoLogVol").size();
        mmap["Element"]                 = db.getTableFromNodeType_VecVecData("GeoElement").size();
        mmap["Material"]                = db.getTableFromNodeType_VecVecData("GeoMaterial").size();
        mmap["Alignable"]               = db.getTableFromNodeType_String("GeoAlignableTransform").size();
        mmap["Function"]                = db.getTableFromNodeType_VecVecData("Function").size();
        mmap["SerialTransformer"]       = db.getTableFromNodeType_String("GeoSerialTransformer").size();
        mmap["SerialDenominator"]       = db.getTableFromNodeType_String("GeoSerialDenominator").size();
        mmap["Transform"]               = db.getTableFromNodeType_VecVecData("GeoTransform").size();
        mmap["SerialIdentifier"]        = db.getTableFromNodeType_VecVecData("GeoSerialIdentifier").size();
        mmap["IdentifierTag"]           = db.getTableFromNodeType_VecVecData("GeoIdentifierTag").size();
        mmap["NameTag"]                 = db.getTableFromNodeType_VecVecData("GeoNameTag").size();
        for (const std::string& shape : supportedShapes()) {
            mmap[shape] = db.getTableFromNodeType_VecVecData(shape).size();
        }
        return mmap;
    }

    static StatMap_t countTreeMemoryNodesFromVolume(const PVConstLink world, unsigned loglevel = 0) {
        StatMap_t mmap;

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
        mmap["SerialDenominator"] = nserialdenominators;
        mmap["ChildrenConnections"] = nchildrenconnections;
        mmap["Transform"] = ntransforms;
        mmap["SerialIdentifier"] = nserialidentifiers;
        mmap["IdentifierTag"] = nidentifiertags;
        mmap["NameTag"] = nnametags;
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
    static void printNodesMap(const StatMap_t& mmap) {
        for (auto& [key, counts] : mmap) {
            std::cout << key << ": " << counts << std::endl;
        }
    }

    static std::size_t readKey(const std::string& key, const StatMap_t& m){
        auto itr = m.find(key);
        return itr != m.end() ? itr->second : 0;
    }

    static void printKeyMaps(const std::string& key,
                             const StatMap_t& m1,
                             const StatMap_t& m2) {
        std::cout << "number of " << key << " : " <<readKey(key, m1) 
                  << " -- " << readKey(key, m2) << std::endl;
    }


    static void printCompareTwoNodesMaps(const StatMap_t& m1,
                                         const StatMap_t& m2) {
        for (const auto& [key, _] : m1) {
            printKeyMaps(key, m1, m2);
        }
    }

    static void printDifferencesBetweenTwoNodesMaps(const StatMap_t& m1,
                                                    const StatMap_t& m2) {
        for (const auto &entry : m1) {
            const std::string key = entry.first;
            const unsigned i1 = readKey(key, m1);
            const unsigned i2 = readKey(key, m2);
            if (i1 != i2) {
                std::cout << "ERROR! ==> the number of '" << key << "' are different! " << i1 << " <-> " << i2 << std::endl;
            }
        }
    }
};

}  // namespace GeoModelIO

#endif
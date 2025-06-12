/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ReadGeoModel.cpp
 *
 *  Created on: May 20, 2016
 *  Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 * Major updates:
 *  - Feb 2019, R.M.Bianchi
 *  - Mar 2020, R.M.Bianchi
 *  - Mar 2020, boudreau
 *  - May 2020, R.M.Bianchi
 *  - Aug 2020, R.M.Bianchi - Added support to read published FullPhysVols and
 * AlignableTransforms back in
 *  - Aug 2021, R.M.Bianchi <riccardo.maria.bianchi@cern.ch> - Added support for
 * GeoSerialIdentifier and GeoIdentifierTag
 *  - Jun 2022, R.M.Bianchi <riccardo.maria.bianchi@cern.ch>
 *              Fixed the duplication of VPhysVol instances due to a wrong key,
 *              which was used for caching volumes that were built already.
 *              The copyNumber was wrongly used together with tableID and volID
 *              For details, see:
 *              https://gitlab.cern.ch/GeoModelDev/GeoModel/-/issues/39
 *  - May 2024, R.M.Bianchi <riccardo.maria.bianchi@cern.ch>
 *              Major re-write: moved to the new DB schema based on numeric data
 *  - Jun 2024, R.Xue  <r.xue@cern.ch><rux23@pitt.edu>
 *              Added methods to read in virtual surfaces from .db files
 *  - Oct 2024, Riccardo Maria Bianchi, <riccardo.maria.bianchi@cern.ch>,                                  
 *              Support for the EllipticalTube shape was added. 
 *
 */

// local includes
#include "BuildGeoShapes_Box.h"
#include "BuildGeoShapes_EllipticalTube.h"
#include "BuildGeoShapes_Tube.h"
#include "BuildGeoShapes_Cons.h"
#include "BuildGeoShapes_Para.h"
#include "BuildGeoShapes_Pcon.h"
#include "BuildGeoShapes_Pgon.h"
#include "BuildGeoShapes_Trap.h"
#include "BuildGeoShapes_Trd.h"
#include "BuildGeoShapes_Tubs.h"
#include "BuildGeoShapes_Torus.h"
#include "BuildGeoShapes_TwistedTrap.h"
#include "BuildGeoShapes_SimplePolygonBrep.h"
#include "BuildGeoShapes_GenericTrap.h"
#include "BuildGeoShapes_UnidentifiedShape.h"

#include "BuildGeoVSurface.h"

#include "GeoModelRead/ReadGeoModel.h"

// TFPersistification includes
#include "TFPersistification/TransFunctionInterpreter.h"

// GeoModelKernel includes
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoGraphNode.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoSerialDenominator.h"
#include "GeoModelKernel/GeoSerialIdentifier.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelHelpers/TransformToStringConverter.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoVSurface.h"
#include "GeoModelKernel/GeoVSurfaceShape.h"
#include "GeoModelKernel/GeoRectSurface.h"
#include "GeoModelKernel/GeoTrapezoidSurface.h"
#include "GeoModelKernel/GeoAnnulusSurface.h"
#include "GeoModelKernel/GeoDiamondSurface.h"

// GeoModel shapes
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoEllipticalTube.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoGenericTrap.h"
#include "GeoModelKernel/GeoPara.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelKernel/GeoShape.h"
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

#include "GeoModelHelpers/variantHelpers.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelHelpers/StringUtils.h"

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS \
    GeoModelKernelUnits  // so we will get, e.g., 'GeoModelKernelUnits::cm'

// VP1Base
//  TODO: we should get rid of VP1Base::VP1Msg dependency, since GeoModelRead
//  should not depend on VP1 packages. Maybe we can move VP1Msg to a standalone
//  package.
// #include "VP1Base/VP1Msg.h"

// C++ includes
#include <cstdlib> /* exit, EXIT_FAILURE */

#include <chrono>  /* system_clock */
#include <ctime>   /* std::time */
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

// mutexes for synchronized access to containers and output streams in
// multi-threading mode
std::mutex muxVPhysVol;
std::mutex muxCout;

using namespace GeoGenfun;
using namespace GeoXF;

// Set default (false) values for the debugging variables.
#ifndef GEOMODEL_IO_READ_DEBUG
#define GEOMODEL_IO_READ_DEBUG false
#endif  // not GEOMODEL_IO_READ_DEBUG
#ifndef GEOMODEL_IO_DEBUG_VERBOSE
#define GEOMODEL_IO_DEBUG_VERBOSE false
#endif  // not GEOMODEL_IO_DEBUG_VERBOSE
#ifndef GEOMODEL_IO_READ_TIMING
#define GEOMODEL_IO_READ_TIMING false
#endif  // not GEOMODEL_IO_READ_TIMING

namespace GeoModelIO {

ReadGeoModel::ReadGeoModel(GMDBManager* db, unsigned long* progress)
    : m_loglevel(0),
      m_deepDebug(GEOMODEL_IO_DEBUG_VERBOSE),
      m_debug(GEOMODEL_IO_READ_DEBUG),
      m_timing(GEOMODEL_IO_READ_TIMING),
      m_runMultithreaded(false),
      m_runMultithreaded_nThreads(0),
      m_progress(nullptr) {
    // Check if the user asked for debug messages
    if ("" != GeoStrUtils::getEnvVar("GEOMODEL_ENV_IO_LOGLEVEL_1")) {
        m_loglevel = 1;
        std::cout << "You defined the GEOMODEL_ENV_IO_DEBUG variable, so you "
                     "will see a verbose output."
                  << std::endl;
    }
    // Check if the user asked for verbose debug messages
    if ("" != GeoStrUtils::getEnvVar("GEOMODEL_ENV_IO_LOGLEVEL_2")) {
        m_loglevel = 2;
        std::cout << "You defined the GEOMODEL_ENV_IO_READ_DEBUG_VERBOSE "
                     "variable, so you will see a verbose output."
                  << std::endl;
    }
    // Check if the user asked for timing output
    if ("" != GeoStrUtils::getEnvVar("GEOMODEL_ENV_IO_READ_TIMING")) {
        m_timing = true;
        std::cout << "You defined the GEOMODEL_ENV_IO_READ_TIMING variable, so "
                     "you will see a timing measurement in the output."
                  << std::endl;
    }

    if (progress != nullptr) {
        m_progress = progress;
    }

    // open the geometry file
    m_dbManager = db;
    if (m_dbManager->checkIsDBOpen()) {
        if (m_loglevel >= 1) std::cout << "OK! Database is open!";
    } else {
        std::cout << "ERROR!! Database is NOT open!";
        return;
    }
    // build caches
    m_dbManager->loadGeoNodeTypesAndBuildCache();
    m_dbManager->createTableDataCaches();

    // Check if the user asked for running in serial or multi-threading mode
    if ("" != GeoStrUtils::getEnvVar("GEOMODEL_ENV_IO_NTHREADS")) {
        int nThreads = std::stoi(GeoStrUtils::getEnvVar("GEOMODEL_ENV_IO_NTHREADS"));
        if (nThreads == 0) {
            std::cout << "Info: You set the GEOMODEL_ENV_IO_NTHREADS to '"
                      << nThreads
                      << "'; thus, GeoModelIO will be run in serial mode."
                      << std::endl;
            m_runMultithreaded_nThreads = nThreads;
            m_runMultithreaded = false;
        } else if (nThreads > 0) {
            std::cout
                << "Info: You set the GEOMODEL_ENV_IO_NTHREADS to '" << nThreads
                << "'; thus, GeoModelIO will use that number of worker threads."
                << std::endl;
            m_runMultithreaded_nThreads = nThreads;
            m_runMultithreaded = true;
        } else if (nThreads == -1) {
            std::cout << "Info: You set the GEOMODEL_ENV_IO_NTHREADS to '"
                      << nThreads
                      << "'; thus, GeoModelIO will use the number of threads "
                         "supported by the platform."
                      << std::endl;
            m_runMultithreaded_nThreads = nThreads;
            m_runMultithreaded = true;
        }
    }
    // if the user did not specify the number of threads, then we just use the
    // hardware-supported number of threads on the platform
    else {
        m_runMultithreaded_nThreads = -1;
        m_runMultithreaded = true;
    }

    // load the data from the DB
    loadDB();
}

ReadGeoModel::~ReadGeoModel() {
    delete m_builderShape_Box;
    delete m_builderShape_EllipticalTube;
    delete m_builderShape_Tube;
    delete m_builderShape_Pcon;
    delete m_builderShape_Cons;
    delete m_builderShape_Pgon;
    delete m_builderShape_Trap;
    delete m_builderShape_Trd;
    delete m_builderShape_Tubs;
    delete m_builderShape_Torus;
    delete m_builderShape_TwistedTrap;
    delete m_builderShape_SimplePolygonBrep;
    delete m_builderShape_GenericTrap;
    delete m_builderShape_UnidentifiedShape;
    m_builderShape_Box = nullptr;
    m_builderShape_EllipticalTube = nullptr;
    m_builderShape_Tube = nullptr;
    m_builderShape_Pcon = nullptr;
    m_builderShape_Cons = nullptr;
    m_builderShape_Pgon = nullptr;
    m_builderShape_Trap = nullptr;
    m_builderShape_Trd = nullptr;
    m_builderShape_Tubs = nullptr;
    m_builderShape_Torus = nullptr;
    m_builderShape_TwistedTrap = nullptr;
    m_builderShape_SimplePolygonBrep = nullptr;
    m_builderShape_GenericTrap = nullptr;
    m_builderShape_UnidentifiedShape = nullptr;
}

// // FIXME: TODO: move to an utility class
// std::string ReadGeoModel::getEnvVar(std::string const& key) const {
//     char* val = std::getenv(key.c_str());
//     return val == NULL ? std::string("") : std::string(val);
// }

const GeoVPhysVol* ReadGeoModel::buildGeoModel() {
    if (m_loglevel >= 2)
        std::cout << "ReadGeoModel::buildGeoModel()" << std::endl;

    GeoVPhysVol* rootVolume = buildGeoModelPrivate();

    // warn the user if there are unknown/unhalded shapes
    if (m_unknown_shapes.size() > 0) {
        std::cout << "\tWARNING!! There were unknwon shapes:" << std::endl;
        for (auto it = m_unknown_shapes.begin(); it != m_unknown_shapes.end();
             it++) {
            std::cout << "\t\t---> " << *it << std::endl;
        }
        std::cout << "\tRemember: unknown shapes are rendered with a dummy "
                     "cube of 30cm side length.\n\n"
                  << std::endl;
    }
    return rootVolume;
}

void ReadGeoModel::loadDB() {
    // *** get all data from the DB ***
    // timing: get start time
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();  
    // get all GeoModel nodes from the DB
    // m_shapes = m_dbManager->getTableFromNodeType("GeoShape");
    m_physVols = m_dbManager->getTableFromNodeType_String("GeoPhysVol");
    m_fullPhysVols = m_dbManager->getTableFromNodeType_String("GeoFullPhysVol");
    m_transforms = m_dbManager->getTableFromNodeType_String("GeoTransform");
    m_alignableTransforms =
        m_dbManager->getTableFromNodeType_String("GeoAlignableTransform");
    m_serialDenominators =
        m_dbManager->getTableFromNodeType_String("GeoSerialDenominator");
    m_serialIdentifiers =
        m_dbManager->getTableFromNodeType_String("GeoSerialIdentifier");
    m_identifierTags = m_dbManager->getTableFromNodeType_String("GeoIdentifierTag");
    m_serialTransformers =
        m_dbManager->getTableFromNodeType_String("GeoSerialTransformer");
    m_nameTags = m_dbManager->getTableFromNodeType_String("GeoNameTag");

    // containers to store data that have been moved to the new DB schema
    m_functions = m_dbManager->getTableFromNodeType_VecVecData("Function");
    m_logVols = m_dbManager->getTableFromNodeType_VecVecData("GeoLogVol");
    m_elements = m_dbManager->getTableFromNodeType_VecVecData("GeoElement");
    m_materials = m_dbManager->getTableFromNodeType_VecVecData("GeoMaterial");
    m_materials_Data = m_dbManager->getTableFromTableName_VecVecData("Materials_Data");

    // shapes from the new DB schema
    m_shapes_Box = m_dbManager->getTableFromNodeType_VecVecData("GeoBox");
    m_shapes_EllipticalTube = m_dbManager->getTableFromNodeType_VecVecData("GeoEllipticalTube");
    m_shapes_Tube = m_dbManager->getTableFromNodeType_VecVecData("GeoTube");
    m_shapes_Cons = m_dbManager->getTableFromNodeType_VecVecData("GeoCons");
    m_shapes_Para = m_dbManager->getTableFromNodeType_VecVecData("GeoPara");
    m_shapes_Trap = m_dbManager->getTableFromNodeType_VecVecData("GeoTrap");
    m_shapes_Trd = m_dbManager->getTableFromNodeType_VecVecData("GeoTrd");
    m_shapes_Tubs = m_dbManager->getTableFromNodeType_VecVecData("GeoTubs");
    m_shapes_Torus = m_dbManager->getTableFromNodeType_VecVecData("GeoTorus");
    m_shapes_TwistedTrap = m_dbManager->getTableFromNodeType_VecVecData("GeoTwistedTrap");
    m_shapes_UnidentifiedShape = m_dbManager->getTableFromNodeType_VecVecData("GeoUnidentifiedShape");
    
    // shapes with variable build parameters stored in a separate table
    m_shapes_Pcon = m_dbManager->getTableFromNodeType_VecVecData("GeoPcon");
    m_shapes_Pgon = m_dbManager->getTableFromNodeType_VecVecData("GeoPgon");
    m_shapes_SimplePolygonBrep = m_dbManager->getTableFromNodeType_VecVecData("GeoSimplePolygonBrep");
    m_shapes_GenericTrap = m_dbManager->getTableFromNodeType_VecVecData("GeoGenericTrap");

    // shapes' data, when needed by shapes that have variable numbers of build parameters
    m_shapes_Pcon_data = m_dbManager->getTableFromTableName_VecVecData("Shapes_Pcon_Data");
    m_shapes_Pgon_data = m_dbManager->getTableFromTableName_VecVecData("Shapes_Pgon_Data");
    m_shapes_SimplePolygonBrep_data = m_dbManager->getTableFromTableName_VecVecData("Shapes_SimplePolygonBrep_Data");
    m_shapes_GenericTrap_data = m_dbManager->getTableFromTableName_VecVecData("Shapes_GenericTrap_Data");

    // shape operators & boolean shapes
    m_shapes_Shift = m_dbManager->getTableFromNodeType_VecVecData("GeoShapeShift");
    m_shapes_Subtraction = m_dbManager->getTableFromNodeType_VecVecData("GeoShapeSubtraction");
    m_shapes_Intersection = m_dbManager->getTableFromNodeType_VecVecData("GeoShapeIntersection");
    m_shapes_Union = m_dbManager->getTableFromNodeType_VecVecData("GeoShapeUnion");

    // get the Function's expression data
    // m_funcExprData = m_dbManager->getTableFromTableNameVecVecData("FuncExprData");
    m_funcExprData = m_dbManager->getTableFromTableName_DequeDouble("FuncExprData");
    
    // get virtual surface shape and class from DB
    m_rectangle_surface = m_dbManager->getTableFromNodeType_VecVecData("RectangleSurface");
    m_trapezoid_surface = m_dbManager->getTableFromNodeType_VecVecData("TrapezoidSurface");
    m_annulus_surface = m_dbManager->getTableFromNodeType_VecVecData("AnnulusSurface");
    m_diamond_surface = m_dbManager->getTableFromNodeType_VecVecData("DiamondSurface");
    m_VSurface = m_dbManager->getTableFromNodeType_VecVecData("GeoVSurface");

    // get the children table from DB
    m_allchildren = m_dbManager->getChildrenTable();
    // get the root volume data
    m_root_vol_data = m_dbManager->getRootPhysVol();
    // get DB metadata
    m_tableID_toTableName = m_dbManager->getAll_TableIDsNodeTypes();
    m_tableName_toTableID = m_dbManager->getAll_NodeTypesTableIDs();

    auto end = std::chrono::system_clock::now();  // timing: get end time
    auto diff =
        std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    if (m_timing || (m_loglevel >= 1)) {
        std::cout << "*** Time taken to fetch GeoModel data from the database: "
                  << diff << " [s]" << std::endl;
    }
}

GeoVPhysVol* ReadGeoModel::buildGeoModelPrivate() {
    // *** build all nodes ***
    std::chrono::system_clock::time_point start =
        std::chrono::system_clock::now();  // timing: get start time
    // parallel mode:
    if (m_runMultithreaded) {
        if (m_loglevel >= 1)
            std::cout << "Building nodes concurrently..." << std::endl;
        std::thread t2(&ReadGeoModel::buildAllElements, this);
        //  std::thread t7(&ReadGeoModel::buildAllFunctions, this); //
        //  FIXME: implement cache for Functions

        std::thread t8(&ReadGeoModel::buildAllTransforms, this);
        std::thread t9(&ReadGeoModel::buildAllAlignableTransforms, this);
        std::thread t10(&ReadGeoModel::buildAllSerialDenominators, this);
        std::thread t13(&ReadGeoModel::buildAllSerialIdentifiers, this);
        std::thread t14(&ReadGeoModel::buildAllIdentifierTags, this);
        std::thread t11(&ReadGeoModel::buildAllNameTags, this);

        t8.join();  // ok, all Transforms have been built
        t9.join();  // ok, all AlignableTransforms have been built
        
        // needs Transforms and AlignableTransforms for Shift boolean shapes
        // std::thread t1(&ReadGeoModel::buildAllShapes, this); // TODO: old, to be removed 
        std::thread t15(&ReadGeoModel::buildAllShapes_Box, this);
        std::thread t16(&ReadGeoModel::buildAllShapes_Tube, this);
        std::thread t17(&ReadGeoModel::buildAllShapes_Pcon, this);
        std::thread t18(&ReadGeoModel::buildAllShapes_Cons, this);
        std::thread t19(&ReadGeoModel::buildAllShapes_Para, this);
        std::thread t20(&ReadGeoModel::buildAllShapes_Pgon, this);
        std::thread t21(&ReadGeoModel::buildAllShapes_Trap, this);
        std::thread t22(&ReadGeoModel::buildAllShapes_Trd, this);
        std::thread t23(&ReadGeoModel::buildAllShapes_Tubs, this);
        std::thread t24(&ReadGeoModel::buildAllShapes_TwistedTrap, this);
        std::thread t25(&ReadGeoModel::buildAllShapes_SimplePolygonBrep, this);
        std::thread t26(&ReadGeoModel::buildAllShapes_UnidentifiedShape, this);
        std::thread t27(&ReadGeoModel::buildAllShapes_Torus, this);
        std::thread t28(&ReadGeoModel::buildAllShapes_GenericTrap, this);
        std::thread t29(&ReadGeoModel::buildAllShapes_EllipticalTube, this);
        

        t2.join();  // ok, all Elements have been built
        // This below needs Elements to be built
        std::thread t3(&ReadGeoModel::buildAllMaterials, this);

        // t1.join();  // ok, all Shapes have been built
        t15.join();  // ok, all Shapes-Box have been built
        t16.join();  // ok, all Shapes-Tube have been built
        t17.join();  // ok, all Shapes-Pcon have been built
        t18.join();  // ok, all Shapes-Cons have been built
        t19.join();  // ok, all Shapes-Para have been built
        t20.join();  // ok, all Shapes-Pgon have been built
        t21.join();  // ok, all Shapes-Trap have been built
        t22.join();  // ok, all Shapes-Trd have been built
        t23.join();  // ok, all Shapes-Tubs have been built
        t24.join();  // ok, all Shapes-TwistedTrap have been built
        t25.join();  // ok, all Shapes-SimplePolygonBrep have been built
        t26.join();  // ok, all Shapes-UnidentifiedShape have been built
        t27.join();  // ok, all Shapes-Torus have been built
        t28.join();  // ok, all Shapes-GenericTrap have been built
        t29.join();  // ok, all Shapes-EllipticalTube have been built

	    // Build boolean shapes and shape operators,
        // this needs Shapes to be built
        std::thread t99(&ReadGeoModel::buildAllShapes_Operators, this);
        t99.join();  // ok, all Shapes-Operators have been built
        
        t3.join();  // ok, all Materials have been built
        // needs Shapes and Materials
        std::thread t4(&ReadGeoModel::buildAllLogVols, this);

        t4.join();  // ok, all LogVols have been built
        // needs LogVols
        std::thread t5(&ReadGeoModel::buildAllPhysVols, this);
        std::thread t6(&ReadGeoModel::buildAllFullPhysVols, this);
        std::thread t50(&ReadGeoModel::buildAllVSurfaces, this);
        
        t5.join();  // ok, all PhysVols have been built
        t6.join();  // ok, all FullPhysVols have been built
        t50.join();        
        //  t7.join(); // ok, all Functions have been built
        // needs Functions, PhysVols, FullPhysVols
        std::thread t12(&ReadGeoModel::buildAllSerialTransformers, this);

        t10.join();  // ok, all SerialDenominators have been built
        t11.join();  // ok, all NameTags have been built
        t12.join();  // ok, all SerialTransformers have been built
        t13.join();  // ok, all SerialIdentifiers have been built
        t14.join();  // ok, all IdentifierTags have been built
    }
    // serial mode:
    else {
        if (m_loglevel >= 1)
            std::cout << "Building nodes serially..." << std::endl;
        buildAllElements();
        // buildAllFunctions();
        buildAllTransforms();
        buildAllAlignableTransforms();
        buildAllSerialDenominators();
        buildAllSerialIdentifiers();
        buildAllIdentifierTags();
        buildAllNameTags();
        // buildAllShapes();
        buildAllShapes_Box();
        buildAllShapes_EllipticalTube();
        buildAllShapes_Tube();
        buildAllShapes_Pcon();
        buildAllShapes_Pgon();
        buildAllShapes_SimplePolygonBrep();
        buildAllShapes_GenericTrap();
        buildAllShapes_UnidentifiedShape();
        buildAllShapes_Cons();
        buildAllShapes_Para();
        buildAllShapes_Trap();
        buildAllShapes_Trd();
        buildAllShapes_Tubs();
        buildAllShapes_Torus();
        buildAllShapes_TwistedTrap();
        buildAllShapes_Operators();
        buildAllMaterials();
        buildAllLogVols();
        buildAllPhysVols();
        buildAllFullPhysVols();
        buildAllSerialTransformers();
        buildAllVSurfaces();          // Virtual Surface!
    }
    auto end = std::chrono::system_clock::now();  // timing: get end time
    auto diff =
        std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    if (m_timing || (m_loglevel >= 1)) {
        std::cout << "*** Time taken to build all GeoModel nodes: " << diff
                  << " [s]" << std::endl;
    }

    // *** recreate all mother-daughter relatioships between nodes ***
    start = std::chrono::system_clock::now();  // timing: get start time
    // loopOverAllChildrenInBunches_String(); // OLD
    loopOverAllChildrenInBunches_VecVecData(); //NEW
    end = std::chrono::system_clock::now();  // timing: get end time
    diff =
        std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    if (m_timing || (m_loglevel >= 1)) {
        std::cout << "*** Time taken to recreate all mother-daughter "
                     "relationships between nodes of the GeoModel tree: "
                  << diff << " [s]" << std::endl;
    }

    return getRootVolume();
}

//----------------------------------------
// loop over parent-child relationship data
void ReadGeoModel::loopOverAllChildrenRecords(
    const DBRowsList& records) {
    int nChildrenRecords = records.size();

    if (m_loglevel >= 1) {
        muxCout.lock();
        std::cout << "\nReadGeoModel::loopOverAllChildrenRecords -- Thread "
                  << std::this_thread::get_id() << " - processing "
                  << nChildrenRecords << " keys..." << std::endl;
        muxCout.unlock();
    }
    for (const auto& record : records) {
        processParentChild(record);
    }
}

//! Iterate over the list of shapes, build them all, and store their
//! pointers
void ReadGeoModel::buildAllShapes() {
    if (m_loglevel >= 1) std::cout << "Building all shapes...\n";
    size_t nSize = m_shapes.size();
    m_memMapShapes.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        const unsigned int shapeID = std::stoi(m_shapes[ii][0]);
        type_shapes_boolean_info
            shapes_info_sub;  // tuple to store the boolean shapes to
                              // complete at a second stage
        buildShape(shapeID, &shapes_info_sub);
        createBooleanShapeOperands(&shapes_info_sub);
    }
    if (nSize > 0) std::cout << "All " << nSize << " Shapes have been built!\n";
}

//! Iterate over the list of shapes, build them all, and store their
//! pointers
void ReadGeoModel::buildAllShapes_Operators()
{
    if (m_loglevel >= 1)
        std::cout << "Building all shape operators / boolean shapes...\n";

    // tuple to store the boolean shapes to
    // complete at a second stage
    boolean_shapes_operands_info shapes_info_sub;    
    
    if (m_loglevel >= 2)
        std::cout << "Building all Shift operators...\n";
    size_t nSize = m_shapes_Shift.size();
    m_memMapShapes_Shift.reserve(nSize);
    for (const auto &row : m_shapes_Shift)
    {
        
        // boolean_shapes_operands_info
        //     shapes_info_sub; // tuple to store the boolean shapes to
                             // complete at a second stage
        buildShapeOperator("Shift", row, &shapes_info_sub);
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " Shape-Operators-Shift have been built!\n";

    if (m_loglevel >= 2)
        std::cout << "Building all Subtraction boolean shapes...\n";
    nSize = m_shapes_Subtraction.size();
    m_memMapShapes_Subtraction.reserve(nSize);
    for (const auto &row : m_shapes_Subtraction)
    {
        
        // boolean_shapes_operands_info
        //     shapes_info_sub; // tuple to store the boolean shapes to
                             // complete at a second stage
        buildShapeOperator("Subtraction", row, &shapes_info_sub);
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " Shape-Operators-Subtraction have been built!\n";
    
    if (m_loglevel >= 2)
        std::cout << "Building all Intersection boolean shapes...\n";
    nSize = m_shapes_Intersection.size();
    m_memMapShapes_Intersection.reserve(nSize);
    for (const auto &row : m_shapes_Intersection)
    {
        
        // boolean_shapes_operands_info
        //     shapes_info_sub; // tuple to store the boolean shapes to
                             // complete at a second stage
        buildShapeOperator("Intersection", row, &shapes_info_sub);
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " Shape-Operators-Intersection have been built!\n";
    
    if (m_loglevel >= 2)
        std::cout << "Building all Union boolean shapes...\n";
    nSize = m_shapes_Union.size();
    m_memMapShapes_Union.reserve(nSize);
    for (const auto &row : m_shapes_Union)
    {
        
        // boolean_shapes_operands_info
        //     shapes_info_sub; // tuple to store the boolean shapes to
                             // complete at a second stage
        buildShapeOperator("Union", row, &shapes_info_sub);
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " Shape-Operators-Union have been built!\n";


    createBooleanShapeOperands(&shapes_info_sub);
}

//! Iterate over the list of GeoBox shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_Box()
{
    if (m_loglevel >= 1) {
        std::cout << "Building all shapes -- Box ...\n";
    }

    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_Box.size();
    m_builderShape_Box = new BuildGeoShapes_Box(nSize);

    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_Box)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_Box->buildShape(row);
    }
    // m_builderShape_Box->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-Box have been built!\n";
    }
}
//! Iterate over the list of GeoEllipticalTube shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_EllipticalTube()
{
    if (m_loglevel >= 1) {
        std::cout << "Building all shapes -- EllipticalTube ...\n";
    }

    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_EllipticalTube.size();
    m_builderShape_EllipticalTube = new BuildGeoShapes_EllipticalTube(nSize);

    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_EllipticalTube)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_EllipticalTube->buildShape(row);
    }
    // m_builderShape_EllipticalTube->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-EllipticalTube have been built!\n";
    }
}
//! Iterate over the list of GeoTube shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_Tube()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_Tube.size();
    m_builderShape_Tube = new BuildGeoShapes_Tube(nSize);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_Tube)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_Tube->buildShape(row);
    }
    // m_builderShape_Tube->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-Tube have been built!\n";
    }
}

//! Iterate over the list of GeoCons shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_Cons()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_Cons.size();
    m_builderShape_Cons = new BuildGeoShapes_Cons(nSize);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_Cons)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_Cons->buildShape(row);
    }
    // m_builderShape_Cons->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-Cons have been built!\n";
    }
}
//! Iterate over the list of GeoPara shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_Para()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_Para.size();
    m_builderShape_Para = new BuildGeoShapes_Para(nSize);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_Para)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_Para->buildShape(row);
    }
    // m_builderShape_Para->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-Para have been built!\n";
    }
}
//! Iterate over the list of GeoTrap shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_Trap()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_Trap.size();
    m_builderShape_Trap = new BuildGeoShapes_Trap(nSize);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_Trap)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_Trap->buildShape(row);
    }
    // m_builderShape_Trap->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-Trap have been built!\n";
    }
}
//! Iterate over the list of GeoTrd shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_Trd()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_Trd.size();
    m_builderShape_Trd = new BuildGeoShapes_Trd(nSize);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_Trd)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_Trd->buildShape(row);
    }
    // m_builderShape_Trd->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-Trd have been built!\n";
    }
}
//! Iterate over the list of GeoTubs shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_Tubs()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_Tubs.size();
    m_builderShape_Tubs = new BuildGeoShapes_Tubs(nSize);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_Tubs)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_Tubs->buildShape(row);
    }
    // m_builderShape_Tubs->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-Tubs have been built!\n";
    }
}
//! Iterate over the list of GeoTorus shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_Torus()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_Torus.size();
    m_builderShape_Torus = new BuildGeoShapes_Torus(nSize);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_Torus)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_Torus->buildShape(row);
    }
    // m_builderShape_Torus->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-Torus have been built!\n";
    }
}
//! Iterate over the list of GeoTwistedTrap shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_TwistedTrap()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_TwistedTrap.size();
    m_builderShape_TwistedTrap = new BuildGeoShapes_TwistedTrap(nSize);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_TwistedTrap)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_TwistedTrap->buildShape(row);
    }
    // m_builderShape_TwistedTrap->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-TwistedTrap have been built!\n";
    }
}
//! Iterate over the list of GeoPcon shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_Pcon()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_Pcon.size();
    m_builderShape_Pcon = new BuildGeoShapes_Pcon(nSize, m_shapes_Pcon_data);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_Pcon)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_Pcon->buildShape(row);
    }
    // m_builderShape_Pcon->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-Pcon have been built!\n";
    }
}
//! Iterate over the list of GeoPgon shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_Pgon()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_Pgon.size();
    m_builderShape_Pgon = new BuildGeoShapes_Pgon(nSize, m_shapes_Pgon_data);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_Pgon)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_Pgon->buildShape(row);
    }
    // m_builderShape_Pgon->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-Pgon have been built!\n";
    }
}
//! Iterate over the list of GeoSimplePolygonBrep shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_SimplePolygonBrep()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_SimplePolygonBrep.size();
    m_builderShape_SimplePolygonBrep = new BuildGeoShapes_SimplePolygonBrep(nSize, m_shapes_SimplePolygonBrep_data);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_SimplePolygonBrep)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_SimplePolygonBrep->buildShape(row);
    }
    // m_builderShape_SimplePolygonBrep->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-SimplePolygonBrep have been built!\n";
    }
}

//! Iterate over the list of GeoGenericTrap shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_GenericTrap()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_GenericTrap.size();
    m_builderShape_GenericTrap = new BuildGeoShapes_GenericTrap(nSize, m_shapes_GenericTrap_data);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_GenericTrap)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_GenericTrap->buildShape(row);
    }
    // m_builderShape_GenericTrap->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-GenericTrap have been built!\n";
    }
}

//! Iterate over the list of GeoTube shape nodes, build them all, 
//! and store their pointers
void ReadGeoModel::buildAllShapes_UnidentifiedShape()
{
    // create a builder and reserve size of memory map
    size_t nSize = m_shapes_UnidentifiedShape.size();
    m_builderShape_UnidentifiedShape = new BuildGeoShapes_UnidentifiedShape(nSize);
    // loop over the DB rows and build the shapes
    for (const auto &row : m_shapes_UnidentifiedShape)
    {
        // GeoModelIO::CppHelper::printStdVectorVariants(row); // DEBUG MSG
        m_builderShape_UnidentifiedShape->buildShape(row);
    }
    // m_builderShape_UnidentifiedShape->printBuiltShapes(); // DEBUG MSG
    if (nSize > 0) {
        std::cout << "All " << nSize << " Shapes-UnidentifiedShape have been built!\n";
    }
}

//! Iterate over the list of GeoSerialDenominator nodes, build them all, and
//! store their pointers
void ReadGeoModel::buildAllSerialDenominators() {
    if (m_loglevel >= 1)
        std::cout << "Building all SerialDenominator nodes...\n";
    size_t nSize = m_serialDenominators.size();
    m_memMapSerialDenominators.reserve(
        nSize * 2);  // TODO: check if *2 is good or redundant...
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        // const unsigned int nodeID =
        // std::stoi(m_serialDenominators[ii][0]);
        // // RMB: not used at the moment, commented to avoid warnings
        const std::string baseName = m_serialDenominators[ii][1];
        GeoSerialDenominator* nodePtr = new GeoSerialDenominator(baseName);
        storeBuiltSerialDenominator(nodePtr);
    }
    if (nSize > 0)
        std::cout << "All " << nSize
                  << " SerialDenominators have been built!\n";
}

//! Iterate over the list of GeoSerialIdentifier nodes, build them all, and
//! store their pointers
void ReadGeoModel::buildAllSerialIdentifiers() {
    if (m_loglevel >= 1)
        std::cout << "Building all SerialIdentifier nodes...\n";
    size_t nSize = m_serialIdentifiers.size();
    m_memMapSerialIdentifiers.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        // const unsigned int nodeID = std::stoi(m_seriaIdentifiers[ii][0]);
        // // RMB: not used at the moment, commented to avoid warnings
        const int baseId = std::stoi(m_serialIdentifiers[ii][1]);
        GeoSerialIdentifier* nodePtr = new GeoSerialIdentifier(baseId);
        storeBuiltSerialIdentifier(nodePtr);
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " SerialIdentifiers have been built!\n";
}

//! Iterate over the list of GeoIdentifierTag nodes, build them all, and
//! store their pointers
void ReadGeoModel::buildAllIdentifierTags() {
    if (m_loglevel >= 1) std::cout << "Building all IdentifierTag nodes...\n";
    size_t nSize = m_identifierTags.size();
    m_memMapIdentifierTags.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        // const unsigned int nodeID = std::stoi(m_identifierTags[ii][0]);
        // // RMB: not used at the moment, commented to avoid warnings
        const int identifier = std::stoi(m_identifierTags[ii][1]);
        GeoIdentifierTag* nodePtr = new GeoIdentifierTag(identifier);
        storeBuiltIdentifierTag(nodePtr);
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " SerialIdentifiers have been built!\n";
}

//! Iterate over the list of NameTag nodes, build them all, and store their
//! pointers
void ReadGeoModel::buildAllNameTags() {
    if (m_loglevel >= 1) std::cout << "Building all NameTag nodes...\n";
    size_t nSize = m_nameTags.size();
    m_memMapNameTags.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        // const unsigned int nodeID = std::stoi(m_nameTags[ii][0]); // RMB:
        // not used at teh moment, commented to avoid warnings
        const std::string baseName = m_nameTags[ii][1];
        GeoNameTag* nodePtr = new GeoNameTag(baseName);
        storeBuiltNameTag(nodePtr);
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " NameTags have been built!\n";
}

//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllElements() {
    if (m_loglevel >= 1) std::cout << "Building all Elements...\n";
    size_t nSize = m_elements.size();
    m_memMapElements.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        // const unsigned int nodeID = std::stoi(m_elements[ii][0]);
        const unsigned nodeID = GeoModelHelpers::variantHelper::getFromVariant_Int(m_elements[ii][0], "Element:ID");
        buildElement(nodeID);  // nodes' IDs start from 1
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " Elements have been built!\n";
}

//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllMaterials() {
    if (m_loglevel >= 1) std::cout << "Building all Materials...\n";
    size_t nSize = m_materials.size();
    m_memMapMaterials.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        const unsigned nodeID = GeoModelHelpers::variantHelper::getFromVariant_Int(m_materials[ii][0], "MaterialElement:ID");
        buildMaterial(nodeID);  // nodes' IDs start from 1
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " Materials have been built!\n";
}

//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllVSurfaces()
{
    if (m_loglevel >= 1)
        std::cout << "Building all Virtual Surfaces...\n";
    size_t nSize = m_VSurface.size();
    BuildGeoVSurface* buildsurfTool = new BuildGeoVSurface();  // TODO: change the name of this class
    for (unsigned int ii = 0; ii < nSize; ++ii)
    {
        try
        {
            const unsigned int nodeID = std::get<int>(m_VSurface[ii][0]);
            buildVSurface(nodeID);
        }
        catch (std::bad_variant_access const &ex)
        {
            std::cout << ex.what() << ": Virtual Surface 'ID' is not an 'int' value!\n";
        }
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " Virtual Surface have been built!\n";
}

//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllLogVols()
{
    if (m_loglevel >= 1)
        std::cout << "Building all LogVols...\n";
    size_t nSize = m_logVols.size();
    m_memMapLogVols.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii)
    {
        try
        {
            const unsigned int nodeID = std::get<int>(m_logVols[ii][0]);
            buildLogVol(nodeID);
        }
        catch (std::bad_variant_access const &ex)
        {
            std::cout << ex.what() << ": logVol 'ID' is not an 'int' value!\n";
        }
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " LogVols have been built!\n";
}

//// FIXME: TODO: move to an utility class
// void printQHashQstringUInt(QHash<QString, unsigned int> qq) {
//   QHash<QString, unsigned int>::const_iterator i = qq.constBegin();
//   while (i != qq.constEnd()) {
//     std::cout << i.key().toStdString() << ": " << i.value() << std::endl;
//   }
// }

//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllPhysVols() {
    if (m_debug) std::cout << "Building all PhysVols...\n";
    if (m_physVols.size() == 0) {
        std::cout << "ERROR!!! No input PhysVols found! Exiting..."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    const unsigned int tableID = m_tableName_toTableID["GeoPhysVol"];
    size_t nSize = m_physVols.size();
    m_memMapPhysVols.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        const unsigned int volID = std::stoi(m_physVols[ii][0]);
        const unsigned int logVolID = std::stoi(m_physVols[ii][1]);
        // std::cout << "building PhysVol n. " << volID << " (logVol: " <<
        // logVolID << ")" << std::endl;
        buildVPhysVol(volID, tableID, logVolID);
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " PhysVols have been built!\n";
}

//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllFullPhysVols() {
    if (m_debug) std::cout << "Building all FullPhysVols...\n";
    const unsigned int tableID = m_tableName_toTableID["GeoFullPhysVol"];
    size_t nSize = m_fullPhysVols.size();
    m_memMapFullPhysVols.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        const unsigned int volID = std::stoi(m_fullPhysVols[ii][0]);
        const unsigned int logVolID = std::stoi(m_fullPhysVols[ii][1]);
        // std::cout << "building PhysVol n. " << volID << " (logVol: " <<
        // logVolID << ")" << std::endl;
        buildVPhysVol(volID, tableID, logVolID);
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " FullPhysVols have been built!\n";
}

//! Iterate over the list of GeoAlignableTransforms nodes, build them all, and
//! store their pointers
void ReadGeoModel::buildAllAlignableTransforms() {
    if (m_debug) std::cout << "Building all AlignableTransforms...\n";
    size_t nSize = m_alignableTransforms.size();
    m_memMapAlignableTransforms.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        const unsigned int volID = std::stoi(m_alignableTransforms[ii][0]);
        buildAlignableTransform(volID);
    }
    if (nSize > 0)
        std::cout << "All " << nSize
                  << " AlignableTransforms have been built!\n";
}

//! Iterate over the list of GeoTransforms nodes, build them all, and store
//! their pointers
void ReadGeoModel::buildAllTransforms() {
    if (m_debug) std::cout << "Building all Transforms...\n";
    size_t nSize = m_transforms.size();
    m_memMapTransforms.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        const unsigned int volID = std::stoi(m_transforms[ii][0]);
        buildTransform(volID);
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " Transforms have been built!\n";
}

//! Iterate over the list of GeoTransforms nodes, build them all, and store
//! their pointers
void ReadGeoModel::buildAllSerialTransformers() {
    if (m_debug) std::cout << "Building all SerialTransformers...\n";
    size_t nSize = m_serialTransformers.size();
    m_memMapSerialTransformers.reserve(nSize);
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        const unsigned int volID = std::stoi(m_serialTransformers[ii][0]);
        buildSerialTransformer(volID);
    }
    if (nSize > 0)
        std::cout << "All " << nSize
                  << " SerialTransformers have been built!\n";
}

// FIXME: implement build function and cache for Functions
// //! Iterate over the list of nodes, build them all, and store their
// pointers
/*
void ReadGeoModel::buildAllFunctions() {
    if (m_loglevel >= 1) std::cout << "Building all Functions...\n";
    ////   if (m_serialTransformers.size() == 0) {
    ////     std::cout << "ERROR!!! input SerialTransformers are empty!
    /// Exiting..."
    ///<< std::endl; /     exit(EXIT_FAILURE); /   }
    size_t nSize = m_functions.size();
    m_memMapFunctions.reserve(nSize *
                              2);  // TODO:  check if 2 is good or redundant...
    for (unsigned int ii = 0; ii < nSize; ++ii) {
        buildFunction(ii + 1);  // nodes' IDs start from 1
    }
    if (nSize > 0)
        std::cout << "All " << nSize << " Functions have been built!\n";
}
*/

void ReadGeoModel::loopOverAllChildrenInBunches_VecVecData() {
    int nChildrenRecords = m_allchildren.size();
    if (m_loglevel >= 1)
        std::cout << "number of children to process: " << nChildrenRecords
                  << std::endl;

    // If we have a few children, then process them serially
    // std::cout << "Running concurrently? " << m_runMultithreaded <<
    // std::endl;
    if (true)  // !(m_runMultithreaded) || nChildrenRecords <= 500) // TODO:
               // test if you can optimize, then revert to if()...else()
    {
        // std::cout << "Running serially...\n";
        loopOverAllChildrenRecords(m_allchildren);
    }
    // ...otherwise, let's spawn some threads to process them in bunches,
    // parallelly!
    else {
        // std::cout << "Running concurrently...\n";

        std::chrono::system_clock::time_point start, end;
        if (m_timing || (m_loglevel >= 1)) {
            // Get Start Time
            start = std::chrono::system_clock::now();
        }

        // set number of worker threads
        unsigned int nThreads = 0;
        if (m_runMultithreaded_nThreads > 0) {
            nThreads = m_runMultithreaded_nThreads;
        } else if (m_runMultithreaded_nThreads == -1) {
            unsigned int nThreadsPlatform = std::thread::hardware_concurrency();
            nThreads = nThreadsPlatform;
            if (m_loglevel >= 1)
                std::cout << "INFO - You have asked for hardware native "
                             "parellelism. On this platform, "
                          << nThreadsPlatform
                          << " concurrent threads are supported. Thus, using "
                          << nThreads << " threads.\n";
        }

        unsigned int nBunches = nChildrenRecords / nThreads;
        if (m_loglevel >= 1)
            std::cout << "Processing " << nThreads << " bunches, with "
                      << nBunches << " children each, plus the remainder."
                      << std::endl;

        // a vector to store the "futures" of async calls
        std::vector<std::future<void>> futures;

        for (unsigned int bb = 0; bb < nThreads; ++bb) {
            DBRowsList bunch;

            unsigned int start = nBunches * bb;
            int len = nBunches;
            const unsigned int stop = start + len;
            DBRowsList::const_iterator first =
                m_allchildren.begin() + start;
            DBRowsList::const_iterator last =
                m_allchildren.begin() + stop;
            DBRowsList::const_iterator end =
                m_allchildren.end();
            if (bb == (nThreads - 1)) {  // last bunch
                bunch = DBRowsList(first, end);
            } else {  // all bunches but last one
                bunch = DBRowsList(first, last);
            }

            if (m_loglevel >= 1) {
                muxCout.lock();
                std::cout << "Thread " << bb + 1 << " - Start: " << start
                          << ", len: " << len
                          << "   ['len=-1' = all remaining items]" << std::endl;
                muxCout.unlock();
            }

            if (m_loglevel >= 1) {
                muxCout.lock();
                std::cout << "'bunch' size: " << bunch.size() << std::endl;
                muxCout.unlock();
            }

            futures.push_back(std::async(
                std::launch::async, &ReadGeoModel::loopOverAllChildrenRecords,
                this, bunch));
        }

        // wait for all async calls to complete
        // retrieve and print the value stored in the 'std::future'
        if (m_loglevel >= 1)
            std::cout << "Waiting for the threads to finish...\n" << std::flush;
        for (auto& e : futures) {
            e.wait();
        }
        if (m_loglevel >= 1) std::cout << "Done!\n";

        if (m_timing || (m_loglevel >= 1)) {
            // Get End Time
            end = std::chrono::system_clock::now();
            auto diff =
                std::chrono::duration_cast<std::chrono::seconds>(end - start)
                    .count();
            std::cout << "(Total time taken to recreate all "
                      << nChildrenRecords
                      << " mother-children relationships: " << diff
                      << " seconds)" << std::endl;
        }
    }
    return;
}

void ReadGeoModel::processParentChild(
    const std::vector<std::string>& parentchild) {
    if (m_loglevel >= 2) {
        muxCout.lock();
        std::cout << "\nReadGeoModel::processParentChild()..." << std::endl;
        for (const auto& rec : parentchild) std::cout << rec << "-";
        std::cout << std::endl;
        muxCout.unlock();
    }

    // safety check
    if (parentchild.size() < 8) {
        std::cout << "ERROR!!! Probably you are using an old geometry file. "
                     "Please, get a new one. Exiting..."
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    // get the parent's details
    const unsigned int parentId = std::stoi(parentchild[1]);
    const unsigned int parentTableId = std::stoi(parentchild[2]);
    const unsigned int parentCopyN = std::stoi(parentchild[3]);

    // get the child's position in the parent's children list
    // const unsigned int position = std::stoi(parentchild[4]); // unused

    // get the child's details
    const unsigned int childTableId = std::stoi(parentchild[5]);
    const unsigned int childId = std::stoi(parentchild[6]);
    const unsigned int childCopyN = std::stoi(parentchild[7]);

    //    std::string childNodeType =
    //    m_tableID_toTableName[childTableId].toStdString();
    std::string childNodeType = m_tableID_toTableName[childTableId];

    if ("" == childNodeType || 0 == childNodeType.size()) {
        std::cout << "ReadGeoModel -- ERROR!!! childNodeType is empty!!! "
                     "Aborting..."
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    GeoVPhysVol* parentVol = nullptr;

    // build or get parent volume.
    // Using the parentCopyNumber here, to get a given instance of the
    // parent volume
    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "build/get parent volume...\n";
        muxCout.unlock();
    }
    parentVol = dynamic_cast<GeoVPhysVol*>(
        buildVPhysVolInstance(parentId, parentTableId, parentCopyN));
    std::string parentName = parentVol->getLogVol()->getName();

    if (childNodeType == "GeoPhysVol") {
        GeoPhysVol* childNode = dynamic_cast<GeoPhysVol*>(
            buildVPhysVolInstance(childId, childTableId, childCopyN));
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoFullPhysVol") {
        GeoFullPhysVol* childNode = dynamic_cast<GeoFullPhysVol*>(
            buildVPhysVolInstance(childId, childTableId, childCopyN));
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoVSurface") {
        GeoVSurface* childNode = m_memMapVSurface[childId-1];
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoSerialDenominator") {
        GeoSerialDenominator* childNode = getBuiltSerialDenominator(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoSerialIdentifier") {
        GeoSerialIdentifier* childNode = getBuiltSerialIdentifier(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoIdentifierTag") {
        GeoIdentifierTag* childNode = getBuiltIdentifierTag(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoAlignableTransform") {
        GeoAlignableTransform* childNode = getBuiltAlignableTransform(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoTransform") {
        if (m_loglevel >= 2) {
            muxCout.lock();
            std::cout << "get transform child...\n";
            muxCout.unlock();
        }
        GeoTransform* childNode = getBuiltTransform(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoSerialTransformer") {
        GeoSerialTransformer* childNode = getBuiltSerialTransformer(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoNameTag") {
        GeoNameTag* childNode = getBuiltNameTag(childId);
        volAddHelper(parentVol, childNode);
    } else {
        std::cout << "[" << childNodeType
                  << "] ==> ERROR!!! - The conversion for this type of child "
                     "node needs to be implemented."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
}
void ReadGeoModel::processParentChild(
    const DBRowEntry& parentchild) {
    
    // if (m_loglevel >= 2) {
    //     muxCout.lock();
    //     std::cout << "\nReadGeoModel::processParentChild()..." << std::endl;
    //     for (auto& rec : parentchild) std::cout << rec << "-";
    //     std::cout << std::endl;
    //     muxCout.unlock();
    // }
    // safety check
    if (parentchild.size() < 8) {
        std::cout << "ERROR!!! Probably you are using an old geometry file. "
                     "Please, get a new one. Exiting..."
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    // get the parent's details
    const unsigned int parentId = GeoModelHelpers::variantHelper::getFromVariant_Int(parentchild[1], "ParentChild:parentID");
    const unsigned int parentTableId = GeoModelHelpers::variantHelper::getFromVariant_Int(parentchild[2], "ParentChild:parentID"); 
    const unsigned int parentCopyN = GeoModelHelpers::variantHelper::getFromVariant_Int(parentchild[3], "ParentChild:parentID"); 

    // get the child's position in the parent's children list
    // const unsigned int position = parentchild[4]; // unused, at the moment

    // get the child's details
    const unsigned int childTableId = GeoModelHelpers::variantHelper::getFromVariant_Int(parentchild[5], "ParentChild:parentID"); 
    const unsigned int childId = GeoModelHelpers::variantHelper::getFromVariant_Int(parentchild[6], "ParentChild:parentID"); 
    const unsigned int childCopyN = GeoModelHelpers::variantHelper::getFromVariant_Int(parentchild[7], "ParentChild:parentID"); 

    //    std::string childNodeType =
    //    m_tableID_toTableName[childTableId].toStdString();
    std::string childNodeType = m_tableID_toTableName[childTableId];

    if ("" == childNodeType || 0 == childNodeType.size()) {
        std::cout << "ReadGeoModel -- ERROR!!! childNodeType is empty!!! "
                     "Aborting..."
                  << std::endl;
        exit(EXIT_FAILURE);
    }

    GeoVPhysVol* parentVol = nullptr;

    // build or get parent volume.
    // Using the parentCopyNumber here, to get a given instance of the
    // parent volume
    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "build/get parent volume...\n";
        muxCout.unlock();
    }
    parentVol = dynamic_cast<GeoVPhysVol*>(
        buildVPhysVolInstance(parentId, parentTableId, parentCopyN));
    std::string parentName = parentVol->getLogVol()->getName();

    if (childNodeType == "GeoPhysVol") {
        GeoPhysVol* childNode = dynamic_cast<GeoPhysVol*>(
            buildVPhysVolInstance(childId, childTableId, childCopyN));
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoFullPhysVol") {
        GeoFullPhysVol* childNode = dynamic_cast<GeoFullPhysVol*>(
            buildVPhysVolInstance(childId, childTableId, childCopyN));
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoVSurface") {
        GeoVSurface* childNode = m_memMapVSurface[childId-1];
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoSerialDenominator") {
        GeoSerialDenominator* childNode = getBuiltSerialDenominator(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoSerialIdentifier") {
        GeoSerialIdentifier* childNode = getBuiltSerialIdentifier(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoIdentifierTag") {
        GeoIdentifierTag* childNode = getBuiltIdentifierTag(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoAlignableTransform") {
        GeoAlignableTransform* childNode = getBuiltAlignableTransform(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoTransform") {
        if (m_loglevel >= 2) {
            muxCout.lock();
            std::cout << "get transform child...\n";
            muxCout.unlock();
        }
        GeoTransform* childNode = getBuiltTransform(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoSerialTransformer") {
        GeoSerialTransformer* childNode = getBuiltSerialTransformer(childId);
        volAddHelper(parentVol, childNode);
    } else if (childNodeType == "GeoNameTag") {
        GeoNameTag* childNode = getBuiltNameTag(childId);
        volAddHelper(parentVol, childNode);
    } else {
        std::cout << "[" << childNodeType
                  << "] ==> ERROR!!! - The conversion for this type of child "
                     "node needs to be implemented."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ReadGeoModel::volAddHelper(GeoVPhysVol* vol, GeoGraphNode* volChild) {
    checkNodePtr(vol, "vol", __func__, __PRETTY_FUNCTION__);
    checkNodePtr(volChild, "volChild", __func__, __PRETTY_FUNCTION__);
    if (dynamic_cast<GeoPhysVol*>(vol)) {
        GeoPhysVol* volume = dynamic_cast<GeoPhysVol*>(vol);
        volume->add(volChild);
    } else if (dynamic_cast<GeoFullPhysVol*>(vol)) {
        GeoFullPhysVol* volume = dynamic_cast<GeoFullPhysVol*>(vol);
        volume->add(volChild);
    }
}

// TODO: to be moved to an utility class
void ReadGeoModel::checkNodePtr(GeoGraphNode* nodePtr, const std::string& varName,
                                const std::string& funcName,
                                const std::string& funcSignature) {
    if (!nodePtr) {
        muxCout.lock();
        std::cout << "ERROR! A pointer to a GeoGraphNode instance is NULL {"
                  << varName << " " << funcName << " "
                  << "[" << funcSignature << "]}. Exiting..." << std::endl;
        exit(EXIT_FAILURE);
        muxCout.unlock();
    }
}

// TODO: move it to an utility class
// void ReadGeoModel::checkInputString(QString input)
//{
//   if (input.isEmpty() || input.isNull() || input == "NULL") {
//     std::cout << "ERROR!!! Input QString is empty or equal to 'NULL'!!!
//     Aborting..." << std::endl; exit(1);
//   }
// }

// Instantiate a PhysVol and get its children
GeoVPhysVol* ReadGeoModel::buildVPhysVolInstance(const unsigned int id,
                                                 const unsigned int tableId,
                                                 const unsigned int copyN) {
    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "ReadGeoModel::buildVPhysVolInstance() - id: " << id
                  << ", tableId: " << tableId << ", copyN: " << copyN
                  << std::endl;
        muxCout.unlock();
    }

    // A - if the instance has been previously built, return that
    // if ( nullptr != getVPhysVol(id, tableId, copyN)) {
    if (nullptr != getVPhysVol(id, tableId)) {
        if (m_loglevel >= 3) {
            muxCout.lock();
            // std::cout << "getting the instance volume from memory...
            // Returning: [" << getVPhysVol(id, tableId, copyN) << "] --
            // logvol: " << ((GeoVPhysVol*)getVPhysVol(id, tableId,
            // copyN))->getLogVol()->getName() << std::endl;
	    const GeoPhysVol *pv=dynamic_cast<const GeoPhysVol *> (getVPhysVol(id,tableId));
	    if (pv) {
	      std::cout
                << "getting the instance volume from memory... Returning: ["
                << getVPhysVol(id, tableId) << "] -- logvol: "
                << pv
		->getLogVol()
		->getName()
                << std::endl;
	      muxCout.unlock();
	    }
	    else {
	    }
        }
        // return dynamic_cast<GeoVPhysVol*>(getVPhysVol(id, tableId,
        // copyN));
        return dynamic_cast<GeoVPhysVol*>(getVPhysVol(id, tableId));
    }

    // B - if not built already, then get the actual volume,
    // which should be already built by now,
    // get the logVol from it and build a new VPhysVol instance in the heap;
    // then, associate a copy number to it,
    // and store the new instance into the cache.
    GeoVPhysVol* vol = nullptr;
    bool volFound = true;
    if (1 == tableId) {
        if (isBuiltPhysVol(id)) {
            vol = new GeoPhysVol(getBuiltPhysVol(id)->getLogVol());
            if (m_loglevel >= 3) {
                muxCout.lock();
                std::cout
                    << "PhysVol not instanced yet, building the instance now ["
                    << vol << "] -- logvol: " << vol->getLogVol()->getName()
                    << std::endl;
                muxCout.unlock();
            }
        } else
            volFound = false;
    } else if (2 == tableId) {
        if (isBuiltFullPhysVol(id))
            vol = new GeoFullPhysVol(getBuiltFullPhysVol(id)->getLogVol());
        else
            volFound = false;
    } else {
        std::cerr << "ERROR!! It seems that the 'world' volume is of type other than GeoPhysVol or GeoFullPhysVol, which is forbidden! If you have questions, please send a message to <geomodel-developers@cern.ch>" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (!volFound) {
        std::cout << "ERROR! VPhysVol not found! It should be already built, "
                     "by now. Exiting...\n";
        exit(EXIT_FAILURE);
    }
    // storeVPhysVol(id, tableId, copyN, vol);
    storeVPhysVol(id, tableId, vol);

    return vol;
}

//! Build the actual VPhysVol (GeoPhysVol or GeoFullPhysVol), which will then be
//! used to create instances of that volume by using the `copyNumber`. Here,
//! however, we do not need to specify any copyNumber, because the actual
//! GeoPVPhysVol is the same for all copy instances.
GeoVPhysVol* ReadGeoModel::buildVPhysVol(
    const unsigned int id, const unsigned int tableId,
    unsigned int /*defaults to "0"*/ logVol_ID) {
    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "ReadGeoModel::buildVPhysVol() - " << id << ", " << tableId
                  << std::endl;
        muxCout.unlock();
    }

    std::string nodeType = m_tableID_toTableName[tableId];

    bool errorType = false;

    // get the actual VPhysVol volume, if built already
    if (nodeType == "GeoPhysVol" && isBuiltPhysVol(id)) {
        if (m_loglevel >= 3) {
            muxCout.lock();
            std::cout << "getting the actual PhysVol from cache...\n";
            ;
            muxCout.unlock();
        }
        return getBuiltPhysVol(id);
    } else if (nodeType == "GeoFullPhysVol" && isBuiltFullPhysVol(id)) {
        if (m_loglevel >= 3) {
            muxCout.lock();
            std::cout << "getting the actual FullPhysVol from cache...\n";
            ;
            muxCout.unlock();
        }
        return getBuiltFullPhysVol(id);
    }

    // if not built already, then get its parameters and build it now
    if (logVol_ID == 0) {
        // get the volume's parameters
        std::vector<std::string> values;
        if (nodeType == "GeoPhysVol")
            values = m_physVols[id];
        else if (nodeType == "GeoFullPhysVol")
            values = m_fullPhysVols[id];

        logVol_ID = std::stoi(values[1]);
    }

    // GET LOGVOL
    GeoLogVol* logVol = getBuiltLog(logVol_ID);
    if (!logVol) {
        std::cout << "ERROR!!! LogVol is NULL!" << std::endl;
        //    exit(EXIT_FAILURE);
    }
    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "using the cached LogVol [" << logVol_ID
                  << "] w/ address: " << logVol << "...\n";
        ;
        muxCout.unlock();
    }

    // a pointer to the VPhysVol
    GeoVPhysVol* vol = nullptr;

    // BUILD THE PHYSVOL OR THE FULLPHYSVOL
    if (nodeType == "GeoPhysVol") {
        GeoPhysVol* pVol = new GeoPhysVol(logVol);
        storeBuiltPhysVol(pVol);
        vol = pVol;
    } else if (nodeType == "GeoFullPhysVol") {
        GeoFullPhysVol* fpVol = new GeoFullPhysVol(logVol);
        storeBuiltFullPhysVol(fpVol);
        vol = fpVol;
    } else
        errorType = true;

    if (errorType) {
        muxCout.lock();
        std::cout << "ERROR! Unkonwn node type. Exiting..." << std::endl;
        muxCout.unlock();
        exit(EXIT_FAILURE);
    }

    return vol;
}

// Get the root volume
GeoVPhysVol* ReadGeoModel::getRootVolume() {
    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "ReadGeoModel::getRootVolume()" << std::endl;
        std::cout << "m_root_vol_data: " << m_root_vol_data.first << ", " << m_root_vol_data.second << std::endl;       
        muxCout.unlock();
    }
    const unsigned tableId = m_root_vol_data.first;
    const unsigned id = m_root_vol_data.second;
    // const unsigned int tableId = m_dbManager->getTableIdFromNodeType(tableName);
    const unsigned int copyNumber =
        1;  // the Root volume has only one copy by definition
    GeoVPhysVol* root = buildVPhysVolInstance(id, tableId, copyNumber);
    checkNodePtr(root, "root", __func__, __PRETTY_FUNCTION__);
    return root;
}

GeoMaterial* ReadGeoModel::buildMaterial(const unsigned id) {
    if (isBuiltMaterial(id)) {
        return getBuiltMaterial(id);
    }

    if (m_loglevel >= 2) {
        muxCout.lock();
        std::cout << "ReadGeoModel::buildMaterial()" << std::endl;
        muxCout.unlock();
    }

    DBRowEntry values = m_materials[id - 1];
    const unsigned int matId = GeoModelHelpers::variantHelper::getFromVariant_Int(values[0], "Material:id");
    const std::string matName = GeoModelHelpers::variantHelper::getFromVariant_String(values[1], "Material:matName");
    const double matDensity = GeoModelHelpers::variantHelper::getFromVariant_Double(values[2], "Material:matDensity");
    const unsigned dataStart = GeoModelHelpers::variantHelper::getFromVariant_Int(values[3], "Material:dataStart");
    const unsigned dataEnd = GeoModelHelpers::variantHelper::getFromVariant_Int(values[4], "Material:dataEnd");

    // debug msg
    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "\tbuildMaterial() : Material - ID:" << matId << ", name:" << matName
                  << ", density:" << matDensity << " ( "
                  << matDensity / (SYSTEM_OF_UNITS::g / SYSTEM_OF_UNITS::cm3)
                  << "[g/cm3] )"
                  << ", elements: dataStart: " << dataStart << ", dataEnd: " << dataEnd;
        muxCout.unlock();
    }

    GeoMaterial* mat = new GeoMaterial(matName, matDensity);

    DBRowsList matElements(m_materials_Data.begin() + (dataStart-1),
                           m_materials_Data.begin() + (dataEnd) );

    if (matElements.size() > 0) {
        // get parameters from DB

        // const std::vector<std::string> elements = splitString(matElements, ';');
        for (const auto& row : matElements) {
        //     if (m_loglevel >= 2) {
        //         muxCout.lock();
        //         std::cout << "par: " << par << std::endl;
        //         muxCout.unlock();
        //     }
        //     std::vector<std::string> vars = splitString(par, ':');
            const unsigned elId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[1], "MatElement:id");
            double elFraction = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "MatElement:fraction");
            //      GeoElement* el = buildElement(elId);
            GeoElement* el = getBuiltElement(elId);
            mat->add(el, elFraction);
        }
        mat->lock();
    } 
    storeBuiltMaterial(mat);
    return mat;
}

GeoElement* ReadGeoModel::buildElement(const unsigned int id) {
    if (isBuiltElement(id)) {
        return getBuiltElement(id);
    }

    if (m_loglevel >= 2) {
        muxCout.lock();
        std::cout << "ReadGeoModel::buildElement()" << std::endl;
        muxCout.unlock();
    }

    // if (m_elements.size() == 0)
    //     std::cout << "ERROR! 'm_elements' is empty! Did you load the "
    //                  "'Elements' table? \n\t ==> Aborting...\n"
    //               << std::endl;

    // std::vector<std::string> values = m_elements[id - 1];
    DBRowEntry values = m_elements[id - 1];

    const unsigned elId = GeoModelHelpers::variantHelper::getFromVariant_Int(values[0], "Element:ID"); //values[0];
    std::string elName = GeoModelHelpers::variantHelper::getFromVariant_String(values[1], "Element:name"); // values[1];
    std::string elSymbol = GeoModelHelpers::variantHelper::getFromVariant_String(values[2], "Element:symbol"); //values[2];
    double elZ = GeoModelHelpers::variantHelper::getFromVariant_Double(values[3], "Element:Z"); //std::stod(values[3]);
    double elA = GeoModelHelpers::variantHelper::getFromVariant_Double(values[4], "Element:A"); //std::stod(values[4]);

    if (m_loglevel >= 2) {
        muxCout.lock();
        std::cout << "\tElement - ID:" << elId << ", name:" << elName
                  << ", symbol:" << elSymbol << ", Z:" << elZ << ", A:" << elA
                  << " ( " << elA / (SYSTEM_OF_UNITS::g / SYSTEM_OF_UNITS::mole)
                  << "[g/mole] )" << std::endl;
        muxCout.unlock();
    }

    GeoElement* elem = new GeoElement(elName, elSymbol, elZ, elA);
    storeBuiltElement(elem);
    return elem;
}

std::string ReadGeoModel::getShapeType(const unsigned int shapeId) {
    if (shapeId > m_shapes.size()) {
        std::cout
            << "ERROR!! Shape ID is larger than the container size. Exiting..."
            << std::endl;
        exit(EXIT_FAILURE);
    }
    std::vector<std::string> paramsShape =
        m_shapes[shapeId - 1];  // remember: shapes' IDs start from 1
    std::string type = paramsShape[1];
    return type;
}

GeoShape *ReadGeoModel::buildShapeOperator(const std::string_view shapeType, const DBRowEntry& row,
                                            boolean_shapes_operands_info *shapes_info_sub)
{
    GeoShape *shape = nullptr;
    unsigned shapeId{0};

    if ("Shift" == shapeType)
    {
        // metadata
        shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Shift:shapeID");
        // computed volume (not defined by default)
        const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Shift:shapeVolume");
        // shape parameters
        const std::string shapeOpType = GeoModelHelpers::variantHelper::getFromVariant_String(row[2], "Shift:shapeType");
        const unsigned shapeOpId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[3], "Shift:shapeId");
        const unsigned transfId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[4], "Shift:transformId");
        if (shapeOpId == 0 || transfId == 0)
        {
            THROW_EXCEPTION("ERROR!!! Shift shape - input operand shapes' IDs are empty!");
        }

        // if both operands are built already,
        // then get them from cache,
        // and build the operator shape with them,
        if (isBuiltShape(shapeOpType, shapeOpId) && isBuiltTransform(transfId))
        {
            const GeoShape *shapeOp = getBuiltShape(shapeOpId, shapeOpType);
            const GeoTransform *transf = getBuiltTransform(transfId);
            if( nullptr == shapeOp || nullptr == transf) {
                std::cout << "'Shift' debug -- shapeOpType: " << shapeOpType << ", shapeOpId: " << shapeOpId << ", transfId: " << transfId << std::endl;
                THROW_EXCEPTION("ERROR!!! Shape operand or Transformation are NULL!");
            }
            // TODO: here we create a fake GeoTransform to get a
            // GeoTrf::Transform3D.
            // TODO: ==> Perhaps we could keep a table for bare
            // GeoTrf::Transform3D transforms used in GeoShift nodes.
            GeoTrf::Transform3D transfX = transf->getTransform();
            //transf->unref(); this doesnt delete the object, and (if it were to do so)
            // causes problems later
            GeoShapeShift *shapeNew = new GeoShapeShift(shapeOp, transfX);
            // storeBuiltShape(shapeId, shapeNew);
            shape = shapeNew;
        }
        // else {
        //     THROW_EXCEPTION("Operand shapes have not been built!");
        // }
        // otherwise, build the operands
        else
        {
            // TODO: IMPORTANT!!! --> check how the transf used in shift are
            // saved into the DB, because they are bare transf and not
            // GeoTransform nodes...

            // first, check if the transform is built;
            // if so, use that;
            // if not, build the transform

            // get the referenced Transform, then get the bare transform matrix
            // from it
            GeoTransform *transf = nullptr;
            GeoTrf::Transform3D transfX;
            // if (isBuiltTransform(transfId))
            // {
            //     transf = getBuiltTransform(transfId);
            // }
            // else
            // {
                transf = buildTransform(transfId);
            // }
            // TODO: here we create a fake GeoTransform to get a
            // GeoTrf::Transform3D.
            // TODO: ==> Perhaps we could keep a table for bare
            // GeoTrf::Transform3D transforms used in GeoShift nodes.
            transfX = transf->getTransform();
            //transf->unref(); tries to unref when ref count is zero
            // then, check the type of the operand shape
            bool isOperatorShape = isShapeOperator(shapeOpType);

            // if operand shape is simple/actual shape (i.e., not
            // boolean/operator), then build it, 
            // and then build the boolean shape with that
            if (!isOperatorShape)
            {
                const GeoShape *shapeOp =
                    getBuiltShape(shapeOpId, shapeOpType);

                if (shapeOp == nullptr || transf == nullptr)
                {
                    std::cout << "ERROR!!! Shift - shapeOp or transfX are "
                                 "NULL! Exiting..."
                              << std::endl;
                    exit(EXIT_FAILURE);
                }
                // GeoShapeShift *shapeNew = new GeoShapeShift(shapeOp, transfX);
                shape = new GeoShapeShift(shapeOp, transfX);
            }
            // ...otherwise, build the Shift operator shape without operands
            // and save the needed pieces of information to build the actual
            // operands and shape later.
            else
            {
                GeoShapeShift *shapeNew = new GeoShapeShift();
                tuple_boolean_shapes_operands tt(std::string(shapeType), shapeId, shapeNew, shapeOpType, shapeOpId, "NULL",
                                             transfId); // We set opBType to NULL because for Shift we don't need/use it.
                shapes_info_sub->push_back(
                    tt); //! Push the information about the new boolean shape
                         //! at the end of the very same container we are
                         //! iterating over
                shape = shapeNew;
            }
        }


        //! store into the cache the shape we have just built,
        //! for later use when referenced by another node
        // storeBuiltShapeOperators_Shift(shapeId, shape);
    }
    else if ("Subtraction" == shapeType || "Union" == shapeType ||
               "Intersection" == shapeType) {
        // Check what shapes are subtracted/united/intersected:
        // - If they are actual shapes, build them and return
        // - If they are boolean/operator shapes, then store the shape for later
        // and continue

        // metadata
        shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Boolean:shapeID");
        // computed volume (not defined by default)
        const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Boolean:shapeVolume");
        // shape operands
        const std::string shapeOpAType = GeoModelHelpers::variantHelper::getFromVariant_String(row[2], "Boolean:shapeType");
        const unsigned shapeOpAId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[3], "Boolean:shapeId");
        const std::string shapeOpBType = GeoModelHelpers::variantHelper::getFromVariant_String(row[4], "Boolean:shapeType");
        const unsigned shapeOpBId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[5], "Boolean:shapeId");
        if ( ! shapeOpAId || ! shapeOpBId )
        {
            THROW_EXCEPTION("ERROR!!! Shift shape - input operand shapes' IDs are empty!");
        }

        // if both operands are built already,
        // then get them from cache, and build the operator with them
        if (isBuiltShape(shapeOpAType, shapeOpAId) && isBuiltShape(shapeOpBType, shapeOpBId))
        {
            GeoShape *shapeNew = nullptr;
            const GeoShape *shapeA = getBuiltShape(shapeOpAId, shapeOpAType);
            const GeoShape *shapeB = getBuiltShape(shapeOpBId, shapeOpBType);
            if ("Subtraction" == shapeType)
                shape = new GeoShapeSubtraction(shapeA, shapeB);
            else if ("Union" == shapeType)
                shape = new GeoShapeUnion(shapeA, shapeB);
            else if ("Intersection" == shapeType)
                shape = new GeoShapeIntersection(shapeA, shapeB);
        }
        // else {
        //     THROW_EXCEPTION("Operand shapes have not been built!");
        // }
        
        // otherwise, build the operand shapes...
        else {
            // first check the operands' type
            bool isAOperator = isShapeOperator(shapeOpAType);
            bool isBOperator = isShapeOperator(shapeOpBType);

            // if both are simple/actual shapes (i.e., not booleans),
            // build them, then build the boolean shape with them, and
            // store that.
            // TODO: I suspect that this snippet below in the IF clause is dead code,
            // it's never reached.
            if (!isAOperator && !isBOperator) {
                const GeoShape* shapeA = getBuiltShape(shapeOpAId, shapeOpAType);
                const GeoShape* shapeB = getBuiltShape(shapeOpAId, shapeOpBType);
                if (shapeA == NULL || shapeB == NULL)
                {
                    THROW_EXCEPTION("ERROR!!! shapeA or shapeB are NULL!");
                }

                GeoShape* shapeNew = nullptr;
                if ("Subtraction" == shapeType) {
                    shapeNew = new GeoShapeSubtraction(shapeA, shapeB);
                } else if ("Union" == shapeType) {
                    shapeNew = new GeoShapeUnion(shapeA, shapeB);
                } else if ("Intersection" == shapeType) {
                    shapeNew = new GeoShapeIntersection(shapeA, shapeB);
                }

                shape = shapeNew;
            }
            // ...otherwise, build the boolean shape without
            // operands and save the needed pieces of information to build the
            // actual operands and shape later.
            else {
                GeoShape* shapeNew = nullptr;
                if ("Subtraction" == shapeType) {
                    shapeNew = new GeoShapeSubtraction;
                } else if ("Union" == shapeType) {
                    shapeNew = new GeoShapeUnion;
                } else if ("Intersection" == shapeType) {
                    shapeNew = new GeoShapeIntersection;
                }

                tuple_boolean_shapes_operands tt(std::string(shapeType), shapeId, shapeNew, shapeOpAType, shapeOpAId, shapeOpBType,
                                             shapeOpBId); 
                //! Push the information about the new boolean shape
                //! at the end of the very same container we are
                //! iterating over
                shapes_info_sub->push_back(tt); 
                shape = shapeNew;
            }
        }
        
    }
    else {
        THROW_EXCEPTION("ERROR!!! Shape type not defined/handled: '" + std::string(shapeType) + "'!");
    }

    //! store into the cache the shape we have just built,
    //! for later use when referenced by another node
    storeBuiltShape(shapeType, shapeId, shape);
    return shape;
}

// TODO: move shapes in different files, so code here is more managable
/// Recursive function, to build GeoShape nodes
GeoShape* ReadGeoModel::buildShape(const unsigned int shapeId,
                                   type_shapes_boolean_info* shapes_info_sub) {
    if (isBuiltShape(shapeId)) {
        return getBuiltShape(shapeId);
    }

    if (m_loglevel >= 2) {
        muxCout.lock();
        std::cout << "ReadGeoModel::buildShape()" << std::endl;
        muxCout.unlock();
    }

    //   try // TODO: implement try/catch
    //   {
    std::vector<std::string> paramsShape =
        m_shapes[shapeId - 1];  // remember: nodes' IDs start from 1

    //  const unsigned int id = std::stoi(paramsShape[0]); // unused
    std::string type = paramsShape[1];
    std::string parameters = paramsShape[2];

    // Get shape's parameters from the stored string.
    // This will be interpreted differently according to the shape.
    std::vector<std::string> shapePars = splitString(parameters, ';');

    GeoShape* shape = nullptr;

    if (false) {
    } 
    else if (type == "TessellatedSolid") {
        // Tessellated pars example:
        // "nFacets=1;TRI;vT=ABSOLUTE;nV=3;xV=0;yV=0;zV=1;xV=0;yV=1;zV=0;xV=1;yV=0;zV=0"
        std::cout << "Reading-in: TessellatedSolid: ";  // debug
                                                        // Facet type
        std::string facetType = "";
        // shape parameters
        unsigned int nFacets = 0;

        bool error = 0;
        GeoTessellatedSolid* sh = nullptr;

        std::string par;
        std::vector<std::string> vars;
        std::string varName;
        std::string varValue;

        int sizePars = shapePars.size();
        // check if we have at least 13 parameters,
        // which is the minimum for a shape
        // with a single triangular facet
        if (sizePars >= 13) {
            // get the first parameter
            par = shapePars[0];
            vars = splitString(par, '=');
            varName = vars[0];
            varValue = vars[1];
            if (varName == "nFacets")
                nFacets = std::stoi(varValue);
            else {
                muxCout.lock();
                //        qWarning("ERROR!! - GeoTessellatedSolid - nFacets is
                //        not defined!!");
                muxCout.unlock();
                error = true;  // TODO: check "error.h" functionalities and
                               // replace with that, if useful
            }

            // build the basic GeoTessellatedSolid shape
            sh = new GeoTessellatedSolid();

            // and now loop over the rest of the list,
            // to get the parameters of the vertices of each facet
            // and to build the full GeoTessellatedSolid with all the facets
            for (int it = 1; it < sizePars; it++) {
                // get facet type
                par = shapePars[it];
                vars = splitString(par, '=');
                varName = vars[0];
                if (varName == "QUAD") {
                    facetType = "QUAD";
                } else if (varName == "TRI") {
                    facetType = "TRI";
                } else {
                    muxCout.lock();
                    std::cout << "ERROR!! - GeoTessellatedSolid - Facet type "
                                 "is not defined! [got: '"
                              << varName << "']" << std::endl;
                    muxCout.unlock();
                    error = true;
                }

                it++;  // advance to the next parameter

                // get the type of the vertexes composing the facet
                bool isAbsolute = true;
                par = shapePars[it];
                vars = splitString(par, '=');
                varName = vars[0];
                varValue = vars[1];
                if (varName == "vT") {
                    if (varValue == "ABSOLUTE")
                        isAbsolute = true;
                    else if (varValue == "RELATIVE")
                        isAbsolute = false;
                    else {
                        muxCout.lock();
                        std::cout << "ERROR! - GeoTessellatedSolid - Vertex "
                                     "type not defined!"
                                  << std::endl;
                        muxCout.unlock();
                        error = true;
                    }
                } else
                    error = 1;

                it++;  // advance to the next parameter

                /*
                                unsigned int nVertexes = 0; // TODO: check how
          this is used par = shapePars[it]; vars = splitString(par, '=');
                                varName = vars[0];
                                varValue = vars[1];
                                if (varName == "nV") {
                    nVertexes = std::stoi(varValue);
                }
                                else {
          muxCout.lock();
                                        std::cout << "ERROR! -
          GeoTessellatedSolid - nVertices not defined!" << std::endl;
          muxCout.unlock();
                                        error=true;
                                }
                */

                // if we get a QUAD ==> GeoQuadrangularFacet
                if (facetType == "QUAD") {
                    muxCout.lock();
                    std::cout << "Handling a QUAD facet..." << std::endl;
                    muxCout.unlock();
                    // to store the 4 vertices of the GeoQuadrangularFacet
                    auto vV = std::vector<std::unique_ptr<GeoFacetVertex>>{};

                    // we look for 4 vertices for QUAD;
                    // for each of them, we get 3 coordinates
                    //					vertStart = it;
                    for (unsigned int iV = 0; iV < 4; ++iV) {
                        it++;  // advance to the first of the facet's vertices'
                               // coordinates

                        double xV = 0.;
                        par = shapePars[it];
                        vars = splitString(par, '=');
                        varName = vars[0];
                        varValue = vars[1];
                        if (varName == "xV")
                            xV = std::stod(varValue);
                        else {
                            muxCout.lock();
                            std::cout << "ERROR! Got '" << varName
                                      << "' instead of 'xV'!" << std::endl;
                            muxCout.unlock();
                            error = 1;
                        }

                        it++;  // go to the next coordinate

                        double yV = 0.;
                        par = shapePars[it];
                        vars = splitString(par, '=');
                        varName = vars[0];
                        varValue = vars[1];
                        if (varName == "yV")
                            yV = std::stod(varValue);
                        else {
                            muxCout.lock();
                            std::cout << "ERROR! Got '" << varName
                                      << "' instead of 'yV'!" << std::endl;
                            muxCout.unlock();
                            error = 1;
                        }

                        it++;  // go to the next coordinate

                        double zV = 0.;
                        par = shapePars[it];
                        vars = splitString(par, '=');
                        varName = vars[0];
                        varValue = vars[1];
                        if (varName == "zV")
                            zV = std::stod(varValue);
                        else {
                            muxCout.lock();
                            std::cout << "ERROR! Got '" << varName
                                      << "' instead of 'zV'!" << std::endl;
                            muxCout.unlock();
                            error = 1;
                        }

                        if (error) {
                            muxCout.lock();
                            std::cout << "ERROR! GeoTessellatedSolid 'xV', "
                                         "'yV', and 'zV' values are not at the "
                                         "right place! --> ";
                            printStdVectorStrings(shapePars);
                            muxCout.unlock();
                        }

                        // build the facet's vertex and store it
                        vV.push_back(std::make_unique<GeoFacetVertex>(
                            GeoFacetVertex(xV, yV, zV)));
                    }

                    // build the facet and add it to the GeoTessellatedSolid
                    GeoQuadrangularFacet* quadFacet = new GeoQuadrangularFacet(
                        *vV[0], *vV[1], *vV[2], *vV[3],
                        (isAbsolute ? GeoFacet::ABSOLUTE : GeoFacet::RELATIVE));
                    sh->addFacet(quadFacet);
                }
                // if we get a TRI ==> GeoTriangularFacet
                else if (facetType == "TRI") {
                    muxCout.lock();
                    std::cout << "Handling a TRI facet..." << std::endl;
                    muxCout.unlock();

                    // std::vector<GeoFacetVertex*> vV(3, 0); // to store the 3
                    // vertices of the GeoTriangularFacet
                    auto vV = std::vector<std::unique_ptr<GeoFacetVertex>>{};

                    // we look for 3 vertices for GeoTriangularFacet;
                    // for each of them, we get 3 coordinates
                    // vertStart = it;
                    for (unsigned int iV = 0; iV < 3; ++iV) {
                        it++;  // advance to the first of the facet's vertices'
                               // coordinates

                        double xV = 0.;
                        par = shapePars[it];
                        vars = splitString(par, '=');
                        varName = vars[0];
                        varValue = vars[1];
                        if (varName == "xV")
                            xV = std::stod(varValue);
                        else
                            error = 1;

                        it++;  // go to the next coordinate

                        double yV = 0.;
                        par = shapePars[it];
                        vars = splitString(par, '=');
                        varName = vars[0];
                        varValue = vars[1];
                        if (varName == "yV")
                            yV = std::stod(varValue);
                        else
                            error = 1;

                        it++;  // go to the next coordinate

                        double zV = 0.;
                        par = shapePars[it];
                        vars = splitString(par, '=');
                        varName = vars[0];
                        varValue = vars[1];
                        if (varName == "zV")
                            zV = std::stod(varValue);
                        else
                            error = 1;

                        if (error) {
                            muxCout.lock();
                            std::cout << "ERROR! GeoTessellatedSolid 'xV', "
                                         "'yV', and 'zV' values are not at the "
                                         "right place! --> ";
                            printStdVectorStrings(shapePars);
                            muxCout.unlock();
                        }

                        // build the facet's vertex and store it
                        vV.push_back(std::make_unique<GeoFacetVertex>(
                            GeoFacetVertex(xV, yV, zV)));
                    }

                    // build the facet and add it to the GeoTessellatedSolid
                    GeoTriangularFacet* triFacet = new GeoTriangularFacet(
                        *vV[0], *vV[1], *vV[2],
                        (isAbsolute ? GeoFacet::ABSOLUTE : GeoFacet::RELATIVE));
                    sh->addFacet(triFacet);
                }
            }

            // sanity check on the resulting shape
            if (sh->getNumberOfFacets() != nFacets) {
                error = 1;
                muxCout.lock();
                std::cout << "ERROR! GeoTessellatedSolid number of facets: "
                          << sh->getNumberOfFacets()
                          << " is not equal to the original size! --> ";
                printStdVectorStrings(shapePars);
                muxCout.unlock();
            }
            /*
             * TODO: uncomment it, when the isValid() method will be implemented
            for GeoTessellatedSolid if(!sh->isValid()) { error = 1; std::cout <<
            "ERROR! GeoTessellatedSolid shape is not valid!! -- input: " <<
            shapePars;
            }
            */
        }  // END OF if (size>13)
        else {
            muxCout.lock();
            std::cout << "ERROR!! GeoTessellatedSolid has no facets!! --> "
                         "shape input parameters: ";
            printStdVectorStrings(shapePars);
            muxCout.unlock();
            error = 1;
        }
        if (error) {
            muxCout.lock();
            std::cout << "GeoTessellatedSolid shape error!!! Aborting..."
                      << std::endl;
            muxCout.unlock();
            exit(EXIT_FAILURE);
        }
        shape = sh;

    } 
    
    else {
        m_unknown_shapes.insert(
            type);  // save unknwon shapes for later warning message
        shape = buildDummyShape();
    }

    //! store into the cache the shape we have just built,
    //! for later use when referenced by another node
    storeBuiltShape(shapeId, shape);

    return shape;  // FIXME: do we still need the return? probably not, because
                   // we now store all the shapes that have been built in this
                   // method

    // }

    // catch (std::invalid_argument& e)
    // {
    //     std::cerr << e.what() << std::endl;
    //     exit(EXIT_FAILURE);
    // }
}

// TODO: move to an untilities file/class
// void printTuple(tuple_shapes_boolean_info tuple) {
template<typename T> void printTuple(T tuple) {
    std::apply([](auto&&... args) { ((std::cout << args << ", "), ...); },
               tuple);  // needs C++17
    std::cout << std::endl;
}
// TODO: move to an untilities file/class
void inspectListShapesToBuild(type_shapes_boolean_info& list) {
    for (auto tuple : list) {
        printTuple(tuple);
        std::cout << std::endl;
    }
}
// TODO: move to an untilities file/class
void inspectListShapesToBuild(boolean_shapes_operands_info& list) {
    for (const auto& tuple : list) {
        printTuple(tuple);
        std::cout << std::endl;
    }
}

// OLD VERSION, REMOVE IT WHEN THE NEW DB SCHEMA IS IN PRODUCTION
void ReadGeoModel::createBooleanShapeOperands(
    type_shapes_boolean_info* shapes_info_sub) {
    std::cout << "OLD VERSION" << std::endl;
    if (shapes_info_sub->size() == 0) return;

    // debug
    // std::cout << "\ncreateBooleanShapeOperands() - start..." <<
    // std::endl; inspectListShapesToBuild(shapes_info_sub);

    // Iterate over the list. The size may be incremented while iterating
    // (therefore, we cannot use iterators)
    for (type_shapes_boolean_info::size_type ii = 0;
         ii < shapes_info_sub->size(); ++ii) {
        // get the tuple containing the data about the operand shapes to
        // build
        tuple_shapes_boolean_info tuple = (*shapes_info_sub)[ii];
        // std::cout << "tuple: "; printTuple(tuple); // debug

        // Initializing variables for unpacking
        unsigned int shapeID = 0;       // std::get<0>(tuple);
        GeoShape* boolShPtr = nullptr;  // std::get<1>(tuple);
        unsigned int idA = 0;           // std::get<2>(tuple);
        unsigned int idB = 0;           // std::get<3>(tuple);

        // use 'tie' to unpack the tuple values into separate variables
        std::tie(shapeID, boolShPtr, idA, idB) = tuple;

        if (shapeID == 0 || boolShPtr == nullptr || idA == 0 || idB == 0) {
            muxCout.lock();
            std::cout << "ERROR! Boolean/Operator shape - shape is NULL or "
                         "operands' IDs are not defined! (shapeID: "
                      << shapeID << ", idA: " << idA << ", idB:" << idB << ")"
                      << std::endl;
            muxCout.unlock();
            exit(EXIT_FAILURE);
        }

        if (isShapeBoolean(shapeID)) {
            GeoShape* shapeA = nullptr;
            GeoShape* shapeB = nullptr;

            // if both operands are built already...
            if (isBuiltShape(idA) && isBuiltShape(idB)) {
                // then build the operator shape...
                shapeA = getBuiltShape(idA);
                shapeB =
                    getBuiltShape(idB);  // TODO: customize for Shift as well
            } else {
                // otherwise, build the operand shapes
                shapeA = getBooleanReferencedShape(idA, shapes_info_sub);
                shapeB = getBooleanReferencedShape(idB, shapes_info_sub);
            }
            // Now, assign the new shapes to the boolean shape we're
            // building
            if (dynamic_cast<GeoShapeIntersection*>(boolShPtr)) {
                GeoShapeIntersection* ptr =
                    dynamic_cast<GeoShapeIntersection*>(boolShPtr);
                ptr->m_opA = shapeA;
                ptr->m_opB = shapeB;
                ptr->m_opA->ref();
                ptr->m_opB->ref();
            } else if (dynamic_cast<GeoShapeSubtraction*>(boolShPtr)) {
                GeoShapeSubtraction* ptr =
                    dynamic_cast<GeoShapeSubtraction*>(boolShPtr);
                ptr->m_opA = shapeA;
                ptr->m_opB = shapeB;
                ptr->m_opA->ref();
                ptr->m_opB->ref();
            } else if (dynamic_cast<GeoShapeUnion*>(boolShPtr)) {
                GeoShapeUnion* ptr = dynamic_cast<GeoShapeUnion*>(boolShPtr);
                ptr->m_opA = shapeA;
                ptr->m_opB = shapeB;
                ptr->m_opA->ref();
                ptr->m_opB->ref();
            } else {
                // TODO: move to standard error message for all instances
                std::cout << "ERROR!!! shape is not boolean/operator! Write to "
                             "'geomodel-developers@cern.ch'. Exiting..."
                          << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if ("Shift" == getShapeType(shapeID)) {
            GeoShape* opShape = nullptr;
            GeoTrf::Transform3D shiftX;
            GeoTransform* shiftTransf =
                nullptr;  // TODO: remove the need for a temp GeoTransform,
                          // store the bare transforms as well...

            // if both operands are built already...
            if (isBuiltShape(idA) && isBuiltTransform(idB)) {
                // then build the operator shape...
                opShape = getBuiltShape(idA);
                shiftTransf = getBuiltTransform(idB);
            } else {
                // otherwise, build the operand shapes
                opShape = getBooleanReferencedShape(idA, shapes_info_sub);
                shiftTransf = buildTransform(idB);
            }
            shiftX = shiftTransf->getTransform();
            //shiftTransf->unref();  refcount is zero already

            if (dynamic_cast<GeoShapeShift*>(boolShPtr)) {
                GeoShapeShift* ptr = dynamic_cast<GeoShapeShift*>(boolShPtr);
                ptr->m_op = opShape;
                ptr->m_shift = shiftX;
                ptr->m_op->ref();
            } else {
                std::cout
                    << "ERROR!!! shape is not a Shift operator! Exiting..."
                    << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            std::cout << "ERROR! Undefined operator shape! This part of the "
                         "code should not be reached! Exiting..."
                      << std::endl;
            exit(EXIT_FAILURE);
        }
        // then, store the now completed shape and continue to the next item
        storeBuiltShape(shapeID, boolShPtr);
    }
}

// NEW VERSION FOR THE NEW DB SCHEMA!
void ReadGeoModel::createBooleanShapeOperands(boolean_shapes_operands_info* shapes_info_sub) {
    if (shapes_info_sub->size() == 0) return;

    // debug
    if (m_loglevel >= 3)
    {
        std::cout << "\ncreateBooleanShapeOperands() - start..." << std::endl;
        inspectListShapesToBuild(*shapes_info_sub);
        std::cout << std::endl;
    }

    // Iterate over the list. The size may be incremented while iterating
    // (therefore, we cannot use iterators)
    for (boolean_shapes_operands_info::size_type ii = 0;
         ii < shapes_info_sub->size(); ++ii) {
        // get the tuple containing the data about the operand shapes to
        // build
        tuple_boolean_shapes_operands tuple = (*shapes_info_sub)[ii];
        // std::cout << "tuple: "; printTuple(tuple); // debug

        // Initializing variables for unpacking
        std::string shapeType;     
        unsigned shapeID{0};       
        GeoShape* boolShPtr{nullptr}; 
        std::string typeA;          
        unsigned idA{0};           
        std::string typeB;         
        unsigned idB{0};           
        // use 'tie' to unpack the tuple values into separate variables
        std::tie(shapeType, shapeID, boolShPtr, typeA, idA, typeB, idB) = tuple;

        if (shapeID == 0 || boolShPtr == nullptr || idA == 0 || idB == 0) {
            THROW_EXCEPTION("ERROR! Boolean/Operator shape - shape is NULL or operands' IDs are not defined!");
        }

        if (isShapeBoolean(shapeType)) {
            GeoShape* shapeA = nullptr;
            GeoShape* shapeB = nullptr;

            // if both operands are built already...
            if (isBuiltShape(typeA, idA) && isBuiltShape(typeB, idB)) {
                // then build the operator shape...
                shapeA = getBuiltShape(idA, typeA);
                shapeB = getBuiltShape(idB, typeB);
            } else {
                // otherwise, build the operand shapes
                shapeA = getBooleanReferencedShape(typeA, idA, shapes_info_sub);
                shapeB = getBooleanReferencedShape(typeB, idB, shapes_info_sub);
            }
            // Now, assign the new shapes to the boolean shape we're
            // building
            if (dynamic_cast<GeoShapeIntersection*>(boolShPtr)) {
                GeoShapeIntersection* ptr =
                    dynamic_cast<GeoShapeIntersection*>(boolShPtr);
                ptr->m_opA = shapeA;
                ptr->m_opB = shapeB;
                ptr->m_opA->ref();
                ptr->m_opB->ref();
            } else if (dynamic_cast<GeoShapeSubtraction*>(boolShPtr)) {
                GeoShapeSubtraction* ptr =
                    dynamic_cast<GeoShapeSubtraction*>(boolShPtr);
                ptr->m_opA = shapeA;
                ptr->m_opB = shapeB;
                ptr->m_opA->ref();
                ptr->m_opB->ref();
            } else if (dynamic_cast<GeoShapeUnion*>(boolShPtr)) {
                GeoShapeUnion* ptr = dynamic_cast<GeoShapeUnion*>(boolShPtr);
                ptr->m_opA = shapeA;
                ptr->m_opB = shapeB;
                ptr->m_opA->ref();
                ptr->m_opB->ref();
            } else {
                THROW_EXCEPTION("ERROR!!! shape is not boolean/operator! Write to 'geomodel-developers@cern.ch'...");
            }
        } else if ("Shift" == shapeType) {
            GeoShape* opShape = nullptr;
            GeoTrf::Transform3D shiftX;
            GeoTransform* shiftTransf =
                nullptr;  // TODO: remove the need for a temp GeoTransform,
                          // store the bare transforms as well...

            // if both operands are built already...
            if (isBuiltShape(typeA, idA) && isBuiltTransform(idB)) {
                // then build the operator shape...
                opShape = getBuiltShape(idA, typeA);
                shiftTransf = getBuiltTransform(idB);
            } else {
                // otherwise, build the operand shapes
                opShape = getBooleanReferencedShape(typeA, idA, shapes_info_sub);
                shiftTransf = buildTransform(idB);
            }
            shiftX = shiftTransf->getTransform();
            //shiftTransf->unref();  refcount is zero already

            if (dynamic_cast<GeoShapeShift*>(boolShPtr)) {
                GeoShapeShift* ptr = dynamic_cast<GeoShapeShift*>(boolShPtr);
                ptr->m_op = opShape;
                ptr->m_shift = shiftX;
                ptr->m_op->ref();
            } else {
                std::cout
                    << "ERROR!!! shape is not a Shift operator! Exiting..."
                    << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            THROW_EXCEPTION("ERROR! Undefined operator shape! This part of the code should not be reached!");
        }
        // then, store the now completed shape and continue to the next item
        storeBuiltShape(shapeType, shapeID, boolShPtr);
    }
}

GeoShape* ReadGeoModel::getBooleanReferencedShape(
    const unsigned int shapeID, type_shapes_boolean_info* shapes_info_sub) {
    if (0 == shapeID) {
        std::cout << "ERROR!! ShapeID = 0!" << std::endl;
        exit(EXIT_FAILURE);
    }

    GeoShape* shape;
    // if A is built already, then take it from cache
    if (isBuiltShape(shapeID)) {
        if (m_loglevel >= 2)
            std::cout << "operandA is built, taking it from cache..."
                      << std::endl;  // debug
        shape = getBuiltShape(shapeID);
    } else {
        // if not built and not a boolean shape, then build it
        if (!isShapeOperator(shapeID)) {
            if (m_loglevel >= 2)
                std::cout << "operandA is not built and not an operator, "
                             "build it..."
                          << std::endl;  // debug
            shape = buildShape(shapeID, shapes_info_sub);
            if (shape == NULL) {
                std::cout << "ERROR!!! shape is NULL!" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        // if A is a boolean shape, then create an empty shape,
        // store it for later completion, and use that
        else {
            if (m_loglevel >= 2)
                std::cout << "operandA is not built and it is an operator, add "
                             "it to build it later..."
                          << std::endl;  // debug
            shape = addEmptyBooleanShapeForCompletion(shapeID, shapes_info_sub);
        }
    }

    //   inspectListShapesToBuild(m_shapes_info_sub); // debug
    return shape;
}
GeoShape* ReadGeoModel::getBooleanReferencedShape(
    const std::string_view shapeType, const unsigned shapeID, boolean_shapes_operands_info* shapes_info_sub) {
    if (0 == shapeID || "" == shapeType) {
        THROW_EXCEPTION("ERROR!! ShapeID == 0 or shapeType == NULL !");
    }

    GeoShape* shape{nullptr};
    // if A is built already, then take it from cache
    if (isBuiltShape(shapeID)) {
        if (m_loglevel >= 2)
            std::cout << "operandA is built, taking it from cache..."
                      << std::endl;  // debug
        shape = getBuiltShape(shapeID, shapeType);
    } else {
        // // if not a boolean shape, then build it
        // if (!isShapeOperator(shapeType)) {
        //     if (m_loglevel >= 2)
        //         std::cout << "operandA is not built and not an operator, "
        //                      "build it..."
        //                   << std::endl;  // debug
        //     shape = getBuiltShape(shapeID, shapeType);
        //     if ( nullptr == shape ) {
        //         THROW_EXCEPTION("ERROR!!! shape is NULL!");
        //     }
        // }
        // if A is a boolean shape, then create an empty shape,
        // store it for later completion, and use that
        if (isShapeOperator(shapeType)) {
            if (m_loglevel >= 2)
                std::cout << "operandA is not built and it is an operator, add "
                             "it to build it later..."
                          << std::endl;  // debug
            shape = addEmptyBooleanShapeForCompletion(shapeType, shapeID, shapes_info_sub);
        } else {
            THROW_EXCEPTION("Undefined behavior... Check the shape operand!");
        }
    }
    if ( nullptr == shape ) {
        THROW_EXCEPTION("ERROR!!! shape is NULL!");
    }

    //   inspectListShapesToBuild(m_shapes_info_sub); // debug
    return shape;
}

GeoShape* ReadGeoModel::addEmptyBooleanShapeForCompletion(
    const unsigned int shapeID, type_shapes_boolean_info* shapes_info_sub) {
    // get the operands' IDs,
    // build an empty instance of the appropriate boolean/operator shape,
    // and store all of that together, by appending to this same container,
    // so it will be visited at a later time during this very same loop
    std::pair<unsigned int, unsigned int> ops =
        getBooleanShapeOperands(shapeID);
    unsigned int opA = ops.first;
    unsigned int opB = ops.second;

    // get the type and create an appropriate empty shape
    std::string shType = getShapeType(shapeID);
    GeoShape* shape = nullptr;
    if (shType == "Intersection") {
        shape = new GeoShapeIntersection();
    } else if (shType == "Shift") {
        shape = new GeoShapeShift();
    } else if (shType == "Subtraction") {
        shape = new GeoShapeSubtraction();
    } else if (shType == "Union") {
        shape = new GeoShapeUnion();
    }

    tuple_shapes_boolean_info tt(shapeID, shape, opA, opB);
    shapes_info_sub->push_back(
        tt);  //! Push the information about the new boolean shape at the
              //! end of the very same container we are iterating over

    return shape;
}
GeoShape *ReadGeoModel::addEmptyBooleanShapeForCompletion(
    const std::string_view shapeType, const unsigned shapeID, boolean_shapes_operands_info *shapes_info_sub)
{
    // get the operands' IDs,
    // build an empty instance of the appropriate boolean/operator shape,
    // and store all of that together, by appending to this same container,
    // so it will be visited at a later time during this very same loop
    std::tuple<std::string, unsigned int, std::string, unsigned int> ops =
        getBooleanShapeOperands(shapeType, shapeID);
    std::string opAType;
    unsigned opAId{0};  
    std::string opBType;
    unsigned opBId{0};  
    std::tie(opAType, opAId, opBType, opBId) = ops;

    GeoShape *shape = nullptr;
    if (shapeType == "Intersection")
    {
        shape = new GeoShapeIntersection();
    }
    else if (shapeType == "Shift")
    {
        shape = new GeoShapeShift();
    }
    else if (shapeType == "Subtraction")
    {
        shape = new GeoShapeSubtraction();
    }
    else if (shapeType == "Union")
    {
        shape = new GeoShapeUnion();
    }

    tuple_boolean_shapes_operands tt(shapeType, shapeID, shape, opAType, opAId, opBType, opBId);
    shapes_info_sub->push_back(
        tt); //! Push the information about the new boolean shape at the
             //! end of the very same container we are iterating over

    return shape;
}

// TODO: move this to utility class/file
std::vector<std::string> ReadGeoModel::splitString(const std::string& s,
                                                   const char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

//  // TODO: move this to utility class/file
// QStringList ReadGeoModel::toQStringList(std::vector<std::string> vec)
//  {
//    QStringList ll;
//    for ( auto& str : vec ) {
//      ll << QString::fromStdString(str);
//    }
//    return ll;
//  }

//// TODO: move this to utility class/file
// std::vector<std::string> ReadGeoModel::toStdVectorStrings(QStringList qlist)
//{
//   std::vector<std::string> vec;
//   foreach(QString qstr, qlist) {
//     vec.push_back(qstr.toStdString());
//   }
//   return vec;
// }

// TODO: move this to utility class/file
void ReadGeoModel::printStdVectorStrings(const std::vector<std::string>& vec) {
    for (const auto& str : vec) {
        std::cout << str << " ";
    }
    std::cout << std::endl;
    return;
}

std::pair<unsigned int, unsigned int> ReadGeoModel::getBooleanShapeOperands(
    const unsigned int shapeID) {
    std::pair<unsigned int, unsigned int> pair;

    //  std::vector<std::string> paramsShape = toStdVectorStrings(m_shapes[
    //  shapeID ]);
    std::vector<std::string> paramsShape =
        m_shapes[shapeID - 1];  // remember: shapes' IDs start from 1

    //  unsigned int id = std::stoi(paramsShape[0]); //! the ID of the
    //  boolean/operator shape
    std::string type = paramsShape[1];        //! the GeoModel type of the shape
    std::string parameters = paramsShape[2];  //! the parameters defining the
                                              //! shape, coming from the DB

    //! The Subtraction boolean shape has two operands, here we store their IDs
    unsigned int opA = 0;
    unsigned int opB = 0;
    // get parameters from DB string
    std::vector<std::string> shapePars = splitString(parameters, ';');
    // std::cout << "shapePars size: " << shapePars.size() << std::endl; //
    // debug only

    for (const auto& par : shapePars) {
        std::vector<std::string> vars = splitString(par, '=');
        std::string varName = vars[0];
        std::string varValue = vars[1];
        // std::cout << "vars ==> " << vars[0] << ", " << vars[1] << std::endl;
        // // debug
        if (isShapeBoolean(type)) {
            if (varName == "opA") opA = std::stoi(varValue);
            if (varName == "opB") opB = std::stoi(varValue);
        } else if ("Shift" == type) {
            if (varName == "A") opA = std::stoi(varValue);
            if (varName == "X") opB = std::stoi(varValue);
        } else {
            std::cout << "ERROR!!! This type of shape cannot be handled here: "
                      << type << ", --> Exiting..." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    if (opA == 0 || opB == 0) {
        std::cout << "ERROR! Intersection/Subtraction/Union/Shift shape - "
                     "operands' IDs are empty! (opA: "
                  << opA << ", opB:" << opB << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    pair.first = opA;
    pair.second = opB;

    return pair;
}
std::tuple<std::string, unsigned int, std::string, unsigned int> ReadGeoModel::getBooleanShapeOperands(
    const std::string_view shapeType, const unsigned int shapeID)
{

    DBRowEntry record;
    if ("Shift" == shapeType)
        record = m_shapes_Shift[shapeID - 1];
    else if ("Subtraction" == shapeType)
        record = m_shapes_Subtraction[shapeID - 1];
    else if ("Intersection" == shapeType)
        record = m_shapes_Intersection[shapeID - 1];
    else if ("Union" == shapeType)
        record = m_shapes_Union[shapeID - 1];

    unsigned shapeId{0};
    double shapeVolume{0};
    std::string shapeOpAType;
    unsigned shapeOpAId{0};
    std::string shapeOpBType;
    unsigned shapeOpBId{0};

    shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(record[0], "Boolean:shapeID");
    // computed volume (not defined by default)
    shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(record[1], "Boolean:shapeVolume");
    // shape operands
    shapeOpAType = GeoModelHelpers::variantHelper::getFromVariant_String(record[2], "Boolean:shapeType");
    shapeOpAId = GeoModelHelpers::variantHelper::getFromVariant_Int(record[3], "Boolean:shapeId");
    if ("Shift" == shapeType)
    {
        shapeOpBType = "NULL";
        shapeOpBId = GeoModelHelpers::variantHelper::getFromVariant_Int(record[4], "Boolean:shapeId");
    }
    else
    {
        shapeOpBType = GeoModelHelpers::variantHelper::getFromVariant_String(record[4], "Boolean:shapeType");
        shapeOpBId = GeoModelHelpers::variantHelper::getFromVariant_Int(record[5], "Boolean:shapeId");
    }

    if (shapeOpAId == 0 || shapeOpBId == 0 || shapeOpAType.empty() || shapeOpBType.empty())
    {
        THROW_EXCEPTION("ERROR! Intersection/Subtraction/Union/Shift shape operands' info are empty!");
    }

    return std::make_tuple(shapeOpAType, shapeOpAId, shapeOpBType, shapeOpBId);
}

bool ReadGeoModel::isShapeOperator(const unsigned int shapeId) {
    return isShapeOperator(getShapeType(shapeId));
}
bool ReadGeoModel::isShapeOperator(const std::string_view type) {
    std::unordered_set<std::string> opShapes = {"Intersection", "Shift",
                                                "Subtraction", "Union"};
    return (opShapes.find(std::string(type)) != opShapes.end());
}

bool ReadGeoModel::isShapeBoolean(const unsigned int shapeId) {
    return isShapeBoolean(getShapeType(shapeId));
}
bool ReadGeoModel::isShapeBoolean(const std::string_view type) {
    std::unordered_set<std::string> opShapes = {"Intersection", "Subtraction",
                                                "Union"};
    return (opShapes.find(std::string(type)) != opShapes.end());
}

GeoBox* ReadGeoModel::buildDummyShape() {
    return new GeoBox(30.0 * SYSTEM_OF_UNITS::cm, 30 * SYSTEM_OF_UNITS::cm,
                      30 * SYSTEM_OF_UNITS::cm);
}

void ReadGeoModel::buildVSurface(const unsigned int id) {
    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "buildVSurface(), testing VSurface id: " << id << "...\n";
        ;
        muxCout.unlock();
    }
    
    if(!isBuiltVSurface(id)){  // the VSurface hasnt been built yet
      DBRowEntry values = m_VSurface[id - 1];
      const std::string shapeType = GeoModelHelpers::variantHelper::getFromVariant_String(values[1], "VSurf_shapeType");
      const unsigned int shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(values[2], "VSurf_shapeID");
      GeoVSurfaceShape* buildVSurfShape = BuildWhichSurface(shapeType, shapeId);  // build the shape of surface
      GeoVSurface* buildVSurf = new GeoVSurface(buildVSurfShape);                 // build the virtual surface class itself
      storeBuiltVSurface(buildVSurf);
    }
    // else do nothing
}
//-----------------------------------------------------------------------

GeoLogVol* ReadGeoModel::buildLogVol(const unsigned int id) {
    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "buildLogVol(), testing LogVol id: " << id << "...\n";
        ;
        muxCout.unlock();
    }

    if (isBuiltLog(id)) {
        if (m_loglevel >= 3) {
            muxCout.lock();
            std::cout << "getting the LogVol from cache...\n";
            ;
            muxCout.unlock();
        }
        return getBuiltLog(id);
    }

    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "ReadGeoModel::buildLogVol()" << std::endl;
        muxCout.unlock();
    }

    // get logVol properties from the DB
    // std::vector<std::string> values = m_logVols[id - 1];
    DBRowEntry values = m_logVols[id - 1];

    // --- get the parameters to build the GeoLogVol node
    // get the name of the LogVol
    const std::string logVolName = GeoModelHelpers::variantHelper::getFromVariant_String(values[1], "LogVol_name");
    // get the ID and the type of the referenced GeoShape node
    const int shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(values[2], "LogVol_shapeID");
    const std::string shapeType = GeoModelHelpers::variantHelper::getFromVariant_String(values[3], "LogVol_shapeType");
    GeoShape* shape = getBuiltShape(shapeId, shapeType);
    if (!shape) {
            THROW_EXCEPTION("ERROR!! While building the LogVol '" + logVolName + "', shape of type '" + shapeType + "' is NULL! Exiting...");
    }

    // build the referenced GeoMaterial node
    // const unsigned int matId = std::stoi(values[3]);
    const int matId = GeoModelHelpers::variantHelper::getFromVariant_Int(values[4], "LogVol_MaterialID");

    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "buildLogVol() - material Id:" << matId << std::endl;
        muxCout.unlock();
    }
    GeoMaterial* mat = getBuiltMaterial(matId);
    if (!mat) {
            THROW_EXCEPTION("ERROR!! While building the LogVol '" + logVolName + "', Material of ID '" + std::to_string(matId) + "' is NULL! Exiting...");
    }

    GeoLogVol* logPtr = new GeoLogVol(logVolName, shape, mat);
    storeBuiltLog(logPtr);
    if (m_loglevel >= 3) {
        muxCout.lock();
        std::cout << "buildLogVol() - address of the stored LogVol:" << logPtr
                  << std::endl;
        muxCout.unlock();
    }
    return logPtr;
}


//// TODO: should go in a QtUtils header-only class, to be used in other
/// packages
// QList<double> ReadGeoModel::convertQstringListToDouble(QStringList listin) {
//   QList<double> listout;
//   foreach (const QString &s, listin) {
//       listout.append(s.toDouble());
//   }
//   return listout;
// }

//// TODO: move it to an utility class
// void ReadGeoModel::printTransformationValues(QStringList values) {
//   QList<double> t = convertQstringListToDouble(values);
//   muxCout.lock();
//   std::cout << "transformation input values: " << std::endl;
//   std::cout << "[[" << t[0] << "," << t[1] << "," << t[2] << "]["
//                      << t[3] << "," << t[4] << "," << t[5] << "]["
//                      << t[6] << "," << t[7] << "," << t[8] << "]["
//                      << t[9] << "," << t[10] << "," << t[11] << "]]" <<
//                      std::endl;
//   muxCout.unlock();
// }

GeoAlignableTransform* ReadGeoModel::buildAlignableTransform(
    const unsigned int id) {
    if (isBuiltAlignableTransform(id)) {
        return getBuiltAlignableTransform(id);
    }

    std::vector<std::string> values =
        m_alignableTransforms[id - 1];  // nodes' IDs start from 1
    values.erase(values.begin());       // remove the first element, that is the
                                   // 'id', leaving the other items in the list

    // get the 12 matrix elements
    double xx = std::stod(values[0]);
    double xy = std::stod(values[1]);
    double xz = std::stod(values[2]);

    double yx = std::stod(values[3]);
    double yy = std::stod(values[4]);
    double yz = std::stod(values[5]);

    double zx = std::stod(values[6]);
    double zy = std::stod(values[7]);
    double zz = std::stod(values[8]);

    double dx = std::stod(values[9]);
    double dy = std::stod(values[10]);
    double dz = std::stod(values[11]);

    GeoTrf::Transform3D txf;
    // build the rotation matrix with the first 9 elements
    txf(0, 0) = xx;
    txf(0, 1) = xy;
    txf(0, 2) = xz;

    txf(1, 0) = yx;
    txf(1, 1) = yy;
    txf(1, 2) = yz;

    txf(2, 0) = zx;
    txf(2, 1) = zy;
    txf(2, 2) = zz;

    // build the translation matrix with the last 3 elements
    txf(0, 3) = dx;
    txf(1, 3) = dy;
    txf(2, 3) = dz;

    // GeoUtilFunctions::printTrf(txf); // DEBUG
    GeoAlignableTransform* tr = new GeoAlignableTransform(txf);
    storeBuiltAlignableTransform(tr);
    return tr;
}

GeoTransform* ReadGeoModel::buildTransform(const unsigned int id) {
    if (isBuiltTransform(id)) {
        return getBuiltTransform(id);
    }

    std::vector<std::string> values =
        m_transforms[id - 1];      // nodes' IDs start from 1
    values.erase(values.begin());  // remove the first element, that is the
                                   // 'id', leaving the other items in the list

    // get the 12 matrix elements
    double xx = std::stod(values[0]);
    double xy = std::stod(values[1]);
    double xz = std::stod(values[2]);

    double yx = std::stod(values[3]);
    double yy = std::stod(values[4]);
    double yz = std::stod(values[5]);

    double zx = std::stod(values[6]);
    double zy = std::stod(values[7]);
    double zz = std::stod(values[8]);

    double dx = std::stod(values[9]);
    double dy = std::stod(values[10]);
    double dz = std::stod(values[11]);

    GeoTrf::Transform3D txf;
    // build the rotation matrix with the first 9 elements
    txf(0, 0) = xx;
    txf(0, 1) = xy;
    txf(0, 2) = xz;

    txf(1, 0) = yx;
    txf(1, 1) = yy;
    txf(1, 2) = yz;

    txf(2, 0) = zx;
    txf(2, 1) = zy;
    txf(2, 2) = zz;

    // build the translation matrix with the last 3 elements
    txf(0, 3) = dx;
    txf(1, 3) = dy;
    txf(2, 3) = dz;

    // GeoUtilsFunctions::printTrf(txf); // DEBUG
    GeoTransform* tr = new GeoTransform(txf);
    storeBuiltTransform(tr);
    return tr;
}

GeoSerialTransformer* ReadGeoModel::buildSerialTransformer(
    const unsigned int id) {
    muxCout.lock();
    if (m_loglevel >= 2)
        std::cout << "ReadGeoModel::buildSerialTransformer()" << std::endl;
    muxCout.unlock();

    const unsigned int nodeID = id - 1;  // nodes' IDs start from 1
    std::vector<std::string> values = m_serialTransformers[nodeID];

    const unsigned int functionId = std::stoi(values[1]);
    const unsigned int physVolId = std::stoi(values[2]);
    const unsigned int physVolTableId = std::stoi(values[3]);
    const unsigned int copies = std::stoi(values[4]);

    // GET THE REFERENCED FUNCTION
    TRANSFUNCTION func = buildFunction(functionId);

    // GET THE REFERENCED VPHYSVOL
    const GeoVPhysVol* vphysVol = buildVPhysVolInstance(
        physVolId, physVolTableId,
        1);  // we use "1" as default copyNumber: taking the first copy of the
             // VPhysVol as the referenced volume

    // get PhysVol or FullPhysVol pointer and return the SerialTransformer
    if (dynamic_cast<const GeoFullPhysVol*>(vphysVol)) {
        const GeoFullPhysVol* vol =
            dynamic_cast<const GeoFullPhysVol*>(vphysVol);
        GeoSerialTransformer* nodePtr =
            new GeoSerialTransformer(vol, &func, copies);
        storeBuiltSerialTransformer(nodePtr);
        return nodePtr;
    }
    const GeoPhysVol* vol = dynamic_cast<const GeoPhysVol*>(vphysVol);
    GeoSerialTransformer* nodePtr =
        new GeoSerialTransformer(vol, &func, copies);
    storeBuiltSerialTransformer(nodePtr);
    return nodePtr;
}

TRANSFUNCTION ReadGeoModel::buildFunction(const unsigned int id) {
    /* FIXME: see below
    if( isBuiltFunction(id) ) {
     GeoXF::Function* fPtr = getBuiltFunction(id);
     TRANSFUNCTION tf = (*fPtr); // Remember: "typedef const Function &
    TRANSFUNCTION" return tf;
    }
     */

    std::string expr = std::get<std::string>(m_functions[id - 1][1]);  // nodes' IDs start from 1
    unsigned dataStart = std::get<int>(m_functions[id - 1][2]);
    unsigned dataEnd = std::get<int>(m_functions[id - 1][3]);
    // debug msg
    // std::cout << "expr:" << expr << " -- dataStart,End: " << dataStart << ", " << dataEnd << std::endl;

    if (0 == expr.size()) {
        muxCout.lock();
        THROW_EXCEPTION("FATAL ERROR!! Function expression is empty!! Aborting...");
        muxCout.unlock();
    }

    // get exprData, extract subvector
    // NOTE: we use (dataStart-1) to cope with the difference between the DB rows starting from '1', 
    //       which is what the 'dataStart' stores, and the vector items, which start '0'; 
    //       also, the constructor of the sub-vector takes the element from 'begin+dataStart-1' included
    //       and 'begin+dataEnd' excluded.
    std::deque<double> sub_vector(m_funcExprData.begin() + (dataStart-1),
                           m_funcExprData.begin() + (dataEnd) );

    TransFunctionInterpreter interpreter;
    TFPTR func = interpreter.interpret(expr, &sub_vector);
    TRANSFUNCTION tf =
        *(func.release());  // make func returns a pointer to the managed
                            // object and releases the ownership, then get
                            // the object dereferencing the pointer

    //-- FIXME: At the moment, enabling this cache makes the app crash at the
    //-- end, when calling the destructor. I suspect because the pointers are
    //-- not correct and so removing them throws an error.
    // Get a non-const pointer to the Function object,
    // and store that into the cache.
    // Remember: "typedef const Function & TRANSFUNCTION"
    /*
    const GeoXF::Function* fPtrConst = &tf;
        GeoXF::Function* fPtr = const_cast<GeoXF::Function*>(fPtrConst);
        storeBuiltFunction(id, fPtr);
    */

    return tf;
}
// --- methods for caching GeoShape nodes ---
bool ReadGeoModel::isBuiltShape(const unsigned int id) {
return (!(m_memMapShapes.find(id) == m_memMapShapes.end()));
}
bool ReadGeoModel::isBuiltShape_Operators_Shift(const unsigned int id) {
return (!(m_memMapShapes_Shift.find(id) == m_memMapShapes_Shift.end()));
}
bool ReadGeoModel::isBuiltShape_Operators_Subtraction(const unsigned int id) {
return (!(m_memMapShapes_Subtraction.find(id) == m_memMapShapes_Subtraction.end()));
}
bool ReadGeoModel::isBuiltShape_Operators_Intersection(const unsigned int id) {
return (!(m_memMapShapes_Intersection.find(id) == m_memMapShapes_Intersection.end()));
}
bool ReadGeoModel::isBuiltShape_Operators_Union(const unsigned int id) {
return (!(m_memMapShapes_Union.find(id) == m_memMapShapes_Union.end()));
}
// --- methods for caching GeoShape nodes ---
bool ReadGeoModel::isBuiltShape(std::string_view shapeType, const unsigned shapeId) {
const std::set<std::string> shapesNewDB{"Box", "EllipticalTube", "Tube", "Pcon", "Cons", "Para", "Pgon", "Trap", "Trd", "Tubs", "Torus", "TwistedTrap", "SimplePolygonBrep", "GenericTrap", "Shift", "Subtraction", "Intersection", "Union"};
    // get shape parameters
    if (std::count(shapesNewDB.begin(), shapesNewDB.end(), shapeType))
    {
        if ("Box" == shapeType)
        {
            return m_builderShape_Box->isBuiltShape(shapeId);
        }
        else if ("EllipticalTube" == shapeType)
        {
            return m_builderShape_EllipticalTube->isBuiltShape(shapeId);
        }
        else if ("Tube" == shapeType)
        {
            return m_builderShape_Tube->isBuiltShape(shapeId);
        }
        else if ("Pcon" == shapeType)
        {
            return m_builderShape_Pcon->isBuiltShape(shapeId);
        } 
        else if ("Cons" == shapeType)
        {
            return m_builderShape_Cons->isBuiltShape(shapeId);
        } 
        else if ("Para" == shapeType)
        {
            return m_builderShape_Para->isBuiltShape(shapeId);
        } 
        else if ("Pgon" == shapeType)
        {
            return m_builderShape_Pgon->isBuiltShape(shapeId);
        } 
        else if ("Trap" == shapeType)
        {
            return m_builderShape_Trap->isBuiltShape(shapeId);
        } 
        else if ("Trd" == shapeType)
        {
            return m_builderShape_Trd->isBuiltShape(shapeId);
        } 
        else if ("Tubs" == shapeType)
        {
            return m_builderShape_Tubs->isBuiltShape(shapeId);
        } 
        else if ("Torus" == shapeType)
        {
            return m_builderShape_Torus->isBuiltShape(shapeId);
        } 
        else if ("TwistedTrap" == shapeType)
        {
            return m_builderShape_TwistedTrap->isBuiltShape(shapeId);
        } 
        else if ("SimplePolygonBrep" == shapeType)
        {
            return m_builderShape_SimplePolygonBrep->isBuiltShape(shapeId);
        } 
        else if ("GenericTrap" == shapeType)
        {
            return m_builderShape_GenericTrap->isBuiltShape(shapeId);
        } 
        else if ("Shift" == shapeType)
        {
            return isBuiltShape_Operators_Shift(shapeId);
        } 
        else if ("Intersection" == shapeType)
        {
            return isBuiltShape_Operators_Intersection(shapeId);
        } 
        else if ("Subtraction" == shapeType)
        {
            return isBuiltShape_Operators_Subtraction(shapeId);
        } 
        else if ("Union" == shapeType)
        {
            return isBuiltShape_Operators_Union(shapeId);
        } 
        else {
            THROW_EXCEPTION("ERROR!!! Shape '" + std::string(shapeType) + "' is not handled correctly!");
        }
    }
    std::cout << "WARNING! 'isBuiltShape' - For the shape '" << shapeType << "' we're using the old DB schema..." << std::endl;
    return isBuiltShape(shapeId);
}
void ReadGeoModel::storeBuiltShape(const unsigned id, GeoShape* nodePtr) {
    m_memMapShapes[id] = nodePtr;
}

void ReadGeoModel::storeBuiltShape(const std::string_view type, const unsigned id, GeoShape *nodePtr)
{
    if ("Shift" == type)
        storeBuiltShapeOperators_Shift(id, nodePtr);
    else if ("Subtraction" == type)
        storeBuiltShapeOperators_Subtraction(id, nodePtr);
    else if ("Intersection" == type)
        storeBuiltShapeOperators_Intersection(id, nodePtr);
    else if ("Union" == type)
        storeBuiltShapeOperators_Union(id, nodePtr);
    else
        THROW_EXCEPTION("ERROR!!! Undefined operator type!");
}

void ReadGeoModel::storeBuiltShapeOperators_Shift(const unsigned id, GeoShape* nodePtr) {
    m_memMapShapes_Shift[id] = nodePtr;
}
void ReadGeoModel::storeBuiltShapeOperators_Subtraction(const unsigned id, GeoShape* nodePtr) {
    m_memMapShapes_Subtraction[id] = nodePtr;
}
void ReadGeoModel::storeBuiltShapeOperators_Intersection(const unsigned id, GeoShape* nodePtr) {
    m_memMapShapes_Intersection[id] = nodePtr;
}
void ReadGeoModel::storeBuiltShapeOperators_Union(const unsigned id, GeoShape* nodePtr) {
    m_memMapShapes_Union[id] = nodePtr;
}

//------------------------------------------------------------------------------------------

GeoVSurfaceShape* ReadGeoModel::BuildWhichSurface(std::string_view shapeType, const unsigned int shapeId)
{   
  
  if ("RectangleSurface" == shapeType){
          m_memMapRectSurface.reserve(m_rectangle_surface.size());      
     if(!isBuiltRectSurface(shapeId)){
          // Not built yet
          DBRowEntry row = m_rectangle_surface[shapeId - 1];
          GeoRectSurface* builtSurface = buildsurfTool->buildRectSurface(row);
          storeBuiltRectSurface(builtSurface);
          return builtSurface;     
     }
     else{
          GeoRectSurface* builtSurface = m_memMapRectSurface[shapeId - 1];
          return builtSurface;
     }
     
  }
  else if ("TrapezoidSurface" == shapeType){
          m_memMapTrapezoidSurface.reserve(m_trapezoid_surface.size());      
     if(!isBuiltTrapezoidSurface(shapeId)){
          // Not built yet
          DBRowEntry row = m_trapezoid_surface[shapeId - 1];
          GeoTrapezoidSurface* builtSurface = buildsurfTool->buildTrapezoidSurface(row);
          storeBuiltTrapezoidSurface(builtSurface);
          return builtSurface;     
     }
     else{
          GeoTrapezoidSurface* builtSurface = m_memMapTrapezoidSurface[shapeId - 1];
          return builtSurface;
     }
     
  }
  else if ("AnnulusSurface" == shapeType){
          m_memMapAnnulusSurface.reserve(m_annulus_surface.size());
     if(!isBuiltAnnulusSurface(shapeId)){
          // Not built yet
          DBRowEntry row = m_annulus_surface[shapeId - 1];
          GeoAnnulusSurface* builtSurface = buildsurfTool->buildAnnulusSurface(row);
          storeBuiltAnnulusSurface(builtSurface);
          return builtSurface;
     }
     else{
          GeoAnnulusSurface* builtSurface = m_memMapAnnulusSurface[shapeId - 1];
          return builtSurface;
     }

  }
  else if ("DiamondSurface" == shapeType){
        m_memMapDiamondSurface.reserve(m_diamond_surface.size());
        if(!isBuiltDiamondSurface(shapeId)){
            // Not built yet
            DBRowEntry row = m_diamond_surface[shapeId - 1];
            GeoDiamondSurface* builtSurface = buildsurfTool->buildDiamondSurface(row);
            storeBuiltDiamondSurface(builtSurface);
            return builtSurface;
        }
        else{
            GeoDiamondSurface* builtSurface = m_memMapDiamondSurface[shapeId - 1];
            return builtSurface;
        }
            
  }
  else{
    THROW_EXCEPTION("ERROR!!! VSurface '" + std::string(shapeType) + "' is not built correctly!");
  }    
}

bool ReadGeoModel::isBuiltVSurface(const unsigned int shapeId) {
    return (shapeId <= m_memMapVSurface.size());
}

//------------------------------------------------------------------------------------------

bool ReadGeoModel::isBuiltRectSurface(const unsigned int shapeId) {
    return (shapeId <= m_memMapRectSurface.size());
}

void ReadGeoModel::storeBuiltRectSurface(GeoRectSurface* nodePtr) {
    if (!nodePtr)
        THROW_EXCEPTION(" RECTANGLE SURFACE POINTER IS NULL, NO RECT SURFS ARE BUILT YET ");
    m_memMapRectSurface.push_back(nodePtr);
}

//------------------------------------------------------------------------------------------

bool ReadGeoModel::isBuiltTrapezoidSurface(const unsigned int shapeId) {
    return (shapeId <= m_memMapTrapezoidSurface.size());
}

void ReadGeoModel::storeBuiltTrapezoidSurface(GeoTrapezoidSurface* nodePtr) {
    if (!nodePtr)
        THROW_EXCEPTION(" TRAPEZOID SURFACE POINTER IS NULL, NO TRAPE SURFS ARE BUILT YET ");
    m_memMapTrapezoidSurface.push_back(nodePtr);
}

//------------------------------------------------------------------------------------------

bool ReadGeoModel::isBuiltAnnulusSurface(const unsigned int shapeId) {
    return (shapeId <= m_memMapAnnulusSurface.size());
}

void ReadGeoModel::storeBuiltAnnulusSurface(GeoAnnulusSurface* nodePtr) {
    if (!nodePtr)
        THROW_EXCEPTION(" ANNULUS SURFACE POINTER IS NULL, NO ANNULUS SURFS ARE BUILT YET ");
    m_memMapAnnulusSurface.push_back(nodePtr);
}

//------------------------------------------------------------------------------------------

bool ReadGeoModel::isBuiltDiamondSurface(const unsigned int shapeId) {
    return (shapeId <= m_memMapDiamondSurface.size());
}

void ReadGeoModel::storeBuiltDiamondSurface(GeoDiamondSurface* nodePtr) {
    if (!nodePtr)
        THROW_EXCEPTION(" DIAMOND SURFACE POINTER IS NULL, NO DIAMOND SURFS ARE BUILT YET ");
    m_memMapDiamondSurface.push_back(nodePtr);
}

//------------------------------------------------------------------------------------------

void ReadGeoModel::storeBuiltVSurface(GeoVSurface* nodePtr) {
    if (!nodePtr)
        THROW_EXCEPTION(" VIRTUAL SURFACE POINTER IS NULL, NO VSURFS ARE BUILT YET ");
    m_memMapVSurface.push_back(nodePtr);    
}

//------------------------------------------------------------------------------------------

GeoShape *ReadGeoModel::getBuiltShape(const unsigned shapeId, std::string_view shapeType)
{

    const std::set<std::string> shapesNewDB{"Box", "EllipticalTube", "Tube", "Pcon", "Cons", "Para", "Pgon", "Trap", "Trd", "Tubs", "Torus", "TwistedTrap", "SimplePolygonBrep", "GenericTrap", "Shift", "Intersection", "Subtraction", "Union", "UnidentifiedShape"};
    // get shape parameters
    if (std::count(shapesNewDB.begin(), shapesNewDB.end(), shapeType))
    {
        if ("Box" == shapeType)
        {
            return m_builderShape_Box->getBuiltShape(shapeId);
        }
        else if ("EllipticalTube" == shapeType)
        {
            return m_builderShape_EllipticalTube->getBuiltShape(shapeId);
        }
        else if ("Tube" == shapeType)
        {
            return m_builderShape_Tube->getBuiltShape(shapeId);
        }
        else if ("Pcon" == shapeType)
        {
            return m_builderShape_Pcon->getBuiltShape(shapeId);
        } 
        else if ("Cons" == shapeType)
        {
            return m_builderShape_Cons->getBuiltShape(shapeId);
        } 
        else if ("Para" == shapeType)
        {
            return m_builderShape_Para->getBuiltShape(shapeId);
        } 
        else if ("Pgon" == shapeType)
        {
            return m_builderShape_Pgon->getBuiltShape(shapeId);
        } 
        else if ("Trap" == shapeType)
        {
            return m_builderShape_Trap->getBuiltShape(shapeId);
        } 
        else if ("Trd" == shapeType)
        {
            return m_builderShape_Trd->getBuiltShape(shapeId);
        } 
        else if ("Tubs" == shapeType)
        {
            return m_builderShape_Tubs->getBuiltShape(shapeId);
        } 
        else if ("Torus" == shapeType)
        {
            return m_builderShape_Torus->getBuiltShape(shapeId);
        } 
        else if ("TwistedTrap" == shapeType)
        {
            return m_builderShape_TwistedTrap->getBuiltShape(shapeId);
        } 
        else if ("SimplePolygonBrep" == shapeType)
        {
            return m_builderShape_SimplePolygonBrep->getBuiltShape(shapeId);
        } 
        else if ("GenericTrap" == shapeType)
        {
            return m_builderShape_GenericTrap->getBuiltShape(shapeId);
        } 
        else if ("UnidentifiedShape" == shapeType)
        {
            return m_builderShape_UnidentifiedShape->getBuiltShape(shapeId);
        } 
        else if ("Shift" == shapeType)
        {
            return m_memMapShapes_Shift[shapeId];
        } 
        else if ("Intersection" == shapeType)
        {
            return m_memMapShapes_Intersection[shapeId];
        } 
        else if ("Subtraction" == shapeType)
        {
            return m_memMapShapes_Subtraction[shapeId];
        } 
        else if ("Union" == shapeType)
        {
            return m_memMapShapes_Union[shapeId];
        } 
        else {
            THROW_EXCEPTION("ERROR!!! Shape '" + std::string(shapeType) + "' is not handled correctly!");
        }
    }
    std::cout << "WARNING! 'getBuiltShape' - For the shape '" << shapeType << "' we're using the old DB schema..." << std::endl;
    return m_memMapShapes[shapeId]; // this is a map, and 'id' is the key
}

// --- methods for caching GeoLogVol nodes ---
bool ReadGeoModel::isBuiltLog(const unsigned int id) {
    return (id <=
            m_memMapLogVols.size());  // vector: we exploit the fact that we
                                      // built the vols ordered by their IDs
}
void ReadGeoModel::storeBuiltLog(GeoLogVol* nodePtr) {
    m_memMapLogVols.push_back(nodePtr);  // vector: we exploit the fact that we
                                         // built the vols ordered by their IDs
}
GeoLogVol* ReadGeoModel::getBuiltLog(const unsigned int id) {
    return m_memMapLogVols[id - 1];  // nodes' IDs start from 1
}

// --- methods for caching GeoPhysVol nodes ---
bool ReadGeoModel::isBuiltPhysVol(const unsigned int id) {
    return (id <=
            m_memMapPhysVols.size());  // vector: we exploit the fact that we
                                       // built the vols ordered by their IDs
}
void ReadGeoModel::storeBuiltPhysVol(GeoPhysVol* nodePtr) {
    m_memMapPhysVols.push_back(nodePtr);  // vector: we exploit the fact that we
                                          // built the vols ordered by their IDs
}
GeoPhysVol* ReadGeoModel::getBuiltPhysVol(const unsigned int id) {
    return m_memMapPhysVols[id - 1];  // nodes' IDs start from 1
}

// --- methods for caching GeoFullPhysVol nodes ---
bool ReadGeoModel::isBuiltFullPhysVol(const unsigned int id) {
    return (
        id <=
        m_memMapFullPhysVols.size());  // vector: we exploit the fact that we
                                       // built the vols ordered by their IDs
}
void ReadGeoModel::storeBuiltFullPhysVol(GeoFullPhysVol* nodePtr) {
    m_memMapFullPhysVols.push_back(
        nodePtr);  // vector, we store them in the order of IDs
}
GeoFullPhysVol* ReadGeoModel::getBuiltFullPhysVol(const unsigned int id) {
    return m_memMapFullPhysVols[id - 1];  // nodes' IDs start from 1
}

// --- methods for caching GeoMaterial nodes ---
bool ReadGeoModel::isBuiltMaterial(const unsigned int id) {
    return (id <= m_memMapMaterials.size());
}
void ReadGeoModel::storeBuiltMaterial(GeoMaterial* nodePtr) {
    m_memMapMaterials.push_back(nodePtr);  // we store them in the order of IDs
}
GeoMaterial* ReadGeoModel::getBuiltMaterial(const unsigned int id) {
    return m_memMapMaterials[id - 1];
}

// --- methods for caching GeoElement nodes ---
bool ReadGeoModel::isBuiltElement(const unsigned int id) {
    return (id <= m_memMapElements.size());
}
void ReadGeoModel::storeBuiltElement(GeoElement* nodePtr) {
    m_memMapElements.push_back(
        nodePtr);  // vector, we store them in the order of IDs
}
GeoElement* ReadGeoModel::getBuiltElement(const unsigned int id) {
    return m_memMapElements[id - 1];
}

// --- methods for caching GeoTransform nodes ---
bool ReadGeoModel::isBuiltTransform(const unsigned int id) {
    return (id <=
            m_memMapTransforms.size());  // vector: we exploit the fact that we
                                         // built the vols ordered by their IDs
}
void ReadGeoModel::storeBuiltTransform(GeoTransform* nodePtr) {
    m_memMapTransforms.push_back(
        nodePtr);  // vector, we store them in the order of IDs
}
GeoTransform* ReadGeoModel::getBuiltTransform(const unsigned int id) {
    return m_memMapTransforms[id - 1];  // nodes' IDs start from 1
}

// --- methods for caching GeoAlignableTransform nodes ---
bool ReadGeoModel::isBuiltAlignableTransform(const unsigned int id) {
    return (id <= m_memMapAlignableTransforms
                      .size());  // vector: we exploit the fact that we built
                                 // the vols ordered by their IDs
}
void ReadGeoModel::storeBuiltAlignableTransform(
    GeoAlignableTransform* nodePtr) {
    m_memMapAlignableTransforms.push_back(
        nodePtr);  // vector, we store them in the order of IDs
}
GeoAlignableTransform* ReadGeoModel::getBuiltAlignableTransform(
    const unsigned int id) {
    return m_memMapAlignableTransforms[id - 1];  // nodes' IDs start from 1
}

// --- methods for caching GeoSerialDenominator nodes ---
bool ReadGeoModel::isBuiltSerialDenominator(
    const unsigned int id)  // TODO: not used at the moment, implemnt the use of
                            // this method!! and check all the others too...!!
{
    return (id <= m_memMapSerialDenominators
                      .size());  // vector: we exploit the fact that we built
                                 // the vols ordered by their IDs
}
void ReadGeoModel::storeBuiltSerialDenominator(GeoSerialDenominator* nodePtr) {
    m_memMapSerialDenominators.push_back(
        nodePtr);  // vector, we store them in the order of IDs
}
GeoSerialDenominator* ReadGeoModel::getBuiltSerialDenominator(
    const unsigned int id) {
    return m_memMapSerialDenominators[id - 1];  // nodes' IDs start from 1
}

// --- methods for caching GeoSerialIdentifier nodes ---
bool ReadGeoModel::isBuiltSerialIdentifier(const unsigned int id) {
    return (id <= m_memMapSerialIdentifiers
                      .size());  // vector: we exploit the fact that we built
                                 // the vols ordered by their IDs
}
void ReadGeoModel::storeBuiltSerialIdentifier(GeoSerialIdentifier* nodePtr) {
    m_memMapSerialIdentifiers.push_back(
        nodePtr);  // vector, we store them in the order of IDs
}
GeoSerialIdentifier* ReadGeoModel::getBuiltSerialIdentifier(
    const unsigned int id) {
    return m_memMapSerialIdentifiers[id - 1];  // nodes' IDs start from 1
}

// --- methods for caching GeoIdentifierTag nodes ---
bool ReadGeoModel::isBuiltIdentifierTag(const unsigned int id) {
    return (
        id <=
        m_memMapIdentifierTags.size());  // vector: we exploit the fact that we
                                         // built the vols ordered by their IDs
}
void ReadGeoModel::storeBuiltIdentifierTag(GeoIdentifierTag* nodePtr) {
    m_memMapIdentifierTags.push_back(
        nodePtr);  // vector, we store them in the order of IDs
}
GeoIdentifierTag* ReadGeoModel::getBuiltIdentifierTag(const unsigned int id) {
    if (0 == m_memMapIdentifierTags.size())
        std::cout
            << "WARNING!!! vector is empty! A crash is on its way..."
            << std::endl;  // TODO: make this check for all get methods, to
                           // catch the situation when a new GeoModel class is
                           // added but no buildAllXXX method is called.
    return m_memMapIdentifierTags[id - 1];  // nodes' IDs start from 1
}

// --- methods for caching GeoNameTag nodes ---
bool ReadGeoModel::isBuiltNameTag(const unsigned int id) {
    return (id <=
            m_memMapNameTags.size());  // vector: we exploit the fact that we
                                       // built the vols ordered by their IDs
}
void ReadGeoModel::storeBuiltNameTag(GeoNameTag* nodePtr) {
    m_memMapNameTags.push_back(
        nodePtr);  // vector, we store them in the order of IDs
}
GeoNameTag* ReadGeoModel::getBuiltNameTag(const unsigned int id) {
    return m_memMapNameTags[id - 1];  // vector, but nodes' IDs start from 1
}

// --- methods for caching GeoSerialTransformer nodes ---
bool ReadGeoModel::isBuiltSerialTransformer(const unsigned int id) {
    return (id <= m_memMapSerialTransformers
                      .size());  // vector: we exploit the fact that we built
                                 // the vols ordered by their IDs
}
// void ReadGeoModel::storeBuiltSerialTransformer(const unsigned int id,
// GeoSerialTransformer* nodePtr)
void ReadGeoModel::storeBuiltSerialTransformer(GeoSerialTransformer* nodePtr) {
    m_memMapSerialTransformers.push_back(
        nodePtr);  // vector, we store them in the order of IDs
}
GeoSerialTransformer* ReadGeoModel::getBuiltSerialTransformer(
    const unsigned int id) {
    return m_memMapSerialTransformers[id - 1];  // remember: nodes' IDs
                                                // start from 1
}
/* FIXME:
// --- methods for caching Functions nodes ---
bool ReadGeoModel::isBuiltFunction(const unsigned int id)
{
  return (id <= m_memMapFunctions.size()); // vector: we exploit the fact that
we built the vols ordered by their IDs
}
void ReadGeoModel::storeBuiltFunction(const unsigned int id, GeoXF::Function*
nodePtr)
{
  m_memMapFunctions.push_back(nodePtr); // vector, we store them in the order of
IDs

}
GeoXF::Function* ReadGeoModel::getBuiltFunction(const unsigned int id)
{
  return m_memMapFunctions[id-1]; // remember: nodes' IDs start from 1
}
*/

// --- methods for caching GeoPhysVol/GeoFullPhysVol nodes ---
// std::string getVPhysVolKey(const unsigned int id, const unsigned int tableId,
// const unsigned int copyNumber)
//{
// std::string key = std::to_string(id) + ":" + std::to_string(tableId) + ":" +
// std::to_string(copyNumber); return key;
//}
std::string getVPhysVolKey(const unsigned int id, const unsigned int tableId) {
    std::string key = std::to_string(id) + ":" + std::to_string(tableId);
    return key;
}
// void ReadGeoModel::storeVPhysVol(const unsigned int id, const unsigned int
// tableId, const unsigned int copyN, GeoGraphNode* nodePtr)
void ReadGeoModel::storeVPhysVol(const unsigned int id,
                                 const unsigned int tableId,
                                 GeoGraphNode* nodePtr) {
    std::lock_guard<std::mutex> lk(muxVPhysVol);
    // std::string key = getVPhysVolKey(id, tableId, copyN);
    std::string key = getVPhysVolKey(id, tableId);
    m_memMap[key] = nodePtr;
}
// GeoGraphNode* ReadGeoModel::getVPhysVol(const unsigned int id, const unsigned
// int tableId, const unsigned int copyN)
GeoGraphNode* ReadGeoModel::getVPhysVol(const unsigned int id,
                                        const unsigned int tableId) {
    std::lock_guard<std::mutex> lk(muxVPhysVol);
    // std::string key = getVPhysVolKey(id, tableId, copyN);
    std::string key = getVPhysVolKey(id, tableId);
    if (m_memMap.find(key) == m_memMap.end()) {
        return nullptr;  // if volume is not found in cache
    }
    return m_memMap[key];
}

} /* namespace GeoModelIO */

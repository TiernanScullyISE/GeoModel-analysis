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
#include "BuildGeoShapes_Shift.h"
#include "BuildGeoShapes_Union.h"
#include "BuildGeoShapes_Subtraction.h"
#include "BuildGeoShapes_Intersect.h"

#include "BuildGeoSurface_Trapezoid.h"
#include "BuildGeoSurface_Rectangular.h"
#include "BuildGeoSurface_Diamond.h"
#include "BuildGeoSurface_Annulus.h"

#include "GeoModelRead/ReadGeoModel.h"

// TFPersistification includes
#include "TFPersistification/TransFunctionInterpreter.h"

// GeoModelKernel includes
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoSerialDenominator.h"
#include "GeoModelKernel/GeoSerialIdentifier.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelHelpers/TransformToStringConverter.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoVSurface.h"


// GeoModel shapes
#include "GeoModelHelpers/variantHelpers.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelHelpers/StringUtils.h"
#include "GeoModelHelpers/printVolume.h"
#include "GeoModelHelpers/ThreadPool.h"

// Units
#include "GeoModelKernel/Units.h"
// VP1Base
//  TODO: we should get rid of VP1Base::VP1Msg dependency, since GeoModelRead
//  should not depend on VP1 packages. Maybe we can move VP1Msg to a standalone
//  package.
// #include "VP1Base/VP1Msg.h"

// C++ includes
#include <cstdlib> /* exit, EXIT_FAILURE */

#include <chrono>  /* system_clock */
#include <ctime>   /* std::time */

#include <memory>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <format>

// mutexes for synchronized access to containers and output streams in
// multi-threading mode
std::mutex muxCout;

using namespace GeoGenfun;
using namespace GeoXF;

namespace {
    constexpr std::size_t objectBatch = 5000;

    template <typename NodeType_t>
    void addGraphNode(const PVLink& appendTo, const GeoIntrusivePtr<NodeType_t>& addMe){
        if (!appendTo) {
            THROW_EXCEPTION("The volume to add is a nullptr");
        }
        if (!addMe) {
            THROW_EXCEPTION("The object of type "<<typeid(NodeType_t).name()<<" is a nullptr. ");
        }
        appendTo->add(addMe);
    }
    using TimePoint_t = std::chrono::system_clock::time_point;

    struct NoDeleter{
        void operator()(GMDBManager*){}
    };

    using UnpackData_t = GeoModelHelpers::variantHelper;
    
}

#define PRINT_MSG(THE_MSG) \
    {                      \
        std::lock_guard guard{muxCout}; \
        std::cout<<__func__<<"() - :"<<__LINE__<<" "<<THE_MSG<<std::endl; \
    }


namespace GeoModelIO {

ReadGeoModel::ReadGeoModel(GMDBManager* db):
    ReadGeoModel{std::shared_ptr<GMDBManager>(db, NoDeleter{})} {}
ReadGeoModel::ReadGeoModel(std::shared_ptr<GMDBManager> db):
    m_dbManager{db} {
    // Check if the user asked for debug messages
    if (GeoStrUtils::getEnvVar("GEOMODEL_ENV_IO_LOGLEVEL_1").size()) {
        m_loglevel = 1;
        PRINT_MSG("You defined the GEOMODEL_ENV_IO_DEBUG variable, so you will see a verbose output.");
    }
    // Check if the user asked for verbose debug messages
    if (GeoStrUtils::getEnvVar("GEOMODEL_ENV_IO_LOGLEVEL_2").size()) {
        m_loglevel = 2;
        PRINT_MSG("You defined the GEOMODEL_ENV_IO_READ_DEBUG_VERBOSE variable, so you will see a verbose output.");
    }
    // Check if the user asked for timing output
    if (GeoStrUtils::getEnvVar("GEOMODEL_ENV_IO_READ_TIMING").size()) {
        m_timing = true;
        PRINT_MSG("You defined the GEOMODEL_ENV_IO_READ_TIMING variable, so you will see a timing measurement in the output.");
    }
}
GMDBManager& ReadGeoModel::dbManager() const {
    assert(m_dbManager);
    return *m_dbManager;
}
std::shared_ptr<GMDBManager> ReadGeoModel::dbManagerPtr() const{ return m_dbManager; }


PVConstLink ReadGeoModel::buildGeoModel() {
    if (m_loglevel >= 2){
        PRINT_MSG("ReadGeoModel::buildGeoModel()");
    }
    if (!m_world) {
        m_world = assembleWorld();
    }
    return m_world;
}


template <typename ShapeFactory_t> 
    void ReadGeoModel::setupShapeFactory(const std::string& shapeType,
                                         const std::string& primaryDataTable) {
    DBRowsList buildMe = m_dbManager->getTableFromNodeType_VecVecData(primaryDataTable);
    auto shapeBuilder = std::make_unique<ShapeFactory_t>(std::move(buildMe));
    if (!m_shapeFactories.insert(std::make_pair(shapeType, std::move(shapeBuilder))).second){
        THROW_EXCEPTION("Failed to register factory "<<shapeType<<" of type "<<typeid(ShapeFactory_t).name());
    }    
}
template <typename ShapeFactory_t> 
    void ReadGeoModel::setupBooleanFactory(const std::string& shapeType,
                                           const std::string& primaryDataTable) {
    DBRowsList buildMe = m_dbManager->getTableFromNodeType_VecVecData(primaryDataTable);
    auto shapeBuilder = std::make_unique<ShapeFactory_t>(this, std::move(buildMe));
    if (!m_shapeFactories.insert(std::make_pair(shapeType, std::move(shapeBuilder))).second){
        THROW_EXCEPTION("Failed to register factory "<<shapeType<<" of type "<<typeid(ShapeFactory_t).name());
    } 
}

template <typename ShapeFactory_t> 
    void ReadGeoModel::setupShapeFactory(const std::string& shapeType,
                                         const std::string& primaryDataTable,
                                         const std::string& auxilllaryDataTable) {
    DBRowsList primary = m_dbManager->getTableFromNodeType_VecVecData(primaryDataTable);
    DBRowsList auxillary = m_dbManager->getTableFromTableName_VecVecData(auxilllaryDataTable);
    auto shapeBuilder = std::make_unique<ShapeFactory_t>(std::move(primary), std::move(auxillary));
    if (!m_shapeFactories.insert(std::make_pair(shapeType, std::move(shapeBuilder))).second){
        THROW_EXCEPTION("Failed to register factory "<<shapeType<<" of type "<<typeid(ShapeFactory_t).name());
    }   
}

template <typename SurfaceFactory_t> 
    void ReadGeoModel::setupSurfaceFactory(const std::string& surfaceType,
                                           const std::string& dataTable) {
        DBRowsList primary = m_dbManager->getTableFromNodeType_VecVecData(dataTable);
        auto surfaceBuilder = std::make_unique<SurfaceFactory_t>(std::move(primary));
        if (!m_surfaceFactories.insert(std::make_pair(surfaceType, std::move(surfaceBuilder))).second){
           THROW_EXCEPTION("Failed to register factory "<<surfaceType<<" of type "<<typeid(SurfaceFactory_t).name());
        }
    }

void ReadGeoModel::loadDB() {
    // *** get all data from the DB ***
    // timing: get start time
    TimePoint_t start = std::chrono::system_clock::now();  
   
    setupShapeFactory<BuildGeoShapes_Box>("Box", "GeoBox");
    setupShapeFactory<BuildGeoShapes_EllipticalTube>("EllipticalTube", "GeoEllipticalTube");
    setupShapeFactory<BuildGeoShapes_Tube>("Tube", "GeoTube");
    setupShapeFactory<BuildGeoShapes_Cons>("Cons", "GeoCons");
    setupShapeFactory<BuildGeoShapes_Para>("Para", "GeoPara");
    setupShapeFactory<BuildGeoShapes_Trap>("Trap", "GeoTrap");
    setupShapeFactory<BuildGeoShapes_Trd>("Trd", "GeoTrd");
    setupShapeFactory<BuildGeoShapes_Tubs>("Tubs", "GeoTubs");
    setupShapeFactory<BuildGeoShapes_Torus>("Torus", "GeoTorus");
    setupShapeFactory<BuildGeoShapes_TwistedTrap>("TwistedTrap", "GeoTwistedTrap");
    setupShapeFactory<BuildGeoShapes_UnidentifiedShape>("UnidentifiedShape", "GeoUnidentifiedShape");
    
    setupShapeFactory<BuildGeoShapes_Pcon>("Pcon", "GeoPcon", "Shapes_Pcon_Data");
    setupShapeFactory<BuildGeoShapes_Pgon>("Pgon", "GeoPgon", "Shapes_Pgon_Data");
    setupShapeFactory<BuildGeoShapes_SimplePolygonBrep>("SimplePolygonBrep", "GeoSimplePolygonBrep", 
                                                        "Shapes_SimplePolygonBrep_Data");
    setupShapeFactory<BuildGeoShapes_GenericTrap>("GenericTrap", "GeoGenericTrap",
                                                  "Shapes_GenericTrap_Data");

    setupBooleanFactory<BuildGeoShapes_Shift>("Shift", "GeoShapeShift");
    setupBooleanFactory<BuildGeoShapes_Union>("Union", "GeoShapeUnion");    
    setupBooleanFactory<BuildGeoShapes_Subtraction>("Subtraction", "GeoShapeSubtraction");
    setupBooleanFactory<BuildGeoShapes_Intersect>("Intersection", "GeoShapeIntersection");
  
    setupSurfaceFactory<BuildGeoSurface_Trapezoid>("TrapezoidSurface", "TrapezoidSurface");
    setupSurfaceFactory<BuildGeoSurface_Rectangular>("RectangleSurface", "RectangleSurface");
    setupSurfaceFactory<BuildGeoSurface_Diamond>("DiamondSurface", "DiamondSurface");
    setupSurfaceFactory<BuildGeoSurface_Annulus>("AnnulusSurface", "AnnulusSurface");
    
    // get the root volume data
    m_root_vol_data = m_dbManager->getRootPhysVol();
    // get DB metadata
    m_tableID_toTableName = m_dbManager->getAll_TableIDsNodeTypes();
    m_tableName_toTableID = m_dbManager->getAll_NodeTypesTableIDs();

    TimePoint_t end = std::chrono::system_clock::now();  // timing: get end time
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    if (m_timing || (m_loglevel >= 1)) {
        PRINT_MSG("*** Time taken to fetch GeoModel data from the database: " << diff << " [s]")
    }
}

PVLink ReadGeoModel::assembleWorld() {
    if (m_dbManager && m_dbManager->checkIsDBOpen()) {
        if (m_loglevel >= 1) {
            PRINT_MSG( "OK! Database is open!");
        } 
    } else {
        THROW_EXCEPTION("ERROR!! Database is NOT open!");
    }
    // build caches
    m_dbManager->loadGeoNodeTypesAndBuildCache();
    m_dbManager->createTableDataCaches();
    // load the data from the DB
    loadDB();

    // *** build all nodes ***
    TimePoint_t start = std::chrono::system_clock::now();  // timing: get start time

    if (m_loglevel >= 1) {
        PRINT_MSG("Building nodes serially...");
    }
    
    auto& pool = GeoThreading::ThreadPool::getPool(-1); 
    pool.appendTask([this](){buildAllElements(); });

    pool.appendTask([this](){ buildAllSerialDenominators(); });
    pool.appendTask([this](){ buildAllSerialIdentifiers(); });
    pool.appendTask([this](){ buildAllIdentifierTags(); });
    pool.appendTask([this](){ buildAllNameTags(); });

    pool.appendTask([this](){ buildAllVSurfaces(); });
    pool.appendTask([this](){ buildAllGeoFunc(); });

    buildAllTransforms();
    buildAllAlignableTransforms();

    buildAllMaterials();
    
    buildAllLogVols();
    ///
    buildAllPhysVols();
    buildAllFullPhysVols();

    /// Logical volumes no longer needed...
    if (m_autoClean) {
        m_memMapLogVols.clear();
    }
    /// Build the functions to construct the serial transformers

    buildAllSerialTransformers();
    
    TimePoint_t end = std::chrono::system_clock::now();  // timing: get end time
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    if (m_timing || (m_loglevel >= 1)) {
       PRINT_MSG("*** Time taken to build all GeoModel nodes: " << diff << " [s]");
    }

    // *** recreate all mother-daughter relatioships between nodes ***
    start = std::chrono::system_clock::now();  // timing: get start time
    connectNodes();
    end = std::chrono::system_clock::now();  // timing: get end time
    diff = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    if (m_timing || (m_loglevel >= 1)) {
       PRINT_MSG("*** Time taken to recreate all mother-daughter relationships between nodes of the GeoModel tree: "
                  << diff << " [s]");
    }
    /// Fetch the world because all other pointers are now going to be cleared
    PVLink world = getRootVolume();
    /// We can clean up the memory here quite a bit
  
    if (m_autoClean) {
        m_memMapPhysVols.clear();
        m_memMapTransforms.clear();
        m_memMapSerialDenominators.clear();
        m_memMapSerialIdentifiers.clear();
        m_memMapIdentifierTags.clear();
        m_memMapNameTags.clear();
        m_memMapVSurface.clear();
        m_memMapSerialTransformers.clear();
        m_memMapFunctions.clear();
    }
    pool.closePool();
    /// What remains are the full physcal volumes and alignable transforms
    return world;
}
//! Iterate over the list of GeoSerialDenominator nodes, build them all, and
//! store their pointers
void ReadGeoModel::buildAllSerialDenominators() {
    if (m_loglevel >= 1){
        PRINT_MSG("Building all SerialDenominator nodes...");
    }
    const DBRowsList serialDenoms = m_dbManager->getTableFromNodeType_VecVecData("GeoSerialDenominator");
    for (const DBRowEntry& values : serialDenoms) {
        assert(values.size() >= 2);
        const unsigned int nodeID = UnpackData_t::getFromVariant_Int(values[0], "GeoSerialDenominator::nodeID");
        // // RMB: not used at the moment, commented to avoid warnings
        const std::string& baseName = UnpackData_t::getFromVariant_String(values[1], "GeoSerialDenominator::baseName");
        auto nodePtr = make_intrusive<GeoSerialDenominator>(baseName);
        if(!m_memMapSerialDenominators.insert(std::make_pair(nodeID, std::move(nodePtr)))){
            THROW_EXCEPTION("Failed to register "<<baseName<<" under ID: "<<nodeID);
        }
    }
    if (m_memMapSerialDenominators.size()) {
        std::cout << "All " << m_memMapSerialDenominators.size() << " SerialDenominators have been built!\n";
    }
}

//! Iterate over the list of GeoSerialIdentifier nodes, build them all, and
//! store their pointers
void ReadGeoModel::buildAllSerialIdentifiers() {
    if (m_loglevel >= 1) {
        PRINT_MSG("Building all SerialIdentifier nodes...");
    }
    const DBRowsList serialsIds = m_dbManager->getTableFromNodeType_VecVecData("GeoSerialIdentifier");
    for (const auto& serialValues : serialsIds) {
        assert(serialValues.size() >= 2);
        const unsigned int nodeID = UnpackData_t::getFromVariant_Int(serialValues[0], "GeoSerialIdentifier:nodeID");
        // // RMB: not used at the moment, commented to avoid warnings
        const int baseId = UnpackData_t::getFromVariant_Int(serialValues[1], "GeoSerialIdentifier::baseID");
        auto nodePtr = make_intrusive<GeoSerialIdentifier>(baseId);
        if (!m_memMapSerialIdentifiers.insert(std::make_pair(nodeID, nodePtr))){
            THROW_EXCEPTION("Failed to register serial Identifier: "<<baseId<<" under ID: "<<nodeID);
        }
    }
    if (m_memMapSerialIdentifiers.size()) {
        std::cout << "All " << m_memMapSerialIdentifiers.size() << " SerialIdentifiers have been built!\n";
    }
}

//! Iterate over the list of GeoIdentifierTag nodes, build them all, and
//! store their pointers
void ReadGeoModel::buildAllIdentifierTags() {
    if (m_loglevel >= 1) {
        PRINT_MSG("Building all IdentifierTag nodes...");
    }
    const DBRowsList idTags{m_dbManager->getTableFromNodeType_VecVecData("GeoIdentifierTag")};
    for (const auto& idValues : idTags) {
        assert(idValues.size() >=2);
        const unsigned int nodeID = UnpackData_t::getFromVariant_Int(idValues[0], "GeoIdentifierTag::nodeID");
        const int identifier      = UnpackData_t::getFromVariant_Int(idValues[1], "GeoIdentifierTag::identifier");
        auto nodePtr = make_intrusive<GeoIdentifierTag>(identifier);
        if (!m_memMapIdentifierTags.insert(std::make_pair(nodeID, nodePtr))) {
            THROW_EXCEPTION("Failed to register identifier "<<identifier<<" under: "<<nodeID);
        }
    }
    if (m_memMapIdentifierTags.size()){
        std::cout << "All " << m_memMapIdentifierTags.size() << " SerialIdentifiers have been built!\n";
    }
}

//! Iterate over the list of NameTag nodes, build them all, and store their
//! pointers
void ReadGeoModel::buildAllNameTags() {
    if (m_loglevel >= 1) {
        PRINT_MSG("Building all NameTag nodes...");
    }
    const DBRowsList nameTagDB = m_dbManager->getTableFromNodeType_VecVecData("GeoNameTag");
    for (const auto& entry : nameTagDB) {
        assert(entry.size() >= 2);
        const unsigned int nodeID = UnpackData_t::getFromVariant_Int(entry[0], "GeoNameTag::nodeID");
        const std::string& baseName = UnpackData_t::getFromVariant_String(entry[1], "GeoNameTag::baseName");
        auto nodePtr = make_intrusive<GeoNameTag>(baseName);
        if (!m_memMapNameTags.insert(std::make_pair(nodeID, nodePtr))) {
            THROW_EXCEPTION("Failed to regitert name tag "<<baseName<<" under ID: "<<nodeID);
        }
    }
    if (m_memMapNameTags.size()){
        std::cout << "All " << m_memMapNameTags.size() << " NameTags have been built!\n";
    }
}

//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllElements() {
    if (m_loglevel >= 1) {
        PRINT_MSG("Building all Elements... ");
    }
    const DBRowsList elements = m_dbManager->getTableFromNodeType_VecVecData("GeoElement");
    for (const DBRowEntry& values : elements) {
        const unsigned elId = UnpackData_t::getFromVariant_Int(values[0], "Element:ID");
        std::string elName = UnpackData_t::getFromVariant_String(values[1], "Element:name");
        std::string elSymbol = UnpackData_t::getFromVariant_String(values[2], "Element:symbol");
        double elZ = UnpackData_t::getFromVariant_Double(values[3], "Element:Z");
        double elA = UnpackData_t::getFromVariant_Double(values[4], "Element:A");
        if (m_loglevel >= 2) {
            PRINT_MSG("\tElement - ID:" << elId << ", name:" << elName
                    << ", symbol:" << elSymbol << ", Z:" << elZ << ", A:" << elA
                    << " ( " << elA / (GeoModelKernelUnits::g / GeoModelKernelUnits::mole)
                    << "[g/mole] )");
        }
        auto newElem = make_intrusive<GeoElement>(elName, elSymbol, elZ, elA);
        if (!m_memMapElements.insert(std::make_pair(elId, std::move(newElem)))) {
            THROW_EXCEPTION("Failed to register new element with ID: "<<elId);
        }
    }
    if (!m_memMapElements.empty()) {
        std::cout << "All " << m_memMapElements.size() << " Elements have been built!\n";
    }
}

//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllMaterials() {
    if (m_loglevel >= 1) {
        PRINT_MSG("Building all Materials...");
    }
    const DBRowsList materials = m_dbManager->getTableFromNodeType_VecVecData("GeoMaterial");
    const DBRowsList materials_Data = m_dbManager->getTableFromTableName_VecVecData("Materials_Data");
    for (const DBRowEntry& values : materials) {
         const unsigned int matId = UnpackData_t::getFromVariant_Int(values[0], "Material:id");
         const std::string matName = UnpackData_t::getFromVariant_String(values[1], "Material:matName");
         const double matDensity = UnpackData_t::getFromVariant_Double(values[2], "Material:matDensity");
         const unsigned dataStart = UnpackData_t::getFromVariant_Int(values[3], "Material:dataStart");
         const unsigned dataEnd = UnpackData_t::getFromVariant_Int(values[4], "Material:dataEnd");

        if (m_loglevel >= 3) {
            PRINT_MSG("\tbuildMaterial() : Material - ID:" << matId << ", name:" << matName
                      << ", density:" << matDensity << " ( "<< matDensity / (GeoModelKernelUnits::g / GeoModelKernelUnits::cm3)
                      << "[g/cm3] )"<< ", elements: dataStart: " << dataStart << ", dataEnd: " << dataEnd);
        }
        auto mat = make_intrusive<GeoMaterial>(matName, matDensity);
        assert(dataStart > 0 && dataEnd <= materials_Data.size());
        DBRowsList matElements{materials_Data.begin() + (dataStart-1),
                               materials_Data.begin() + (dataEnd)};

        /// Assign the elements
        for (const DBRowEntry& matRow : matElements) {
            const unsigned elId = UnpackData_t::getFromVariant_Int(matRow[1], "MatElement:id");
            double elFraction = UnpackData_t::getFromVariant_Double(matRow[2], "MatElement:fraction");
            GeoIntrusivePtr<GeoElement> el = getBuiltElement(elId);
            assert(el);
            mat->add(el, elFraction);
        }
        mat->lock();
        if (!m_memMapMaterials.insert(std::make_pair(matId, std::move(mat)))){
            THROW_EXCEPTION("Failed to register material ID: "<<matId);
        }
    }
    if (!m_memMapMaterials.empty()){
        std::cout << "All " << m_memMapMaterials.size() << " Materials have been built!\n";
    }
    /// We don't need any longer the element map
    if (m_autoClean) {
        m_memMapElements.clear();
    }
}

//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllVSurfaces() {
    if (m_loglevel >= 1) {
        PRINT_MSG("Building all Virtual Surfaces...");
    }
    const DBRowsList vSurfaces = m_dbManager->getTableFromNodeType_VecVecData("GeoVSurface");
    for (const DBRowEntry& surfaceEntry : vSurfaces) {
        const unsigned surfId = UnpackData_t::getFromVariant_Int(surfaceEntry[0], "surfaceID");
        if (m_loglevel >= 3) {
           PRINT_MSG("buildVSurface(), testing VSurface id: " << surfId << "...");
        }
        const std::string shapeType = UnpackData_t::getFromVariant_String(surfaceEntry[1], "VSurf_shapeType");
        const unsigned int shapeId = UnpackData_t::getFromVariant_Int(surfaceEntry[2], "VSurf_shapeID");
        
        const auto factory_itr = m_surfaceFactories.find(shapeType);
        if (factory_itr == m_surfaceFactories.end()){
            THROW_EXCEPTION("Invalid surface shape type "<<shapeType);
        }
        const auto surfShape = factory_itr->second->getSurface(shapeId);
        if (!surfShape) {
            THROW_EXCEPTION("Surface of type "<<shapeType<<" has no member with ID: "<<shapeId);
        }
        auto buildVSurf = make_intrusive<GeoVSurface>(surfShape);
        if (!m_memMapVSurface.insert(std::make_pair(surfId, std::move(buildVSurf)))) {
            THROW_EXCEPTION("Failed to register virtual surface with ID: "<<surfId);
        }
    }
    if (!m_memMapVSurface.empty()){
        std::cout << "All " << m_memMapVSurface.size() << " Virtual Surface have been built!\n";
    }
    /// Surface factories are no longer needed and can be freed.
    if (m_autoClean) {
        m_surfaceFactories.clear();
    }
}

//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllLogVols() {
    if (m_loglevel >= 1) {
        PRINT_MSG("Building all LogVols...");
    }
    const DBRowsList logVols = m_dbManager->getTableFromNodeType_VecVecData("GeoLogVol");
    auto& pool{GeoThreading::ThreadPool::getPool()};
    for (std::size_t start = 0 ; start< logVols.size(); ) {
        const std::size_t end = std::min(start+objectBatch, logVols.size());
        pool.appendTask([this, start,end,&logVols]{
            for (std::size_t itr = start; itr<end; ++itr){
                const DBRowEntry& values{logVols[itr]};
                // --- get the parameters to build the GeoLogVol node
                const unsigned id = UnpackData_t::getFromVariant_Int(values[0], "LogVol_id");
                // get the name of the LogVol
                const std::string logVolName = UnpackData_t::getFromVariant_String(values[1], "LogVol_name");
                // get the ID and the type of the referenced GeoShape node
                const int shapeId = UnpackData_t::getFromVariant_Int(values[2], "LogVol_shapeID");
                const std::string shapeType = UnpackData_t::getFromVariant_String(values[3], "LogVol_shapeType");
                GeoIntrusivePtr<GeoShape> shape = getBuiltShape(shapeId, shapeType);
                if (!shape) {
                    THROW_EXCEPTION("ERROR!! While building the LogVol '"<<logVolName<< "', shape of type '" 
                                    <<shapeType<<"' is NULL! Exiting...");
                }
                // build the referenced GeoMaterial node
                // const unsigned int matId = std::stoi(values[3]);
                const int matId = UnpackData_t::getFromVariant_Int(values[4], "LogVol_MaterialID");

                if (m_loglevel >= 3) {
                   PRINT_MSG("Material Id:"<<matId);
                }
                GeoIntrusivePtr<GeoMaterial> mat = getBuiltMaterial(matId);
                if (!mat) {
                    THROW_EXCEPTION("ERROR!! While building the LogVol '"<<logVolName + "', Material of ID '" <<matId<<"' is NULL! Exiting...");
                }
                auto logPtr = make_intrusive<GeoLogVol>(logVolName, shape, mat);
                if (m_loglevel >= 3) {
                    PRINT_MSG("Address of the stored LogVol:" << logPtr.get());
                }
                if (!m_memMapLogVols.insert(std::make_pair(id, logPtr))) {
                    THROW_EXCEPTION("Failed to register a new logical volume under ID "<<id<<" The id is already taken");
                }
            }
        });
        start = end;
    }
    pool.drainQueue();
    if (!m_memMapLogVols.empty()) {
        std::cout << "All " << m_memMapLogVols.size() << " LogVols have been built!\n";
    }
    if (m_autoClean) {
        /// We no longer need the list of materials
        m_memMapMaterials.clear();
        /// Neither we need to keep the shape factories
        m_shapeFactories.clear();
    }
}

template <typename PhysVol_t>
    void ReadGeoModel::buildPhysVols(const std::string& tableName,                          
                                     GeoIdObjMap<GeoIntrusivePtr<PhysVol_t>>& memCache) {
        const DBRowsList volRecords = m_dbManager->getTableFromNodeType_VecVecData(tableName);
        auto& pool{GeoThreading::ThreadPool::getPool()};
        for (std::size_t start =0 ; start < volRecords.size() ;) {
            const std::size_t end = std::min(start + objectBatch, volRecords.size());
            pool.appendTask([start, end,this, &volRecords, &memCache, &tableName](){
                for (std::size_t itr = start; itr < end;++itr) {
                    const DBRowEntry& volRecord{volRecords[itr]};
                    assert(volRecord.size() >= 2);
                    const unsigned volID    = UnpackData_t::getFromVariant_Int(volRecord[0], 
                                                                               std::format("{}_volID", tableName));
                    const unsigned logVolID = UnpackData_t::getFromVariant_Int(volRecord[1], 
                                                                               std::format("{}_logVolID", tableName));

                    // GET LOGVOL
                    GeoIntrusivePtr<GeoLogVol> logVol = getBuiltLog(logVolID);
                    if (!logVol) {
                        THROW_EXCEPTION("Logical volume "<<logVolID<<" is not registered");
                    }
                    if (m_loglevel >= 3) {
                        PRINT_MSG("using the cached LogVol [" << logVolID<< "] w/ address: " << logVol.get() << "...");
                    }
                    auto newVol = make_intrusive<PhysVol_t>(logVol);
                    if (!memCache.insert(std::make_pair(volID, newVol))) {
                        THROW_EXCEPTION("Failed to register volume "<<printVolume(newVol)<<" under ID: "<<volID);
                    }
                }
            });
            start = end;
        }
        pool.drainQueue();
    }
template <typename Transform_t> 
    void ReadGeoModel::buildTransforms(const std::string& tableName,
                                      GeoIdObjMap<GeoIntrusivePtr<Transform_t>>& memCache) {
    const std::vector<std::vector<std::string>> trfRecords = m_dbManager->getTableFromNodeType_String(tableName);
    auto& pool = GeoThreading::ThreadPool::getPool();
    for (std::size_t start = 0; start < trfRecords.size(); ) {
        std::size_t end = std::min(start + objectBatch, trfRecords.size());
        pool.appendTask([&trfRecords, &memCache, &tableName, start, end](){
            for (std::size_t itr = start; itr < end; ++itr) {
                const std::vector<std::string>& values{trfRecords[itr]};
                assert(values.size() >=13);
                /// The first element in the vector is the index of the transform
                // get the 12 matrix elements
                const double xx{std::stod(values[1])}, xy{std::stod(values[2])}, xz{std::stod(values[3])};
                const double yx{std::stod(values[4])}, yy{std::stod(values[5])}, yz{std::stod(values[6])};
                const double zx{std::stod(values[7])}, zy{std::stod(values[8])}, zz{std::stod(values[9])};
                /// Translational part
                const double dx{std::stod(values[10])}, dy{std::stod(values[11])}, dz{std::stod(values[12])};

                GeoTrf::Transform3D txf{GeoTrf::Transform3D::Identity()};
                // build the rotation matrix with the first 9 elements
                txf(0, 0) = xx; txf(0, 1) = xy; txf(0, 2) = xz;
                txf(1, 0) = yx; txf(1, 1) = yy; txf(1, 2) = yz;
                txf(2, 0) = zx; txf(2, 1) = zy; txf(2, 2) = zz;
                // build the translation matrix with the last 3 elements
                txf(0, 3) = dx; txf(1, 3) = dy; txf(2, 3) = dz;

                auto newTrf = make_intrusive<Transform_t>(std::move(txf));
                const unsigned id = std::stoi(values[0]);
                if (!memCache.insert(std::make_pair(id, newTrf))) {
                    THROW_EXCEPTION("Failed to insert new transform "
                                  <<GeoTrf::toString(newTrf->getTransform())<<" from table "<<tableName);
                }
            }
        });
        /// Update the iterator range
        start = end;
    }
    pool.drainQueue();
                    
}
//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllPhysVols() {
    if (m_loglevel >= 2) {
        PRINT_MSG("Building all PhysVols...");
    }
    buildPhysVols("GeoPhysVol", m_memMapPhysVols);
    if (m_memMapPhysVols.size()) {
        std::cout<<"All "<<m_memMapPhysVols.size()<<" physical volumes have been built "<<std::endl;
    }
}

//! Iterate over the list of nodes, build them all, and store their pointers
void ReadGeoModel::buildAllFullPhysVols() {
    if (m_loglevel >= 2) PRINT_MSG("Building all FullPhysVols...");
    buildPhysVols("GeoFullPhysVol", m_memMapFullPhysVols);
    if (m_memMapFullPhysVols.size()) {
        std::cout<<"All "<<m_memMapFullPhysVols.size()<<" full physical volumes have been built "<<std::endl;
    }

}

//! Iterate over the list of GeoAlignableTransforms nodes, build them all, and
//! store their pointers
void ReadGeoModel::buildAllAlignableTransforms() {
    if (m_loglevel >= 2) {
        PRINT_MSG("Building all AlignableTransforms... ");
    }
    buildTransforms("GeoAlignableTransform", m_memMapAlignableTransforms);
}
//! Iterate over the list of GeoTransforms nodes, build them all, and store
//! their pointers
void ReadGeoModel::buildAllTransforms() {
    if (m_loglevel >= 2) {
       PRINT_MSG("Building all Transforms...");
    }
    buildTransforms("GeoTransform", m_memMapTransforms);
}
void ReadGeoModel::buildAllGeoFunc(){
    if (m_loglevel >= 2) {
        PRINT_MSG("Building all GeoFunctions");
    }
    // containers to store data that have been moved to the new DB schema
    DBRowsList functions = m_dbManager->getTableFromNodeType_VecVecData("Function");

    // get the Function's expression data
    std::deque<double> funcExprData = m_dbManager->getTableFromTableName_DequeDouble("FuncExprData");

    for (const DBRowEntry& funcEntry : functions) {
        const unsigned funcId = UnpackData_t::getFromVariant_Int(funcEntry[0], "Function::id");
        std::string expr = UnpackData_t::getFromVariant_String(funcEntry[1], "Function::expression");
        if (expr.empty()) {
            THROW_EXCEPTION("FATAL ERROR!! Function expression is empty!! Aborting...");
        }

        const unsigned dataStart = UnpackData_t::getFromVariant_Int(funcEntry[2], "Function::parsStart");
        const unsigned dataEnd   = UnpackData_t::getFromVariant_Int(funcEntry[3], "Function::parsEnd");
        assert(dataStart > 0 && dataEnd <= funcExprData.size());
        std::deque<double> sub_vector(funcExprData.begin() + (dataStart-1),
                                      funcExprData.begin() + (dataEnd) );

        TransFunctionInterpreter interpreter;
        TFPTR func = interpreter.interpret(expr, &sub_vector);

        if (!m_memMapFunctions.insert(std::make_pair(funcId, std::move(func)))) {
            THROW_EXCEPTION("Failed to register "<<expr<<" under "<<funcId);
        }
    }
}
void ReadGeoModel::buildAllSerialTransformers() {
    if (m_loglevel >= 2) {
        PRINT_MSG("Building all SerialTransformers...");
    }
    DBRowsList serialTransformers = m_dbManager->getTableFromNodeType_VecVecData("GeoSerialTransformer");

    for (const auto& serialTrfEntry : serialTransformers) {
        const unsigned int serialID   = UnpackData_t::getFromVariant_Int(serialTrfEntry[0], "GeoSerialTransformer::serialID");
        const unsigned int functionId = UnpackData_t::getFromVariant_Int(serialTrfEntry[1], "GeoSerialTransformer::functionId");
        const unsigned int physVolId  = UnpackData_t::getFromVariant_Int(serialTrfEntry[2], "GeoSerialTransformer::physVolId");
        const unsigned int physVolTableId = UnpackData_t::getFromVariant_Int(serialTrfEntry[3], "GeoSerialTransformer::physVolTableId");
        const unsigned int copies = UnpackData_t::getFromVariant_Int(serialTrfEntry[4], "GeoSerialTransformer::copies");

        auto func = getBuiltFunction(functionId); 
        if (!func) {
            THROW_EXCEPTION("Failed to fetch function "<<functionId);
        }
        PVConstLink vphysVol = getVPhysVol(physVolId, physVolTableId); 
        if (!vphysVol) {
            THROW_EXCEPTION("Failed to fetch volume "<<physVolId<<" from table "<<physVolTableId);
        }
        auto nodePtr = make_intrusive<GeoSerialTransformer>(vphysVol, func.get(), copies);

        if (!m_memMapSerialTransformers.insert(std::make_pair(serialID, nodePtr))){
            THROW_EXCEPTION("Failed to register serial transformer "<<serialID);
        }
    }
    if (m_memMapSerialTransformers.size()) {
        std::cout << "All " << m_memMapSerialTransformers.size()
                  << " SerialTransformers have been built!\n";
    }
}

void ReadGeoModel::connectNodes() {
    
    const DBRowsList records = m_dbManager->getChildrenTable();
 
    if (m_loglevel >= 1) {
        PRINT_MSG(" - processing " << records.size() << " keys...");
    }
    auto& pool{GeoThreading::ThreadPool::getPool()};
    for (std::size_t start = 0 ; start < records.size(); ){
        std::size_t end = std::min(start + objectBatch, records.size());
        /// Ensure that a parent volume is never chopped into two batches
        for (bool newVol{false} ; !newVol && end < records.size() -1; ++end) {
            const DBRowEntry& currEnd{records[end]};
            const DBRowEntry& nextOne{records[end+1]};

            if(UnpackData_t::getFromVariant_Int(currEnd[1], "currEnd:parentID") !=
               UnpackData_t::getFromVariant_Int(nextOne[1], "nextOne:parentID") ||
               UnpackData_t::getFromVariant_Int(currEnd[2], "currEnd:parentTableId") !=
               UnpackData_t::getFromVariant_Int(nextOne[2], "nextOne:parentTableId")){
               newVol = true;
            }
        }
        pool.appendTask([this, start,end, &records](){
            if (m_loglevel >= 2) {
                std::stringstream sstr{};
                sstr<< " - processing records from " << start << " to " << end << std::endl;
                for (std::size_t itr = start; itr < end; ++itr) {
                    const DBRowEntry& record{records[itr]};
                    sstr<<"["<<UnpackData_t::getFromVariant_Int(record[1], "record:parentID")
                        <<";"<<UnpackData_t::getFromVariant_Int(record[2], "record:parentTableId") << "], ";
                }
                PRINT_MSG(sstr.str());
            }
            for (std::size_t itr = start; itr < end; ++itr) {
                processParentChild(records[itr]);
            }
        });
        start = end;
    }
    pool.drainQueue();
}

void ReadGeoModel::processParentChild(const DBRowEntry& parentchild) {
    // safety check
    if (parentchild.size() < 8) {
        THROW_EXCEPTION("ERROR!!! Probably you are using an old geometry file. Please, get a new one. Exiting...");
    }

    // get the parent's details
    const unsigned int parentId = UnpackData_t::getFromVariant_Int(parentchild[1], "ParentChild:parentID");
    const unsigned int parentTableId = UnpackData_t::getFromVariant_Int(parentchild[2], "ParentChild:parentTableId"); 
    const unsigned int parentCopyN = UnpackData_t::getFromVariant_Int(parentchild[3], "ParentChild:parentID"); 

    // get the child's position in the parent's children list
    // const unsigned int position = parentchild[4]; // unused, at the moment

    // get the child's details
    const unsigned int childTableId = UnpackData_t::getFromVariant_Int(parentchild[5], "ParentChild:parentID"); 
    const unsigned int childId = UnpackData_t::getFromVariant_Int(parentchild[6], "ParentChild:childID"); 
    const unsigned int childCopyN = UnpackData_t::getFromVariant_Int(parentchild[7], "ParentChild:childCopyN"); 

    std::string childNodeType = m_tableID_toTableName[childTableId];

    if (childNodeType.empty()) {
        THROW_EXCEPTION("childNodeType is empty!!! Aborting...");
    }


    // build or get parent volume.
    // Using the parentCopyNumber here, to get a given instance of the
    // parent volume
    if (m_loglevel >= 3) {
        PRINT_MSG("build/get parent volume...");
    }
    PVLink parentVol = getVPhysVol(parentId, parentTableId);
    if (childNodeType == "GeoPhysVol") {
        addGraphNode(parentVol, getVPhysVol(childId, childTableId));
    } else if (childNodeType == "GeoFullPhysVol") {
        addGraphNode(parentVol, getVPhysVol(childId, childTableId));
    } else if (childNodeType == "GeoVSurface") {
        addGraphNode(parentVol, getBuiltSurface(childId));
    } else if (childNodeType == "GeoSerialDenominator") {
        addGraphNode(parentVol, getBuiltSerialDenominator(childId));
    } else if (childNodeType == "GeoSerialIdentifier") {
        addGraphNode(parentVol, getBuiltSerialIdentifier(childId));
    } else if (childNodeType == "GeoIdentifierTag") {
        addGraphNode(parentVol, getBuiltIdentifierTag(childId));
    } else if (childNodeType == "GeoAlignableTransform") {
        addGraphNode(parentVol,  getBuiltAlignableTransform(childId));
    } else if (childNodeType == "GeoTransform") {
        addGraphNode(parentVol, getBuiltTransform(childId));
    } else if (childNodeType == "GeoSerialTransformer") {
        addGraphNode(parentVol, getBuiltSerialTransformer(childId));
    } else if (childNodeType == "GeoNameTag") {
        addGraphNode(parentVol, getBuiltNameTag(childId));
    } else {
        THROW_EXCEPTION("[" << childNodeType<< "] ==> ERROR!!! - The conversion for this type of child "
                      <<"node needs to be implemented.");
    }
}
// Instantiate a PhysVol and get its children
PVLink ReadGeoModel::getVPhysVol(const unsigned int id, const unsigned int tableId) const {
    if (m_loglevel >= 3) {
        PRINT_MSG("id: " << id << ", tableId: " << tableId);
    }
    const std::string& tableName = m_tableID_toTableName.at(tableId);
    if (tableName == "GeoPhysVol") {
        return getBuiltPhysVol(id);
    } else if (tableName == "GeoFullPhysVol") {
        return getBuiltFullPhysVol(id);
    }
    return nullptr;
}
// Get the root volume
PVLink ReadGeoModel::getRootVolume() {
    if (m_loglevel >= 3) {
        std::lock_guard guard{muxCout};
        std::cout << "ReadGeoModel::getRootVolume()" << std::endl;
        std::cout << "m_root_vol_data: " << m_root_vol_data.first << ", " << m_root_vol_data.second << std::endl;       
    }
    const unsigned tableId = m_root_vol_data.first;
    const unsigned id = m_root_vol_data.second;
    // const unsigned int tableId = m_dbManager->getTableIdFromNodeType(tableName);
    PVLink root = getVPhysVol(id, tableId);
    if (!root) {
        THROW_EXCEPTION("ROOT volume cannot be built.");
    }
    return root;
}


GeoIntrusivePtr<GeoShape> ReadGeoModel::getBuiltShape(const unsigned shapeId, const std::string& shapeType) const {
    auto factory_itr = m_shapeFactories.find(shapeType);
    if (factory_itr != m_shapeFactories.end()) {
        return factory_itr->second->getShape(shapeId);
    }
    THROW_EXCEPTION("WARNING! 'getBuiltShape' - For the shape '" << shapeType << "' we're using the old DB schema...");
    return nullptr;
}

GeoIntrusivePtr<GeoLogVol> ReadGeoModel::getBuiltLog(const unsigned int id) const { return m_memMapLogVols.get(id); }
PVLink ReadGeoModel::getBuiltPhysVol(const unsigned int id) const { return m_memMapPhysVols.get(id); }
GeoIntrusivePtr<GeoFullPhysVol> ReadGeoModel::getBuiltFullPhysVol(const unsigned int id) const { return m_memMapFullPhysVols.get(id); }
GeoIntrusivePtr<GeoMaterial> ReadGeoModel::getBuiltMaterial(const unsigned int id) const { return m_memMapMaterials.get(id); }
GeoIntrusivePtr<GeoElement> ReadGeoModel::getBuiltElement(const unsigned int id) const { return m_memMapElements.get(id); }
GeoIntrusivePtr<GeoTransform> ReadGeoModel::getBuiltTransform(const unsigned int id) const { return m_memMapTransforms.get(id); }

GeoIntrusivePtr<GeoAlignableTransform>
 ReadGeoModel::getBuiltAlignableTransform(const unsigned int id) const { return m_memMapAlignableTransforms.get(id); }
GeoIntrusivePtr<GeoSerialDenominator> 
    ReadGeoModel::getBuiltSerialDenominator(const unsigned int id) const { return m_memMapSerialDenominators.get(id); }
GeoIntrusivePtr<GeoSerialIdentifier> 
    ReadGeoModel::getBuiltSerialIdentifier(const unsigned int id)  const { return m_memMapSerialIdentifiers.get(id); }
GeoIntrusivePtr<GeoIdentifierTag> 
    ReadGeoModel::getBuiltIdentifierTag(const unsigned int id) const { return m_memMapIdentifierTags.get(id); }
GeoIntrusivePtr<GeoNameTag> ReadGeoModel::getBuiltNameTag(const unsigned int id) const { return m_memMapNameTags.get(id); }
GeoIntrusivePtr<GeoSerialTransformer> 
    ReadGeoModel::getBuiltSerialTransformer(const unsigned int id) const {return m_memMapSerialTransformers.get(id); }
GeoIntrusivePtr<GeoVSurface> ReadGeoModel::getBuiltSurface(const unsigned int id) const { return m_memMapVSurface.get(id); }
std::shared_ptr<const GeoXF::Function> ReadGeoModel::getBuiltFunction(const unsigned int id) const { return m_memMapFunctions.get(id); }
} /* namespace GeoModelIO */

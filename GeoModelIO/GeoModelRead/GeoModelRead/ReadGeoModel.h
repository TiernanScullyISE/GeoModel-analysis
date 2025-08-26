/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ReadGeoModel.h
 *
 * Created on: May 20, 2016
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 * major updates:
 * - 2019 Feb, R.M.Bianchi
 * - 2020 May, R.M.Bianchi - Added parallel read
 * - 2020 Aug, R.M.Bianchi - Added support for reading back published nodes
 * - 2021 Aug, R.M.Bianchi <riccardo.maria.bianchi@cern.ch> - Added support
 * GeoIdentifierTag and GeoSerialIdentifier nodes
 * - Jun 2022, R.M.Bianchi <riccardo.maria.bianchi@cern.ch>
 *              Fixed the duplication of VPhysVol instances due to a wrong key
 * used for caching volumes that were built already The copyNumber was wrongly
 * used together with tableID and volID For details, see:
 * https://gitlab.cern.ch/GeoModelDev/GeoModel/-/issues/39
 * - Jan 2023, R.M.Bianchi <riccardo.maria.bianchi@cern.ch>
 *             Added getters to get number of GeoModel nodes
 *             restored from the * .db file
 * - 2023 Jan, R.M.Bianchi <riccardo.maria.bianchi@cern.ch>
 *   Added method to get records out of custom tables from client code.
 * - Feb 2023, R.M.Bianchi <riccardo.maria.bianchi@cern.ch>
 *             Added 'setLoglevel' method, to steer output messages
 *
 * - Jun 2024, R.Xue  <r.xue@cern.ch><rux23@pitt.edu>
 *             Added methods to read in virtual surfaces from .db files
 *
 */

#ifndef GeoModelRead_ReadGeoModel_H_
#define GeoModelRead_ReadGeoModel_H_


// GeoModel includes
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelDBManager/definitions.h"

#include "GeoModelKernel/GeoXF.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoVSurface.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoSerialDenominator.h"
#include "GeoModelKernel/GeoSerialIdentifier.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoVSurface.h"

// C++ includes
#include <deque>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>
#include <memory>

// FWD declarations
#include "GeoModelRead/GeoIdObjMap.h"
#include "GeoModelRead/BuildGeoShapes.h"
#include "GeoModelRead/BuildGeoVSurface.h"


namespace GeoModelIO {
class IO;
class ReadGeoModel {
   public:
    friend class IO;
    /** @brief constructor not taking the ownership of the GMDBManager 
     *  @param db: Raw pointer to the GMDBManager */
    [[deprecated("Please use the constructor parsing a shared pointer instead.")]]
    explicit ReadGeoModel(GMDBManager* db);
    /** @brief Constructor taking a shared pointer to the GMDBManager.
        @param: Pointer to the GMDBManager */
    explicit ReadGeoModel(std::shared_ptr<GMDBManager> db);
    /** @brief Default move assignment */
    ReadGeoModel& operator=(ReadGeoModel&& other) = default;
    /** @brief Default move constructor */
    ReadGeoModel(ReadGeoModel&& other) = default;

    ~ReadGeoModel() = default;

    PVConstLink buildGeoModel();
    /// Set the 'loglevel', that is the level of output messages.
    /// The loglevel is set to 0 by default, but it can be set
    /// to a larger value.
    /// Loglevel:
    /// - 0 : Default
    /// - 1 : Verbose
    /// - 2 : Debug
    void setLogLevel(unsigned loglevel) { m_loglevel = loglevel; };

    // NB, this template method needs only the "publisher name" to be specified
    // (i.e. the last suffix), since the first part of the table name get added
    // automatically according to the data type it is templated on
    template <typename T, class N>
    std::map<T, N> getPublishedNodes(std::string publisherName = "",
                                     bool doCheckTable = false) const;

    [[deprecated("Please use dbManager().printAllRecords() instead.")]]
    void printDBTable(const std::string& tableName) const {
        m_dbManager->printAllRecords(tableName);
    }
    [[deprecated("Please use dbManager().printAllDBTables() instead.")]]
    void printAllDBTables() const { 
        m_dbManager->printAllDBTables(); 
    }
    [[deprecated("Please use dbManager().getTableRecords_String() instead.")]]
    std::vector<std::vector<std::string>> getTableFromTableName_String(const std::string_view tableName) {
        return m_dbManager->getTableRecords_String(tableName);
    }

    /** @brief Returns the reference to the dbManager */
    GMDBManager& dbManager() const;
    /** @brief Returns the shared pointer to the dBManager */
    std::shared_ptr<GMDBManager> dbManagerPtr() const;

    /** @brief Returns a GeoShape of the specified shape type which is registered under the shapeId. If the
     *         shape has not been built yet, it's attempted to construct it. If the shape is unknown an exception
     *         is thrown.
     * @param shapeId: Identifier of the shape to fetch
     * @param shapeType: String encoding which factory should be queried to return the GeoShape. */
    GeoIntrusivePtr<GeoShape> getBuiltShape(const unsigned shapeId, const std::string& shapeType) const;
    /** @brief Returns the pointer to the ordinary transform which is registered under the 
     *         corresponding ID. If not registered a nullptr is returned.
     * @param id: Identifier of the transform to fetch. */
    GeoIntrusivePtr<GeoTransform> getBuiltTransform(const unsigned id) const;
    /** @brief Returns the pointer to the alignable transform which is registered under the 
     *         corresponding ID. If not registered a nullptr is returned.
     * @param id: Identifier of the transform to fetch. */
    GeoIntrusivePtr<GeoAlignableTransform> getBuiltAlignableTransform(const unsigned id) const;
   private:
    /** @brief Prepare the factories and load the neccessary tables from the database to construct the GeoModel tree
     *         from SQLite. Called at the beginning of the GeoModelTree construction phase */
    void loadDB();
    /** @brief Setup a ShapeFactory of ShapeFactory_t type.
     *  @param shapeType: Name under which the factory is later registered
     *  @param primaryDataTable: Name of the SQLite table to fetch all data from */
    template <typename ShapeFactory_t> void setupShapeFactory(const std::string& shapeType,
                                                              const std::string& primaryDataTable);
    /** @brief Setup a ShapeFactory of ShapeFactory_t type. Extension to cope with auxillary tables
     *  @param shapeType: Name under which the factory is later registered
     *  @param primaryDataTable: Name of the SQLite table to fetch all data from
     *  @param auxillaryDataTable: Name of the SQLite table to fetch the auxillary data from */
    template <typename ShapeFactory_t> void setupShapeFactory(const std::string& shapeType,
                                                              const std::string& primaryDataTable,
                                                              const std::string& auxilllaryDataTable);    
    /** @brief Setups a GeohapeFactory of ShapeFactory_t type where the factory type needs to create
     *         boolean volumes. In contrast, to the ordinary factories, the boolean shape constructors
     *         also pass a reference to this GeoModelRead instance in order to build the operands of the
     *         booleans on the fly
     * @param shapeType: Name under which the factory is later registered
     * @param primaryDataTable: Name of the SQLite table to fetch all instructions from. */
    template <typename ShapeFactory_t> void setupBooleanFactory(const std::string& shapeType,
                                                                const std::string& primaryDataTable);    
    /** @brief Setups a virtual surface shape factory of ShapeFactory_t type. Likewise, the GeoShape
     *         factories, the creates factory is centrally registered and called when the corresponding
     *         virtual surface is assembled.
     * @param surfaceType: Name of the surface type which is assembled by the factory
     * @param dataTable: Name of the SQLite table to fetch all parameters to assemble the shapes */
    template <typename SurfaceFactory_t> void setupSurfaceFactory(const std::string& surfaceType,
                                                                  const std::string& dataTable);

    /** @brief Factory method to construct (Full)PhysVols from the SQLite DB file. The corresponding
     *         table is processed and the constructed & empty volumes are parsed into the memory cache
     *  @param tableName: Name of the SQLite table containing all information to build the volumes
     *  @param memCache: Refrence to the target memory cache where all volumes are stored. */
    template <typename PhysVol_t>
        void buildPhysVols(const std::string& tableName,
                           GeoIdObjMap<GeoIntrusivePtr<PhysVol_t>>& memCache);
    /** @brief Factory method to contruct (Alignable)Transforms from the SQLite DB file. The corresponding table
     *         is processed and the constructed transforms are added to the cache map
     * @param tableName: Name of the SQLite table containing the primary numbers of the transforms
     *  @param memCache: Reference to the target memory cache where all transforms are stored. */
    template <typename Transform_t>
        void buildTransforms(const std::string& tableName,
                             GeoIdObjMap<GeoIntrusivePtr<Transform_t>>& memCache);
    /** @brief Loads the GeoElement data and constructs them in memory */
    void buildAllElements();
    /** @brief Loads the Material data and constructs them in memory */
    void buildAllMaterials();
    /** @brief Loads the logical volume data and constructs them in memory */
    void buildAllLogVols();
    /** @brief Builds the virtual surfaces */
    void buildAllVSurfaces();
    /** @brief Builds all transforms */
    void buildAllTransforms();
    /** @brief Builds all alignable transforms */
    void buildAllAlignableTransforms();
    /** @brief Builds the serial demoniators */
    void buildAllSerialDenominators();
    /** @brief Builds the serial Identifiers */
    void buildAllSerialIdentifiers();
    /** @brief Builds the generic Identifiers */
    void buildAllIdentifierTags();
    /** @brief Builds the physical volume templates */
    void buildAllPhysVols();
    /** @brief Build the full physical volumes */
    void buildAllFullPhysVols();
    /** @brief Build the name tags */
    void buildAllNameTags();
    /** @brief Build all geo functions */
    void buildAllGeoFunc();
    /** @brief Build the serial transformers */
    void buildAllSerialTransformers();
    /** @brief Entry hook to start the construction of the geometry tree from the DB.
     *         Returns the pointer to the world volume once finished. */
    PVLink assembleWorld();
    /** @brief Reads the child record table to fill the empty Full(Phys)Vols with 
     *         transforms, Identifiers, name tags, serial transforms, other volumes or virtual surfaces */
    void connectNodes();
    /** @brief Process a single entry from the child record table to add a new object to a PhysVol */
    void processParentChild(const DBRowEntry& parentchild);
    /** @brief fetches the world volume ID from the table and returns the corresponding PhysVol */
    PVLink getRootVolume();
    /** @brief Returns the constructed logical volume
     *  @param id: Identifier as stored in the SQLite tables */
    GeoIntrusivePtr<GeoLogVol> getBuiltLog(const unsigned int id) const;
    /** @brief Returns the constructed GeoMaterial
     *  @param id: Identifier as stored in the SQLite tables */
    GeoIntrusivePtr<GeoMaterial> getBuiltMaterial(const unsigned int id) const;
    /** @brief Returns the constructed GeoElement
     *  @param id: Identifier as stored in the SQLite tables */
    GeoIntrusivePtr<GeoElement> getBuiltElement(const unsigned int id) const;
    /** @brief Returns the constructed GeoPhysVol
     *  @param id: Identifier as stored in the SQLite tables */
    PVLink getBuiltPhysVol(const unsigned int id) const;
    /** @brief Returns the constructed GeoFullPhysVol
     *  @param id: Identifier as stored in the SQLite tables */
    GeoIntrusivePtr<GeoFullPhysVol> getBuiltFullPhysVol(const unsigned int id) const;

    GeoIntrusivePtr<GeoSerialDenominator> getBuiltSerialDenominator(const unsigned int id) const;

    GeoIntrusivePtr<GeoSerialIdentifier> getBuiltSerialIdentifier(const unsigned int id) const;

    GeoIntrusivePtr<GeoIdentifierTag> getBuiltIdentifierTag(const unsigned int id) const;

    GeoIntrusivePtr<GeoNameTag> getBuiltNameTag(const unsigned int id) const;

    GeoIntrusivePtr<GeoSerialTransformer> getBuiltSerialTransformer(const unsigned int id) const;

    GeoIntrusivePtr<GeoVSurface> getBuiltSurface(const unsigned int id) const;

    std::shared_ptr<const GeoXF::Function> getBuiltFunction(const unsigned int id) const;

    PVLink getVPhysVol(const unsigned int id, const unsigned int tableId) const;

    std::shared_ptr<GMDBManager> m_dbManager{};
    bool m_timing{false};

    PVLink m_world{};

    // to look for table ID starting from node's type name
    std::unordered_map<unsigned int, std::string> m_tableID_toTableName{};
    // to look for node's type name starting from a table ID
    std::unordered_map<std::string, unsigned int> m_tableName_toTableID{};  
    std::pair<unsigned, unsigned> m_root_vol_data{};
    /** @brief Container of all registered volume shape factories */
    std::map<std::string, std::unique_ptr<BuildGeoShapes>> m_shapeFactories{};
    /** @brief Container of all registeres surface shape factories */
    std::map<std::string, std::unique_ptr<BuildGeoVSurface>> m_surfaceFactories{};
    /** @brief List of all cached GeoPhysVol instances */
    GeoIdObjMap<GeoIntrusivePtr<GeoPhysVol>> m_memMapPhysVols{};
    /** @brief List of all cahced GeoFullPhysVol instances */
    GeoIdObjMap<GeoIntrusivePtr<GeoFullPhysVol>> m_memMapFullPhysVols{};
    /** @brief Cached non-alignable transforms */
    GeoIdObjMap<GeoIntrusivePtr<GeoTransform>> m_memMapTransforms{};
    /** @brief Cached alignable transforms */
    GeoIdObjMap<GeoIntrusivePtr<GeoAlignableTransform>> m_memMapAlignableTransforms{};
    /** @brief Cached logical volumes */
    GeoIdObjMap<GeoIntrusivePtr<GeoLogVol>> m_memMapLogVols{};
    /** @brief Cached Materials */
    GeoIdObjMap<GeoIntrusivePtr<GeoMaterial>> m_memMapMaterials{};
    /** @brief Cached Elements */
    GeoIdObjMap<GeoIntrusivePtr<GeoElement>> m_memMapElements{};
    /** @brief Cached Serial denominators */
    GeoIdObjMap<GeoIntrusivePtr<GeoSerialDenominator>> m_memMapSerialDenominators{};
    /** @brief Cached Serial Identifiers */
    GeoIdObjMap<GeoIntrusivePtr<GeoSerialIdentifier>> m_memMapSerialIdentifiers{};
    /** @brief Cached Identifier tags */
    GeoIdObjMap<GeoIntrusivePtr<GeoIdentifierTag>> m_memMapIdentifierTags{};
    /** @brief Cached name tags */
    GeoIdObjMap<GeoIntrusivePtr<GeoNameTag>> m_memMapNameTags{};
    /** @brief Cached virtual surfaces */
    GeoIdObjMap<GeoIntrusivePtr<GeoVSurface>> m_memMapVSurface{};
    /** @brief Cached serial transformers */
    GeoIdObjMap<GeoIntrusivePtr<GeoSerialTransformer>> m_memMapSerialTransformers{};
    /** @brief Cached function table */
    GeoIdObjMap<std::shared_ptr<const GeoXF::Function>> m_memMapFunctions{};
    /** @brief Stores the loglevel, the level of output messages */
    unsigned m_loglevel{0};
    /** @brief Swtich to toggle whether the maps are cleared as soon as they're no longer needed. */
    bool m_autoClean{true};
};

} /* namespace GeoModelIO */

// include the implementation of the class' template functions
#include "ReadGeoModel.tpp"

#endif /* GeoModelRead_ReadGeoModel_H_ */


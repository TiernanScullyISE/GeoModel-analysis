/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * author: Riccardo.Maria.Bianchi@cern.ch, 2017
 *
 * major updates:
 * - Aug 2018 - R.M.Bianchi
 * - Feb 2019 - R.M.Bianchi
 * - May 2020 - R.M.Bianchi
 * - Aug 2020 - R.M.Bianchi // added support for GeoPublisher
 * - Aug 2021 - Riccardo Maria Bianchi, <riccardo.maria.bianchi@cern.ch>
 *              Added support for GeoSerialTransformer nodes
 * - Jun 2022 - Riccardo Maria Bianchi, <riccardo.maria.bianchi@cern.ch>,
 *              Added support for "Verbose" output
 * - Jan 2023 - Riccardo Maria Bianchi, <riccardo.maria.bianchi@cern.ch>,
 *              Added the 'inspect' constructor to inspect
 *              the in-memory GeoModel tree, without saving to a .db file.
 *              Also added methods to get the numbers of visited nodes.
 * - Feb 2023 - R.M.Bianchi <riccardo.maria.bianchi@cern.ch>
 *              Added 'setLoglevel' method, to steer output messages
 * - Jun 2024 - R.Xue <r.xue@cern.ch><rux23@pitt.edu>
 *              Added methods to write out virtual surfaces to .db files
 * - May 2024 - Riccardo Maria Bianchi, <riccardo.maria.bianchi@cern.ch>,
 *              Major change: we move to the new DB schema that uses numerical data
 *              instead of strings/TEXT
 * - Oct 2024 - Riccardo Maria Bianchi, <riccardo.maria.bianchi@cern.ch>,
 *              Support for the EllipticalTube shape was added.
 * - Sep 2025 - riccardo.maria.bianchi@cern.ch,
 *              removed old relics of vector<vector<string>> and moved 
 *              all methods to the new variant-based data types.
 */

#ifndef GeoModelWrite_WriteGeoModel_H
#define GeoModelWrite_WriteGeoModel_H

// local includes
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelDBManager/definitions.h"


// GeoModel includes
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoGraphNode.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoNodeAction.h"
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoXF.h"
#include "GeoModelKernel/GeoVSurfaceShape.h"
#include "GeoModelKernel/GeoPublisher.h"

// C++ includes
#include <set>
#include <deque>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>



// FWD declarations
class GeoPublisher;


typedef std::unordered_map<
    std::string, std::pair<std::vector<std::string>, std::vector<std::string>>>
    AuxTableDefs;
typedef std::unordered_map<
    std::string, std::vector<std::vector<
                     std::variant<int, long, float, double, std::string>>>>
    AuxTableData;
namespace GeoModelIO {
  class WriteGeoModelTest;
/**
 * \class WriteGeoModel
 *
 * WriteGeoModel acts on objects of the GeoModel tree
 * persitifying them offline.
 */
class WriteGeoModel : public GeoNodeAction {
   public:
    /**
     * @brief Constructor
     *
     * Constructor sets up connection with db and opens it
     * @param path - absolute path to db file
     */
    WriteGeoModel(GMDBManager &db);

    /**
     * @brief 'Inspect' Contructor, it does not save to the DB
     */
    WriteGeoModel() { m_inspect = true; };

    /**
     * @brief Destructor
     */
    ~WriteGeoModel();

    /// Set the 'loglevel', that is the level of output messages.
    /// The loglevel is set to 0 by default, but it can be set
    /// to a larger value.
    /// Loglevel:
    /// - 0 : Default
    /// - 1 : Verbose
    /// - 2 : Debug
    /// - 3 : Deep Debug
    void setLogLevel(unsigned loglevel) { m_loglevel = loglevel; };

    virtual void handlePhysVol(
        const GeoPhysVol *vol);  //	Handles a physical volume.
    virtual void handleFullPhysVol(
        const GeoFullPhysVol *vol);  //	Handles a full physical volume.
    virtual void handleVSurface(
        const GeoVSurface* surf);  //	Handles a virtual surface.        
    virtual void handleSerialDenominator(
        const GeoSerialDenominator *sD);  //	Handles a Serial Denominator.
    virtual void handleSerialTransformer(
        const GeoSerialTransformer *obj);  //	Handles a Serial Transformer
    virtual void handleTransform(const GeoTransform *);  //	Handles a
                                                         // Transform.
    virtual void handleNameTag(const GeoNameTag *);      //	Handles a Name Tag.
    virtual void handleIdentifierTag(
        const GeoIdentifierTag *);  //	Handles an Identifier Tag.
    virtual void handleSerialIdentifier(
        const GeoSerialIdentifier *);  //	Handles a SerialIdentifier.

    // void storeDataTable( std::string tableName, std::vector<std::string>
    // colNames, std::vector<std::string> colTypes,
    // std::vector<std::vector<std::string>> tableData );
    void storeDataTable(
        const std::string& tableName, const std::vector<std::string>& colNames,
        const std::vector<std::string>& colTypes,
        DBRowsList
            tableData);

    void saveToDB(GeoPublisher *store = nullptr);
    void saveToDB(std::vector<GeoPublisher *> &vecStores);

    unsigned long getNLogVols() { return m_logVols.size(); };
    unsigned long getNPhysVols() { return m_physVols.size(); };
    unsigned long getNFullPhysVols() { return m_fullPhysVols.size(); };
    unsigned long getNMaterials() { return m_materials.size(); };
    unsigned long getNElements() { return m_elements.size(); };
    unsigned long getNTransforms() { return m_transforms.size(); };
    unsigned long getNAlignableTransforms() {
        return m_alignableTransforms.size();
    };
    unsigned long getNSerialDenominators() {
        return m_serialDenominators.size();
    };
    unsigned long getNSerialIdentifiers() {
        return m_serialIdentifiers.size();
    };
    unsigned long getNIdentifierTags() { return m_identifierTags.size(); };
    unsigned long getNSerialTransformers() {
        return m_serialTransformers.size();
    };
    unsigned long getNFunctions() { return m_functions.size(); };
    unsigned long getNNameTags() { return m_nameTags.size(); };
    unsigned long getNChildrenConnections() {
        return m_childrenPositions.size();
    };
    // unsigned long getNShapes() { return m_shapes.size(); }; // TODO: to be removed later
    unsigned long getNShapes_Box() { return m_shapes_Box.size(); };
    unsigned long getNShapes_EllipticalTube() { return m_shapes_EllipticalTube.size(); };
    unsigned long getNShapes_Cons() { return m_shapes_Cons.size(); };
    unsigned long getNShapes_Para() { return m_shapes_Para.size(); };
    unsigned long getNShapes_Pcon() { return m_shapes_Pcon.size(); };
    unsigned long getNShapes_Pgon() { return m_shapes_Pgon.size(); };
    unsigned long getNShapes_SimplePolygonBrep() { return m_shapes_SimplePolygonBrep.size(); };
    unsigned long getNShapes_GenericTrap() { return m_shapes_GenericTrap.size(); };
    unsigned long getNShapes_Trap() { return m_shapes_Trap.size(); };
    unsigned long getNShapes_Trd() { return m_shapes_Trd.size(); };
    unsigned long getNShapes_Tube() { return m_shapes_Tube.size(); };
    unsigned long getNShapes_Tubs() { return m_shapes_Tubs.size(); };
    unsigned long getNShapes_Torus() { return m_shapes_Torus.size(); };
    unsigned long getNShapes_TwistedTrap() { return m_shapes_TwistedTrap.size(); };
    unsigned long getNShapes_UnidentifiedShape() { return m_shapes_UnidentifiedShape.size(); };
    unsigned long getNShapes_Shift() { return m_shapes_Shift.size(); };
    unsigned long getNShapes_Union() { return m_shapes_Union.size(); };
    unsigned long getNShapes_Intersection() { return m_shapes_Intersection.size(); };
    unsigned long getNShapes_Subtraction() { return m_shapes_Subtraction.size(); };

   private:
    // define copy constructor, needed for the GeoModelAction subclass
    WriteGeoModel(const WriteGeoModel &right);

    // define assignment operator, needed for the GeoModelAction subclass
    WriteGeoModel &operator=(const WriteGeoModel &right);

    void handleVPhysVolObjects(const GeoVPhysVol *vol);
    void handleVSurfaceObjects(const GeoVSurface *surf);
    /// Handles a physical volume referenced by a SerialTrasformer
    void handleReferencedVPhysVol(const GeoVPhysVol *vol);

    void showMemoryMap();

    std::vector<std::string> getParentNode();

    std::pair<std::string, unsigned> storeShape(const GeoShape *shape);
    unsigned int storeMaterial(const GeoMaterial *mat);
    unsigned int storeElement(const GeoElement *el);
    unsigned int storeTranform(const GeoTransform *node);
    
    // virtual surface ---------------------------------------------------------------------
    unsigned int storeSurfaceShape(const GeoVSurfaceShape* surf);
    std::pair<DBRowEntry, DBRowsList> getSurfaceParameters(const GeoVSurfaceShape* surf_shape);
    unsigned int addSurfaceShape(const std::string &type,  const DBRowEntry &parameters);
    unsigned int storeVSurface(const GeoVSurface* pointer, const unsigned int& surfShapeId, const std::string& surfShapeType);
    unsigned int addVSurface(const std::string &type, const unsigned int& surfShapeId);
    unsigned int storeObj(const GeoVSurfaceShape* pointer, const std::string& shapeName, 
                          DBRowEntry& parameters, const DBRowsList &shapeData);
    unsigned int setSurfaceCopyNumber(const unsigned int& VSurfaceId, const std::string& surfType);                          
    //--------------------------------------------------------------------------------------
    
    unsigned int storeObj(const GeoMaterial *pointer, const std::string &name,
                          const double &density, const DBRowsList &materialData);
    unsigned int storeObj(const GeoElement *pointer, const std::string &name,
                          const std::string &symbol, const double &elZ,
                          const double &elA);
    // unsigned int storeObj(const GeoShape *pointer, const std::string &type,
    //                       const std::string &parameters);
    std::pair<std::string, unsigned> storeObj(const GeoShape *pointer, const std::string &type,
                                            DBRowEntry &parameters,
                                            const DBRowsList &shapeData);
    unsigned int storeObj(const GeoLogVol *pointer, const std::string &name,
                          const unsigned int &shapeId, std::string_view shapeType,
                          const unsigned int &materialId);
    unsigned int storeObj(const GeoPhysVol *pointer,
                          const unsigned int &logvolId,
                          const unsigned int parentId = 0,
                          const bool isRootVolume = false);
    unsigned int storeObj(const GeoFullPhysVol *pointer,
                          const unsigned int &logvolId,
                          const unsigned int parentId = 0,
                          const bool isRootVolume = false);
    unsigned int storeObj(const GeoSerialDenominator *pointer,
                          const std::string &baseName);
    unsigned int storeObj(const GeoSerialIdentifier *pointer,
                          const int &baseId);
    unsigned int storeObj(const GeoIdentifierTag *pointer,
                          const int &identifier);
    unsigned int storeObj(const GeoSerialTransformer *pointer,
                          const unsigned int &functionId,
                          const unsigned int &volId, const std::string &volType,
                          const unsigned int &copies);
    unsigned int storeObj(const GeoXF::Function *pointer);
                        //   const std::string &expression,
                        //   const std::deque<double> &exprData);
    unsigned int storeObj(const GeoTransform *pointer,
                          const std::vector<double> &parameters);
    unsigned int storeObj(const GeoAlignableTransform *pointer,
                          const std::vector<double> &parameters);
    unsigned int storeObj(const GeoNameTag *pointer, const std::string &name);

    // void storeExprData(const unsigned funcId, std::deque<double> exprData); // TODO:
    std::vector<unsigned> addExprData(const std::deque<double> &exprData) ;

    unsigned int addRecord(std::vector<std::vector<std::string>> *container,
                           const std::vector<std::string>& values) const;
    unsigned int addRecord(DBRowsList *container,
                           const DBRowEntry& values) const;
    
    std::pair<unsigned, unsigned> addRecordData(
        DBRowsList *container,
        const DBRowsList& values) const;

    unsigned int addElement(const std::string &name, const std::string &symbol,
                            const double &elZ, const double &elA);
    
    unsigned int addMaterial(const std::string &name, const double &density,
                             const unsigned &dataStart, const unsigned &dataEnd);
    std::pair<unsigned, unsigned> addMaterialData(const DBRowsList &matElementFraction);

    unsigned int addNameTag(const std::string &name);
    unsigned int addAlignableTransform(const std::vector<double> &params);
    unsigned int addTransform(const std::vector<double> &params);
    unsigned int addFunction(const std::string &expression, const unsigned &dataStart, const unsigned &dataEnd);
    unsigned int addSerialTransformer(const unsigned &funcId,
                                      const unsigned &physvolId,
                                      const std::string& volType,
                                      const unsigned &copies);
    
    // unsigned int addShape(const std::string &type,
    //                       const std::string &parameters);
    unsigned int addShape(const std::string &type,
                          const DBRowEntry &parameters);
    std::pair<unsigned, unsigned> addShapeData(const std::string& type,
                                       const DBRowsList &shapeData);
    
    unsigned int addSerialDenominator(const std::string &baseName);
    unsigned int addSerialIdentifier(const int &baseId);
    unsigned int addIdentifierTag(const int &identifier);
    unsigned int addPhysVol(const unsigned int &logVolId,
                            const bool &isRootVolume);
    unsigned int addFullPhysVol(const unsigned int &logVolId,
                                const bool &isRootVolume);
    unsigned int addLogVol(const std::string &name, const unsigned int &shapeId,
                           std::string_view shapeType, const unsigned int &materialId);
    void addChildPosition(const unsigned int &parentId,
                          const std::string &parentType,
                          const unsigned int &childId,
                          const unsigned int &parentCopyNumber,
                          const unsigned int &childPos,
                          const std::string &childType,
                          const unsigned int &childCopyN);

    unsigned int getChildPosition(const unsigned int &parentId,
                                  const std::string &parentType,
                                  const unsigned int &copyN);

    unsigned int setVolumeCopyNumber(const unsigned int &volId,
                                     const std::string &volType);
    unsigned int getLatestParentCopyNumber(const unsigned int &parentId,
                                           const std::string &parentType);

    void storeChildPosition(const unsigned int &parentId,
                            const std::string &parentType,
                            const unsigned int &childVol,
                            const unsigned int &parentCopyNumber,
                            const unsigned int &childPos,
                            const std::string &childType,
                            const unsigned int &childCopyN);

    bool isAddressStored(const std::string &address);
    void storeAddress(const std::string &address, const unsigned int &id);

    unsigned int getStoredIdFromAddress(const std::string &address);


    std::vector<double> getTransformParameters(
        GeoTrf::Transform3D);  // TODO: to be moved to Eigen (GeoTrf) and to be
                               // moved to an Utility class, so we can use it
                               // from TransFunctionRecorder as well.
    // std::string getShapeParameters(const GeoShape *);
    std::pair<DBRowEntry,
              DBRowsList>
    getShapeParametersV(const GeoShape *, const bool data = false);

    std::string getGeoTypeFromVPhysVol(const GeoVPhysVol *vol);

    unsigned int getIdFromNodeType(const std::string &nodeType);

    void storePublishedNodes(GeoPublisher *store);
    template <typename TT>
    void storeRecordPublishedNodes(
        const GeoPublisher::RecordMap_t<TT> &storeMap,
        DBRowsList& cachePublishedNodes);

    void storePublishedAuxiliaryData(GeoPublisher *store);

    std::string m_dbpath;
    GMDBManager *m_dbManager;

    // work caches
    std::set<std::string> m_linkSet;
    std::unordered_map<std::string, unsigned int> m_parentChildrenMap;
    std::unordered_map<std::string, unsigned int> m_volumeCopiesMap;
    std::unordered_map<std::string, unsigned int> m_surfaceCopiesMap;
    std::unordered_map<std::string, unsigned int> m_memMap;
    std::unordered_map<std::string, unsigned int> m_memMap_Tables;

    // keep track of the number of visited tree node
    unsigned int m_len;
    unsigned int m_len_nChild;

    // store True if we have found the Root volume
    bool m_rootVolumeFound;

    // store True if we are visiting an unconnected tree
    // (for example, one used in a GeoSerialTransformer)
    bool m_unconnectedTree;

    // caches for GeoModel nodes to be saved into the DB
    DBRowsList m_logVols;
    DBRowsList m_physVols;
    DBRowsList m_fullPhysVols;
    DBRowsList m_nameTags;
    DBRowsList m_identifierTags;
    DBRowsList m_serialIdentifiers;
    DBRowsList m_serialDenominators;
    DBRowsList m_transforms;
    DBRowsList m_alignableTransforms;
    DBRowsList m_serialTransformers;
    // Elements and Material (with data)
    DBRowsList m_elements;
    DBRowsList m_materials;
    DBRowsList m_materials_Data;
    // Shapes
    DBRowsList m_shapes_Box;
    DBRowsList m_shapes_EllipticalTube;
    DBRowsList m_shapes_Tube;
    DBRowsList m_shapes_Cons;
    DBRowsList m_shapes_Para;
    DBRowsList m_shapes_Trap;
    DBRowsList m_shapes_Trd;
    DBRowsList m_shapes_Tubs;
    DBRowsList m_shapes_Torus;
    DBRowsList m_shapes_TwistedTrap;
    // Shapes with data
    DBRowsList m_shapes_Pcon;
    DBRowsList m_shapes_Pcon_Data;
    DBRowsList m_shapes_Pgon;
    DBRowsList m_shapes_Pgon_Data;
    DBRowsList m_shapes_SimplePolygonBrep;
    DBRowsList m_shapes_SimplePolygonBrep_Data;
    DBRowsList m_shapes_GenericTrap;
    DBRowsList m_shapes_GenericTrap_Data;
    // an unidentified shape
    DBRowsList m_shapes_UnidentifiedShape;
    // boolean operator nodes
    DBRowsList m_shapes_Shift;
    DBRowsList m_shapes_Intersection;
    DBRowsList m_shapes_Subtraction;
    DBRowsList m_shapes_Union;
    // Surface nodes
    DBRowsList m_rectangle_surface;  // For Virtual Surface Shape
    DBRowsList m_trapezoid_surface;
    DBRowsList m_annulus_surface;
    DBRowsList m_diamond_surface;
    DBRowsList m_VSurface;           // For Virtual Surface Abstract Class
    // Functions and functions' data
    DBRowsList m_functions; // operators used in Function's expression
    DBRowEntry m_exprData; // numbers used in Function's expression
    
    // caches for Metadata to be saved into the DB
    std::pair<std::string, unsigned> m_rootVolume;

    // All the connections between nodes
    DBRowsList m_childrenPositions;

    // Published lists of FullPhysVol and AlignableTransform nodes
    // (They are used later, in the ReadoutGeometry)
    DBRowsList m_publishedAlignableTransforms;
    DBRowsList m_publishedFullPhysVols;

    // cache to store custom tables to store auxiliary data in the DB:
    // ---> map( tableName, columnsNames, columnsTypes )
    AuxTableDefs m_auxiliaryTablesVar;
    AuxTableData m_auxiliaryTablesVarData;

    // cache to store the node that could not have persistified.
    // Usually, that means that persistification code has not been developed
    // for the particular GeoModel node
    std::vector<std::string> m_objectsNotPersistified;

    // inpect-only mode, no DB writing
    // it is set to 'true' when the overloaded 'inspect' constructor is called
    // instead of the 'db' constructor
    bool m_inspect{0};

    /// Stores the loglevel, the verbosity of the output messages
    unsigned m_loglevel{0};
};

} /* namespace GeoModelIO */
#endif  // GeoModelWrite_WriteGeoModel_H

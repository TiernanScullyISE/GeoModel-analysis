/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes.h
 *
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BUILDGEOSHAPES_H
#define GEOMODELREAD_BUILDGEOSHAPES_H

#include "GeoModelDBManager/definitions.h"

#include "GeoModelRead/GeoIdObjMap.h" 
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"

#include <vector>
#include <unordered_map>


namespace GeoModelIO {

class BuildGeoShapes {

public:
  using GeoShapePtr = GeoIntrusivePtr<GeoShape>;

  virtual ~BuildGeoShapes() = default;

  /** @brief Returns the shape which is registered in the database under the passed ID.
   *         If the shape is not built yet, then it is attempted to construct the shape.
   *  @param id: Database field identifier. */
  GeoShapePtr getShape(const unsigned id);
  // --- print the list of built/cached shapes
  void printBuiltShapes() const;
  /** @brief Returns the number of already constructed shapes */
  std::size_t size() const;
  /** @brief Returns the shape type */
  const std::string& type() const;
protected:
  /** @brief Interface method to construct a new shape */
  virtual void buildShape(const DBRowEntry row) = 0;
   /** @brief Constructor specifying the shape type and taking the primary
   *         data table for all shape Instances. I.e. the defining parameters
   *  @param shapeType: Name of the shape to construct
   *  @param primaryData: The full database readout for all defining parameters of the shapes */
  BuildGeoShapes(std::string_view shapeType, DBRowsList&& primaryData);
  /** @brief Constructor specifying the shape type together with the primary data and 
   *         also the auxillary shape data, which is used for shapes with vertices. E.g. SimplePolygonBrep
   *  @param shapeType: Name of the shape types constructed by this instance
   *  @param primaryData: The full database readout for all primary defining parameters of the shapes
   *  @param auxillaryData: Complementary data needed to construct the shapes */
  BuildGeoShapes(std::string_view shapeType, 
                 DBRowsList&& primaryData,
                 DBRowsList&& auxillaryData);

  // --- methods for caching GeoShape nodes ---
  void storeBuiltShape(const unsigned id, GeoShapePtr&& nodePtr);
private:
  std::string m_shapeType{};
  GeoIdObjMap<GeoShapePtr> m_memMapShapes{};
  DBRowsList m_primaryData{};
  std::atomic<bool> m_printed{false};
protected:
  DBRowsList m_auxillaryData{};
};

}
#endif

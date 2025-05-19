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

#include <vector>
#include <variant>
#include <string>
#include <unordered_map>

class GeoShape;

class BuildGeoShapes
{
protected:
  std::unordered_map<unsigned, GeoShape *> m_memMapShapes{};
  std::string m_shapeType;
  DBRowsList m_shape_data;

public:
  //! contructors
  BuildGeoShapes(std::string_view shapeType, const unsigned size);
  BuildGeoShapes(std::string_view shapeType, const unsigned size, const DBRowsList& shapesData);

  virtual void buildShape(const DBRowEntry row) = 0;

  // --- methods for caching GeoShape nodes ---
  void storeBuiltShape(const unsigned id, GeoShape *nodePtr);
  bool isBuiltShape(const unsigned id);
  GeoShape *getBuiltShape(const unsigned id);
  // --- print the list of built/cached shapes
  void printBuiltShapes();
};


#endif

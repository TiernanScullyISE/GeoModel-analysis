/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoVSurfaces.h
 *
 * Created on: June, 2024
 * Author: Rui XUE <r.xue@cern.ch><rux23@pitt.edu>
 *
 */

#ifndef GEOMODELREAD_BUILDGEOVSURFACE_H
#define GEOMODELREAD_BUILDGEOVSURFACE_H

#include "GeoModelDBManager/definitions.h"
#include "GeoModelHelpers/variantHelpers.h"

#include "GeoModelKernel/GeoVSurfaceShape.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelRead/GeoIdObjMap.h"
#include <iostream>
#include <unordered_map>

namespace GeoModelIO {
/** @brief Interface factory class to construct the Virtual surface shapes from the defininig parameters stored in the
 *         database. The shapes are constructed on demand, meaning a shape of ID needs to be requested first. */
class BuildGeoVSurface {
    public:
      /** @param Standard constructor taking all database entries to construct the shapes
       *  @param surfaceRecords: R-value list of defining parameters. */
      BuildGeoVSurface(DBRowsList&& surfaceRecords);
      /** @brief Returns a surface shape which is registered under dbID in the surfaceRecords
       *         If the shape does not exist yet, it's attempted to construct it. The dbID 
       *         must not exceed the number of database records passed during class instance 
       *         construction. */
      GeoIntrusivePtr<GeoVSurfaceShape> getSurface(const unsigned int dbID);
      /** @brief Returns the number of already constructed shapes */
      std::size_t size() const;
  protected:
       /** @brief Interface function to instantiate the construction of a new GeoShape
        *  @param row: List of all definint parameters needed to construct the shape */
       virtual void buildSurface(const DBRowEntry row) = 0;
       /** @brief Add the parsed surface to the internal cache of the class
        *  @param dbID: Identifier under which the surface shape is registered
        *  @param surface: Pointer to the constructed surface shape to cache */
      void storeNewSurface(unsigned int dbID, GeoIntrusivePtr<GeoVSurfaceShape>&& surface);
  private:
      DBRowsList m_surfaceRecords{};
      GeoIdObjMap<GeoIntrusivePtr<GeoVSurfaceShape>> m_memCache{};
};
}
#endif

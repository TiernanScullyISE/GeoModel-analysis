/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes_SimplePolygonBrep.h
 *
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BuildGeoShapes_SimplePolygonBrep_H
#define GEOMODELREAD_BuildGeoShapes_SimplePolygonBrep_H

#include "GeoModelRead/BuildGeoShapes.h"

#include "GeoModelDBManager/definitions.h"

#include <vector>
#include <variant>

namespace GeoModelIO {

class BuildGeoShapes_SimplePolygonBrep : public BuildGeoShapes {
  public:
      /** @brief Constructor taking all defined entries in the database
        *         to construct SimplePolygonBreps */
      BuildGeoShapes_SimplePolygonBrep(DBRowsList&& allPolyBrepData,
                                       DBRowsList&& allVertexData);
  private:
    void buildShape(const DBRowEntry row) override;
};
}
#endif

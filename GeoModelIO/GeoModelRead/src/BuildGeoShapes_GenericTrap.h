/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes_GenericTrap.h
 *
 * Created on: June, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BuildGeoShapes_GenericTrap_H
#define GEOMODELREAD_BuildGeoShapes_GenericTrap_H

#include "GeoModelRead/BuildGeoShapes.h"

#include "GeoModelDBManager/definitions.h"

#include <vector>
#include <variant>

namespace GeoModelIO {
class BuildGeoShapes_GenericTrap : public BuildGeoShapes {
    public:
      /** @brief Constructor taking all defined entries in the database
       *         to construct GenericTrapezoids */
      BuildGeoShapes_GenericTrap(DBRowsList&& allTrapData,
                                 DBRowsList&& allVertexData);
    private:
      void buildShape(const DBRowEntry row) override;
};
}
#endif

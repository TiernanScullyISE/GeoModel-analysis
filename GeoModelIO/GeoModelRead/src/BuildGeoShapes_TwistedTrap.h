/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes_TwistedTrap.h
 *
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BuildGeoShapes_TwistedTrap_H
#define GEOMODELREAD_BuildGeoShapes_TwistedTrap_H

#include "GeoModelRead/BuildGeoShapes.h"

#include <vector>
#include <variant>

namespace GeoModelIO {

class BuildGeoShapes_TwistedTrap : public BuildGeoShapes {
    public:
      /** @brief Constructor taking all defined entries in the database
       *         to construct GeoTwistedTrapezoids */  
      BuildGeoShapes_TwistedTrap(DBRowsList&& allTwistedTrapData);
    private:
      void buildShape(const DBRowEntry row) override;
};
}

#endif

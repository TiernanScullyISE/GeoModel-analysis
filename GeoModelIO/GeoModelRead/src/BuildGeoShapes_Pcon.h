/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes_Pcon.h
 *
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BuildGeoShapes_Pcon_H
#define GEOMODELREAD_BuildGeoShapes_Pcon_H

#include "GeoModelRead/BuildGeoShapes.h"

#include "GeoModelDBManager/definitions.h"

#include <vector>
#include <variant>

namespace GeoModelIO {

class BuildGeoShapes_Pcon : public BuildGeoShapes {
    public:
      /** @brief Constructor taking all defined entries in the database
       *         to construct GeoPcons */
      BuildGeoShapes_Pcon(DBRowsList&& allPconData,
                          DBRowsList&& allVertexData);  
    private:
      void buildShape(const DBRowEntry row) override;
};
}
#endif

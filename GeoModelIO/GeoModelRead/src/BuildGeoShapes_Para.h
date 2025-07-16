/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes_Para.h
 *
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BuildGeoShapes_Para_H
#define GEOMODELREAD_BuildGeoShapes_Para_H

#include "GeoModelRead/BuildGeoShapes.h"

#include <vector>
#include <variant>
#include <string>

namespace GeoModelIO {

class BuildGeoShapes_Para : public BuildGeoShapes {
    public:
      /** @brief Constructor taking all defined entries in the database
       *         to construct GeoParas */  
       BuildGeoShapes_Para(DBRowsList&& allGeoParasData);
    private:
       void buildShape(const DBRowEntry row) override;
};
}
#endif

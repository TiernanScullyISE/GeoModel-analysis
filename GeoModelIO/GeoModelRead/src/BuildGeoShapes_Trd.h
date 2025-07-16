/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes_Trd.h
 *
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BuildGeoShapes_Trd_H
#define GEOMODELREAD_BuildGeoShapes_Trd_H

#include "GeoModelRead/BuildGeoShapes.h"

#include <vector>
#include <variant>
#include <string>

namespace GeoModelIO {

class BuildGeoShapes_Trd : public BuildGeoShapes {
    public:
      /** @brief Constructor taking all defined entries in the database
       *         to construct GeoTrds */  
      BuildGeoShapes_Trd(DBRowsList&& allTrdData);    
   private:
      void buildShape(const DBRowEntry row) override;
};

}
#endif

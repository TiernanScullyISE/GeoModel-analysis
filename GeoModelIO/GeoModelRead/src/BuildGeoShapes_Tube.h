/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes_Tube.h
 *
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BuildGeoShapes_Tube_H
#define GEOMODELREAD_BuildGeoShapes_Tube_H

#include "GeoModelRead/BuildGeoShapes.h"

#include <vector>
#include <variant>

namespace GeoModelIO {

class BuildGeoShapes_Tube : public BuildGeoShapes {
  public:
      /** @brief Constructor taking all defined entries in the database
       *         to construct GeoTubes */  
      BuildGeoShapes_Tube(DBRowsList&& allTubeData); 
  private: 
      void buildShape(const DBRowEntry row) override;
};
}
#endif

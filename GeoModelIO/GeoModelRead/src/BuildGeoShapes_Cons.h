/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes_Cons.h
 *
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BuildGeoShapes_Cons_H
#define GEOMODELREAD_BuildGeoShapes_Cons_H

#include "GeoModelRead/BuildGeoShapes.h"

#include <vector>
#include <variant>
#include <string>

namespace GeoModelIO {

class BuildGeoShapes_Cons : public BuildGeoShapes {
    public:
      /** @brief Constructor taking all defined entries in the database
       *         to construct GeoCons */
      BuildGeoShapes_Cons(DBRowsList&& allConsData); 
    private:
      void buildShape(const DBRowEntry row) override;
};
}
#endif

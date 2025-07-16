/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes_EllipticalTube.h
 *
 * Created on: Oct 3, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BUILDGEOSHAPES_EllipticalTube_H
#define GEOMODELREAD_BUILDGEOSHAPES_EllipticalTube_H

#include "GeoModelRead/BuildGeoShapes.h"

#include <vector>
#include <variant>

namespace GeoModelIO {

class BuildGeoShapes_EllipticalTube : public BuildGeoShapes {
    public:
      /** @brief Constructor taking all defined entries in the database
       *         to construct GeoEllipticalTubes */
      BuildGeoShapes_EllipticalTube(DBRowsList&& allTubeData);
    private:
      void buildShape(const DBRowEntry row) override;
};
}
#endif

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes_UnidentifiedShape.h
 *
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BUILDGEOSHAPES_UnidentifiedShape_H
#define GEOMODELREAD_BUILDGEOSHAPES_UnidentifiedShape_H

#include "GeoModelRead/BuildGeoShapes.h"

#include <vector>
#include <variant>

namespace GeoModelIO {

class BuildGeoShapes_UnidentifiedShape : public BuildGeoShapes {
    public:
      BuildGeoShapes_UnidentifiedShape(DBRowsList&& allUnidentShapeData);
    private:
      void buildShape(const DBRowEntry row) override;
};
}

#endif

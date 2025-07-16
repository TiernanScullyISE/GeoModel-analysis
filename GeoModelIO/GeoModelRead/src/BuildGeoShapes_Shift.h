/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoShapes_Box.h
 *
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 */

#ifndef GEOMODELREAD_BUILDGEOSHAPES_SHFIT_H
#define GEOMODELREAD_BUILDGEOSHAPES_SHFIT_H

#include "GeoModelRead/BuildGeoShapes.h"

#include <vector>
#include <variant>

namespace GeoModelIO {
  class ReadGeoModel;
  class BuildGeoShapes_Shift : public BuildGeoShapes {
    public:
      /** @brief Constructor taking all defined entries in the database
       *         to construct GeoShapeShifts */
      BuildGeoShapes_Shift(const ReadGeoModel* parent,
                           DBRowsList&& allShiftData);
    private:
      const ReadGeoModel* m_parent{};
      void buildShape(const DBRowEntry row) override;
};

}

#endif

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: June, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_Torus.h"

#include "GeoModelKernel/GeoTorus.h"
#include "GeoModelHelpers/variantHelpers.h"

#include <vector>
#include <iostream>

namespace GeoModelIO {

BuildGeoShapes_Torus::BuildGeoShapes_Torus(DBRowsList&& allToriData):
    BuildGeoShapes{GeoTorus::getClassType(), std::move(allToriData)}{}

void BuildGeoShapes_Torus::buildShape(const DBRowEntry row) {
  // === get shape numeric data from the DB row
  // shape ID
  const int shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Torus:shapeID");
  // shape volume
  const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Torus:shapeVolume");
  // shape parameters
  const double RMin = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "Torus:RMin");
  const double RMax = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "Torus:RMax");
  const double RTor = GeoModelHelpers::variantHelper::getFromVariant_Double(row[4], "Torus:RTor");
  const double SPhi = GeoModelHelpers::variantHelper::getFromVariant_Double(row[5], "Torus:SPhi");
  const double DPhi = GeoModelHelpers::variantHelper::getFromVariant_Double(row[6], "Torus:DPhi");
  
  auto shape = make_intrusive<GeoTorus>(RMin, RMax, RTor, SPhi, DPhi);

  storeBuiltShape(shapeId, shape);
}
}
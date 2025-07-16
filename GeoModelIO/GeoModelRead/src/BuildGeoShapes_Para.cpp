/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_Para.h"

#include "GeoModelKernel/GeoPara.h"
#include "GeoModelHelpers/variantHelpers.h"

#include <vector>
#include <iostream>

namespace GeoModelIO {

BuildGeoShapes_Para::BuildGeoShapes_Para(DBRowsList&& allGeoParasData):
  BuildGeoShapes{GeoPara::getClassType(), std::move(allGeoParasData)}{}
void BuildGeoShapes_Para::buildShape(const DBRowEntry row) {
  // === get shape numeric data from the DB row
  // shape ID
  const int shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Para:shapeID");
  // shape volume
  const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Para:shapeVolume");

  // shape parameters
  const double XHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "Para:XHalfLength");
  const double YHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "Para:YHalfLength");
  const double ZHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[4], "Para:ZHalfLength");
  const double Alpha = GeoModelHelpers::variantHelper::getFromVariant_Double(row[5], "Para:Alpha");
  const double Theta = GeoModelHelpers::variantHelper::getFromVariant_Double(row[6], "Para:Theta");
  const double Phi = GeoModelHelpers::variantHelper::getFromVariant_Double(row[7], "Para:Phi");

  auto shape = make_intrusive<GeoPara>(XHalfLength, YHalfLength, ZHalfLength, Alpha, Theta, Phi);
  storeBuiltShape(shapeId, std::move(shape));
}
}
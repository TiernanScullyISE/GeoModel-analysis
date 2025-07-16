/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_Trap.h"

#include "GeoModelKernel/GeoTrap.h"
#include "GeoModelHelpers/variantHelpers.h"

#include <vector>
#include <iostream>

namespace GeoModelIO {

BuildGeoShapes_Trap::BuildGeoShapes_Trap(DBRowsList&& allTrapData):
  BuildGeoShapes{GeoTrap::getClassType(), std::move(allTrapData)}{}

void BuildGeoShapes_Trap::buildShape(const DBRowEntry row) {
  // === get shape numeric data from the DB row
  // shape ID
  const int shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Trap:shapeID");
  // shape volume
  const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Trap:shapeVolume");
  // shape parameters
  const double ZHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "Trap:ZHalfLength");
  const double Theta = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "Trap:Theta");
  const double Phi = GeoModelHelpers::variantHelper::getFromVariant_Double(row[4], "Trap:Phi");
  const double Dydzn = GeoModelHelpers::variantHelper::getFromVariant_Double(row[5], "Trap:Dydzn");
  const double Dxdyndzn = GeoModelHelpers::variantHelper::getFromVariant_Double(row[6], "Trap:Dxdyndzn");
  const double Dxdypdzn = GeoModelHelpers::variantHelper::getFromVariant_Double(row[7], "Trap:Dxdypdzn");
  const double Angleydzn = GeoModelHelpers::variantHelper::getFromVariant_Double(row[8], "Trap:Angleydzn");
  const double Dydzp = GeoModelHelpers::variantHelper::getFromVariant_Double(row[9], "Trap:Dydzp");
  const double Dxdyndzp = GeoModelHelpers::variantHelper::getFromVariant_Double(row[10], "Trap:Dxdyndzp");
  const double Dxdypdzp = GeoModelHelpers::variantHelper::getFromVariant_Double(row[11], "Trap:Dxdypdzp");
  const double Angleydzp = GeoModelHelpers::variantHelper::getFromVariant_Double(row[12], "Trap:Angleydzp");

  auto shape = make_intrusive<GeoTrap>(ZHalfLength, Theta, Phi, Dydzn, Dxdyndzn, Dxdypdzn,
                                       Angleydzn, Dydzp, Dxdyndzp, Dxdypdzp, Angleydzp);

  storeBuiltShape(shapeId, std::move(shape));
}
}
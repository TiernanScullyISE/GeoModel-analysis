/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_TwistedTrap.h"

#include "GeoModelKernel/GeoTwistedTrap.h"
#include "GeoModelHelpers/variantHelpers.h"

#include <vector>
#include <iostream>


namespace GeoModelIO {

BuildGeoShapes_TwistedTrap::BuildGeoShapes_TwistedTrap(DBRowsList&& allTwistedTrapData):
    BuildGeoShapes{GeoTwistedTrap::getClassType(), std::move(allTwistedTrapData)}{}
void BuildGeoShapes_TwistedTrap::buildShape(const DBRowEntry row) {
  // === get shape numeric data from the DB row
  // shape ID
  const int shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "TwistedTrap:shapeID");
  // shape volume
  const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "TwistedTrap:shapeVolume");
  // shape parameters
  const double PhiTwist = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "TwistedTrap:PhiTwist");
  const double ZHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "TwistedTrap:ZHalfLength");
  const double Theta = GeoModelHelpers::variantHelper::getFromVariant_Double(row[4], "TwistedTrap:Theta");
  const double Phi = GeoModelHelpers::variantHelper::getFromVariant_Double(row[5], "TwistedTrap:Phi");
  const double DY1HalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[6], "TwistedTrap:DY1HalfLength");
  const double DX1HalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[7], "TwistedTrap:DX1HalfLength");
  const double DX2HalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[8], "TwistedTrap:DX2HalfLength");
  const double DY2HalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[9], "TwistedTrap:DY2HalfLength");
  const double DX3HalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[10], "TwistedTrap:DX3HalfLength");
  const double DX4HalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[11], "TwistedTrap:DX4HalfLength");
  const double DTiltAngleAlpha = GeoModelHelpers::variantHelper::getFromVariant_Double(row[12], "TwistedTrap:DTiltAngleAlpha");

  auto shape = make_intrusive<GeoTwistedTrap>(PhiTwist, ZHalfLength, Theta, Phi, DY1HalfLength,
                                              DX1HalfLength, DX2HalfLength, DY2HalfLength,
                                              DX3HalfLength, DX4HalfLength, DTiltAngleAlpha);

  storeBuiltShape(shapeId, std::move(shape));
}
}
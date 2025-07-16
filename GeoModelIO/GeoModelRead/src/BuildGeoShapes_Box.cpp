/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_Box.h"

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelHelpers/variantHelpers.h"

#include <vector>
#include <iostream>

namespace GeoModelIO {

BuildGeoShapes_Box::BuildGeoShapes_Box(DBRowsList&& allBoxData):
    BuildGeoShapes{GeoBox::getClassType(), std::move(allBoxData)} {}
 
void BuildGeoShapes_Box::buildShape(const DBRowEntry row) {
    // === get shape numeric data from the DB row
    // shape ID
    const unsigned shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Box:shapeID");
    // shape volume
    const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Box:shapeVolume");
    // shape parameters
    const double XHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "Box:XHalfLength");
    const double YHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "Box:YHalfLength");
    const double ZHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[4], "Box:ZHalfLength");

    auto shape = make_intrusive<GeoBox>(XHalfLength, YHalfLength, ZHalfLength);

    storeBuiltShape(shapeId, std::move(shape));

}
}
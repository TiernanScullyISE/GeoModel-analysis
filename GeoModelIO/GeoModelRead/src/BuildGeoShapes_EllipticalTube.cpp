/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_EllipticalTube.h"

#include "GeoModelKernel/GeoEllipticalTube.h"
#include "GeoModelHelpers/variantHelpers.h"

#include <vector>
#include <iostream>

namespace GeoModelIO {

BuildGeoShapes_EllipticalTube::BuildGeoShapes_EllipticalTube(DBRowsList&& allTubeData):
    BuildGeoShapes{GeoEllipticalTube::getClassType(), std::move(allTubeData)}{}
void BuildGeoShapes_EllipticalTube::buildShape(const DBRowEntry row) {
    // === get shape numeric data from the DB row
    // shape ID
    const unsigned shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "EllipticalTube:shapeID");
    // shape volume
    const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "EllipticalTube:shapeVolume");
    // shape parameters
    const double XHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "EllipticalTube:XHalfLength");
    const double YHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "EllipticalTube:YHalfLength");
    const double ZHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[4], "EllipticalTube:ZHalfLength");

    auto shape = make_intrusive<GeoEllipticalTube>(XHalfLength, YHalfLength, ZHalfLength);

    storeBuiltShape(shapeId, std::move(shape));
}
}
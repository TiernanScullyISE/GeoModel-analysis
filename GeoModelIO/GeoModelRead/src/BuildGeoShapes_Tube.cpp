/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_Tube.h"

#include "GeoModelKernel/GeoTube.h"
#include "GeoModelHelpers/variantHelpers.h"

#include <vector>
#include <iostream>


namespace GeoModelIO {

BuildGeoShapes_Tube::BuildGeoShapes_Tube(DBRowsList&& allTubeData):
    BuildGeoShapes{GeoTube::getClassType(), std::move(allTubeData)}{}

void BuildGeoShapes_Tube::buildShape(const DBRowEntry row) {
    // === get shape numeric data from the DB row
    // shape ID
    const int shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Tube:shapeID");
    // shape volume
    const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Tube:shapeVolume");
    // shape parameters
    const double RMin = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "Tube:RMin");
    const double RMax = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "Tube:RMax");
    const double ZHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[4], "Tube:ZHalfLength");

    auto shape = make_intrusive<GeoTube>(RMin, RMax, ZHalfLength);

    storeBuiltShape(shapeId, std::move(shape));
}
}
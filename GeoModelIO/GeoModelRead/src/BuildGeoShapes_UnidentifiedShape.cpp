/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_UnidentifiedShape.h"

#include "GeoModelKernel/GeoUnidentifiedShape.h"
#include "GeoModelHelpers/variantHelpers.h"

#include <vector>
#include <iostream>
namespace GeoModelIO {

BuildGeoShapes_UnidentifiedShape::BuildGeoShapes_UnidentifiedShape(DBRowsList&& allUnidentShapeData):
    BuildGeoShapes(GeoUnidentifiedShape::getClassType(), std::move(allUnidentShapeData)){};

void BuildGeoShapes_UnidentifiedShape::buildShape(const DBRowEntry row) {
    // === get shape numeric data from the DB row
    // shape ID
    const unsigned shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "UnidentifiedShape:shapeID");
    // shape volume
    const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "UnidentifiedShape:shapeVolume");
    // shape parameters
    const std::string name = GeoModelHelpers::variantHelper::getFromVariant_String(row[2], "UnidentifiedShape:name");
    const std::string asciiData = GeoModelHelpers::variantHelper::getFromVariant_String(row[3], "UnidentifiedShape:asciiData");

    auto shape =  make_intrusive<GeoUnidentifiedShape>(name, asciiData);

    storeBuiltShape(shapeId, std::move(shape));
}
}
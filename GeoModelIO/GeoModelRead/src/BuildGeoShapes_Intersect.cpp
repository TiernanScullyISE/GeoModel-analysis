/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_Intersect.h"

#include "GeoModelKernel/GeoShapeIntersection.h"
#include "GeoModelHelpers/variantHelpers.h"
#include "GeoModelHelpers/GeoShapeUtils.h"
#include "GeoModelRead/ReadGeoModel.h"

#include <vector>
#include <iostream>
namespace GeoModelIO {

BuildGeoShapes_Intersect::BuildGeoShapes_Intersect(const ReadGeoModel* parent,
                                                   DBRowsList&& allIntresectData):
    BuildGeoShapes{GeoShapeIntersection::getClassType(), std::move(allIntresectData)},
    m_parent{parent} {}
 
void BuildGeoShapes_Intersect::buildShape(const DBRowEntry row) {
    // === get shape numeric data from the DB row
    // shape ID
    const unsigned shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Intresect::shapeID");
    // computed volume (not defined by default)
    const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Intresect::shapeVolume");
    // shape operands
    const std::string shapeOpAType = GeoModelHelpers::variantHelper::getFromVariant_String(row[2], "Intresect::shapeType1");
    const unsigned shapeOpAId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[3], "Intresect::shapeId1");
    const std::string shapeOpBType = GeoModelHelpers::variantHelper::getFromVariant_String(row[4], "Intresect::shapeType2");
    const unsigned shapeOpBId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[5], "Intresect::shapeId2");

    const auto operandA = m_parent->getBuiltShape(shapeOpAId, shapeOpAType);
    if (!operandA) {
        THROW_EXCEPTION("Failed to fetch a valid shape of type: "<<shapeOpAType<<" & id: "<<shapeOpAId);
    }
    const auto operandB = m_parent->getBuiltShape(shapeOpBId, shapeOpBType);
    if (!operandB) {
        THROW_EXCEPTION("Failed to fetch a valid shape of type: "<<shapeOpBType<<" & id: "<<shapeOpBId);
    }
    auto shape = make_intrusive<GeoShapeIntersection>(operandA, operandB);
    shape->setVolumeValue(shapeVolume);
    storeBuiltShape(shapeId, shape);
}
}
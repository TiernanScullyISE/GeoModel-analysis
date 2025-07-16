/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_Union.h"

#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelHelpers/variantHelpers.h"
#include "GeoModelHelpers/GeoShapeUtils.h"
#include "GeoModelRead/ReadGeoModel.h"

#include <vector>
#include <iostream>
namespace GeoModelIO {

BuildGeoShapes_Union::BuildGeoShapes_Union(const ReadGeoModel* parent,
                                           DBRowsList&& allUnionData):
    BuildGeoShapes{GeoShapeUnion::getClassType(), std::move(allUnionData)},
    m_parent{parent} {}
 
void BuildGeoShapes_Union::buildShape(const DBRowEntry row) {
    // === get shape numeric data from the DB row
    // shape ID
    const unsigned shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Union::shapeID");
    // computed volume (not defined by default)
    const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Union::shapeVolume");
    // shape operands
    const std::string shapeOpAType = GeoModelHelpers::variantHelper::getFromVariant_String(row[2], "Union::shapeType1");
    const unsigned shapeOpAId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[3], "Union::shapeId1");
    const std::string shapeOpBType = GeoModelHelpers::variantHelper::getFromVariant_String(row[4], "Union::shapeType2");
    const unsigned shapeOpBId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[5], "Union::shapeId2");

    const auto operandA = m_parent->getBuiltShape(shapeOpAId, shapeOpAType);
    if (!operandA) {
        THROW_EXCEPTION("Failed to fetch a valid shape of type: "<<shapeOpAType<<" & id: "<<shapeOpAId);
    }
    const auto operandB = m_parent->getBuiltShape(shapeOpBId, shapeOpBType);
    if (!operandB) {
        THROW_EXCEPTION("Failed to fetch a valid shape of type: "<<shapeOpBType<<" & id: "<<shapeOpBId);
    }
    auto shape = make_intrusive<GeoShapeUnion>(operandA, operandB);
    shape->setVolumeValue(shapeVolume);
    storeBuiltShape(shapeId, shape);
}
}

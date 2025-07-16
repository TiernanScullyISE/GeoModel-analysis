/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_Shift.h"

#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelHelpers/variantHelpers.h"
#include "GeoModelHelpers/GeoShapeUtils.h"
#include "GeoModelRead/ReadGeoModel.h"

#include <vector>
#include <iostream>

namespace GeoModelIO {

BuildGeoShapes_Shift::BuildGeoShapes_Shift(const ReadGeoModel* parent,
                                           DBRowsList&& allShiftData):
    BuildGeoShapes{GeoShapeShift::getClassType(), std::move(allShiftData)},
    m_parent{parent} {}
 
void BuildGeoShapes_Shift::buildShape(const DBRowEntry row) {
    // === get shape numeric data from the DB row
    // shape ID
    const unsigned shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Shift:shapeID");
    // computed volume (not defined by default)
    const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Shift:shapeVolume");
    // shape parameters
    const std::string shapeOpType = GeoModelHelpers::variantHelper::getFromVariant_String(row[2], "Shift:shapeType");
    const unsigned shapeOpId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[3], "Shift:shapeId");
    const unsigned transfId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[4], "Shift:transformId");

    const auto trf = m_parent->getBuiltTransform(transfId);
    if (!trf) {
        THROW_EXCEPTION("No transform registered unde ID:"<<transfId);
    }
    const auto operand = m_parent->getBuiltShape(shapeOpId, shapeOpType);
    if (!operand) {
        THROW_EXCEPTION("Failed to fetch a valid shape of type: "<<shapeOpType<<" & id: "<<shapeOpId);
    }
    auto shape = make_intrusive<GeoShapeShift>(operand, trf->getTransform());
    shape->setVolumeValue(shapeVolume);
    storeBuiltShape(shapeId, shape);
}
}
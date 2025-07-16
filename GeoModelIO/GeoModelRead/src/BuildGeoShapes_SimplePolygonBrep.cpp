/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_SimplePolygonBrep.h"

#include "GeoModelKernel/GeoSimplePolygonBrep.h"
#include "GeoModelHelpers/variantHelpers.h"
#include "GeoModelKernel/throwExcept.h"

#include <vector>
#include <iostream>

namespace GeoModelIO {

BuildGeoShapes_SimplePolygonBrep::BuildGeoShapes_SimplePolygonBrep(DBRowsList&& allPolyBrepData,
                                                                  DBRowsList&& allVertexData):
    BuildGeoShapes{GeoSimplePolygonBrep::getClassType(), std::move(allPolyBrepData),
                   std::move(allVertexData)}{}
void BuildGeoShapes_SimplePolygonBrep::buildShape(const DBRowEntry row) {
    if (m_auxillaryData.empty()) {
        THROW_EXCEPTION("ERROR! GeoSimplePolygonBrep shape has no ZPlanes data!! [m_shape_data.size() == 0]");
    }

    // === get shape numeric data from the DB row
    // shape ID
    const int shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "SimplePolygonBrep:shapeID");
    // shape volume
    const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "SimplePolygonBrep:shapeVolume");
    // shape parameters
    const double DZ = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "SimplePolygonBrep:DZ");
    const unsigned int NVertices = GeoModelHelpers::variantHelper::getFromVariant_Int(row[3], "SimplePolygonBrep:NVertices");
    // pointers to variable shape data stored in a separate table
    const int dataStart = GeoModelHelpers::variantHelper::getFromVariant_Int(row[4], "SimplePolygonBrep:dataStart");
    const int dataEnd = GeoModelHelpers::variantHelper::getFromVariant_Int(row[5], "SimplePolygonBrep:dataEnd");

    // build the basic GeoSimplePolygonBrep shape
    auto shape = make_intrusive<GeoSimplePolygonBrep>(DZ);

    // and now loop over the additional shape's data,
    // to get the parameters of all vertices

    // extract subvector
    // NOTE: we use (dataStart-1) to cope with the difference between the DB rows starting from '1',
    //       which is what the 'dataStart' stores, and the vector items, which start '0';
    //       also, the constructor of the sub-vector takes the element from 'begin+dataStart-1' included
    //       and 'begin+dataEnd' excluded.
    assert(dataStart > 0 && dataEnd <= m_auxillaryData.size());
    const DBRowsList thisShapeData(m_auxillaryData.begin() + (dataStart - 1),
                                   m_auxillaryData.begin() + (dataEnd));
    if (thisShapeData.empty()) {
        THROW_EXCEPTION("ERROR! GeoSimplePolygonBrep shape ZPlanes data have not been retrieved!!");
    }
    if (NVertices != thisShapeData.size()) {
        THROW_EXCEPTION("ERROR! GeoSimplePolygonBrep shape : size of ZPlanes data does not correspond to the number of ZPlanes declared!!");
    }
    // loop over the data defining the ZPlanes
    for (const DBRowEntry &dataRow : thisShapeData) {
        const double xV = GeoModelHelpers::variantHelper::getFromVariant_Double(dataRow[1], "SimplePolygonBrep:data_xV");
        const double yV = GeoModelHelpers::variantHelper::getFromVariant_Double(dataRow[2], "SimplePolygonBrep:data_yV");
        // add a vertex to the GeoSimplePolygonBrep
        shape->addVertex(xV, yV);
    }

    // sanity checks on the resulting SimplePolygonBrep shape
    if (shape->getNVertices() != NVertices) {
        THROW_EXCEPTION("ERROR! GeoSimplePolygonBrep actual number of vertices: " + std::to_string(shape->getNVertices()) + " is not equal to the original size! --> " + std::to_string(NVertices));
    }
    if (!shape->isValid()) {
        THROW_EXCEPTION("ERROR! GeoSimplePolygonBrep shape is not valid!!");
    }
    storeBuiltShape(shapeId, std::move(shape));
}
}
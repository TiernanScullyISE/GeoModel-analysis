/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_GenericTrap.h"

#include "GeoModelKernel/GeoGenericTrap.h"
#include "GeoModelHelpers/variantHelpers.h"
#include "GeoModelKernel/throwExcept.h"

#include <vector>
#include <iostream>


namespace GeoModelIO {

BuildGeoShapes_GenericTrap::BuildGeoShapes_GenericTrap(DBRowsList&& allTrapData,
                                                      DBRowsList&& allVertexData):
    BuildGeoShapes{GeoGenericTrap::getClassType(), std::move(allTrapData), std::move(allVertexData)}{}


void BuildGeoShapes_GenericTrap::buildShape(const DBRowEntry row){
    if (m_auxillaryData.empty()) {
        THROW_EXCEPTION("ERROR! GeoGenericTrap shape has no ZPlanes data!! [m_shape_data.size() == 0]");
    }

    // === get shape numeric data from the DB row
    // shape ID
    const int shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "GenericTrap:shapeID");
    // shape volume
    const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "GenericTrap:shapeVolume");
    // shape parameters
    const double zHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "GenericTrap:ZHalfLength");
    const unsigned int NVertices = GeoModelHelpers::variantHelper::getFromVariant_Int(row[3], "GenericTrap:NVertices");
    // pointers to variable shape data stored in a separate table
    const int dataStart = GeoModelHelpers::variantHelper::getFromVariant_Int(row[4], "GenericTrap:dataStart");
    const int dataEnd = GeoModelHelpers::variantHelper::getFromVariant_Int(row[5], "GenericTrap:dataEnd");

    // and now loop over the additional shape's data,
    // to get the parameters of all vertices

    //container for GenericTrap vertices
    GeoGenericTrapVertices vertices; 

    // extract subvector
    // NOTE: we use (dataStart-1) to cope with the difference between the DB rows starting from '1',
    //       which is what the 'dataStart' stores, and the vector items, which start '0';
    //       also, the constructor of the sub-vector takes the element from 'begin+dataStart-1' included
    //       and 'begin+dataEnd' excluded.
    assert(dataStart >0 && dataEnd <= m_auxillaryData.size());
    const DBRowsList thisShapeData(m_auxillaryData.begin() + (dataStart - 1),
                                   m_auxillaryData.begin() + (dataEnd));
    if (thisShapeData.empty()) {
        THROW_EXCEPTION("ERROR! GeoGenericTrap shape ZPlanes data have not been retrieved!!");
    }
    if (NVertices != thisShapeData.size()) {
        THROW_EXCEPTION("ERROR! GeoGenericTrap shape : size of ZPlanes data does not correspond to the number of ZPlanes declared!!");
    }
    // loop over the data defining the ZPlanes
    for (const DBRowEntry &dataRow : thisShapeData) {
        const double xV = GeoModelHelpers::variantHelper::getFromVariant_Double(dataRow[1], "GenericTrap:data_xV");
        const double yV = GeoModelHelpers::variantHelper::getFromVariant_Double(dataRow[2], "GenericTrap:data_yV");
        // add a vertex to the collection of all GeoGenericTrap vertices
        vertices.push_back(GeoTwoVector(xV, yV));
    }

    // build the basic GeoGenericTrap shape
    auto shape = make_intrusive<GeoGenericTrap>(zHalfLength, vertices);

    // sanity checks on the resulting GenericTrap shape
    if (shape->getVertices().size() != NVertices) {
        THROW_EXCEPTION("ERROR! GeoGenericTrap actual number of vertices: " + std::to_string(shape->getVertices().size()) + " is not equal to the original size! --> " + std::to_string(NVertices));
    }
   
    storeBuiltShape(shapeId, std::move(shape));
}
}
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes.h"

#include "GeoModelKernel/GeoShape.h"

#include <vector>
#include <iostream>

BuildGeoShapes::BuildGeoShapes(std::string_view shapeType, const unsigned size)
{
    m_shapeType = shapeType;
    m_memMapShapes.reserve(size);
}

BuildGeoShapes::BuildGeoShapes(std::string_view shapeType, const unsigned size, const DBRowsList& shapeData)
{
    m_shapeType = shapeType;
    m_memMapShapes.reserve(size);
    m_shape_data = shapeData;
}

bool BuildGeoShapes::isBuiltShape(const unsigned id) {
    return (!(m_memMapShapes.find(id) == m_memMapShapes.end()));
}
void BuildGeoShapes::storeBuiltShape(const unsigned id, GeoShape* nodePtr) {
    m_memMapShapes[id] = nodePtr;
}
GeoShape* BuildGeoShapes::getBuiltShape(const unsigned id) {
    if (!m_memMapShapes.size()) return nullptr;
    return m_memMapShapes[id];  // this is a map, and 'id' is the key
}

void BuildGeoShapes::printBuiltShapes() {
    for (unsigned id{1}; id<=m_memMapShapes.size(); ++id) {
        std::cout << "shape " << m_shapeType << " -- id: " << id << ", shapePtr: " << m_memMapShapes[id] << std::endl;
    }
}

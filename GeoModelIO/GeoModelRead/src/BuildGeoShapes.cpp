/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "GeoModelRead/BuildGeoShapes.h"

#include "GeoModelKernel/GeoShape.h"
#include "GeoModelHelpers/GeoShapeUtils.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelHelpers/GeoShapeSorter.h"

#include <vector>
#include <iostream>

namespace GeoModelIO {

BuildGeoShapes::BuildGeoShapes(std::string_view shapeType, DBRowsList&& primaryData):
    m_shapeType{shapeType},
    m_primaryData{std::move(primaryData)} {}
 
BuildGeoShapes::BuildGeoShapes(std::string_view shapeType, DBRowsList&& primaryData,
                               DBRowsList&& auxillaryData):
    m_shapeType{shapeType},
    m_primaryData{std::move(primaryData)},
    m_auxillaryData{std::move(auxillaryData)}{}

void BuildGeoShapes::storeBuiltShape(const unsigned id, GeoShapePtr&& nodePtr) {
    if (m_memMapShapes.insert(std::make_pair(id, nodePtr))) {
        return;
    }
    /// Check whether the two shapes are actually different. If not then just ignore
    /// the race conditions
    auto stored = m_memMapShapes.get(id);
    if (GeoShapeSorter{}.compare(stored, nodePtr)) {
        THROW_EXCEPTION("Clash for identifier "<<id<<" detected: "<<std::endl
                        <<"***  stored: "<<printGeoShape(stored)<<std::endl
                        <<"***     new: "<<printGeoShape(nodePtr));
    }
}
std::size_t BuildGeoShapes::size() const { return m_memMapShapes.size(); }
const std::string& BuildGeoShapes::type() const { return m_shapeType; }
BuildGeoShapes::GeoShapePtr 
    BuildGeoShapes::getShape(const unsigned id) {
    if (auto retObj = m_memMapShapes.get(id); retObj != nullptr) {
        return retObj;
    }
    /// By convewntion the primary data is sorted by the shape ID.
    if (id > m_primaryData.size()|| id ==0) {
        THROW_EXCEPTION("Cannot construct shape "<<m_shapeType
            <<" because the ID is out of boundary "<<id<<".");
    }
    const DBRowEntry& entry{m_primaryData[id-1]};
    buildShape(entry);
    /// All shapes have been converted -> free up memory
    if (m_memMapShapes.size() == m_primaryData.size()) {
        // m_primaryData = DBRowsList{};
        // m_auxillaryData = DBRowsList{};
        std::cout<<"All "<<m_memMapShapes.size()<<" "<<m_shapeType<<" have been built. "<<std::endl;
    }
    auto retObj = m_memMapShapes.get(id);
    if (!retObj) {
        std::cout<<__FILE__<<":"<<__LINE__<<"No shape of type "<<m_shapeType<<" is registered for "<<id<<std::endl;
    }
    return retObj;
}

void BuildGeoShapes::printBuiltShapes() const {    
    for (const unsigned id : m_memMapShapes.keys()) {
        std::cout << "shape " << m_shapeType << " -- id: " << id << ", shapePtr: " 
                  << printGeoShape(m_memMapShapes.get(id)) << std::endl;
    }
}
}

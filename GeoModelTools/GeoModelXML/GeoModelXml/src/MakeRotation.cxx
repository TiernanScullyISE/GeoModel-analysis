/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelXml/MakeRotation.h"
#include <xercesc/dom/DOM.hpp>

#include "GeoModelKernel/GeoDefinitions.h"
#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"

#include <array>

MakeRotation::MakeRotation() {}

using namespace xercesc;
using namespace GeoXML;
//  using namespace HepGeom;

GeoTrf::Rotation3D MakeRotation::getTransform(const DOMElement *rotation, GmxUtil &gmxUtil) {

    const std::size_t nParams = 4; 
    static const std::array<std::string, nParams> parName {"angle", "xcos", "ycos", "zcos"};
    const std::array<double, nParams> p{fetchAttributes(gmxUtil,*rotation, parName)};
    return GeoTrf::Rotation3D{GeoTrf::AngleAxis3D(p[0], GeoTrf::Vector3D(p[1], p[2], p[3]))};

}

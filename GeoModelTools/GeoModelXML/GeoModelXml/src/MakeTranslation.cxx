/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelXml/MakeTranslation.h"
#include <xercesc/dom/DOM.hpp>
#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"
#include <array>

#include "GeoModelKernel/GeoDefinitions.h"


using namespace GeoXML;
using namespace xercesc;
//using namespace HepGeom;
GeoTrf::Translate3D MakeTranslation::getTransform(const DOMElement *translation, GmxUtil &gmxUtil) {

    constexpr std::size_t nParams = 3; 
    static const std::array<std::string, nParams> parName {"x", "y", "z"};
    std::array<double, nParams> p{fetchAttributes(gmxUtil,*translation, parName)};
    return GeoTrf::Translate3D(p[0], p[1], p[2]);
}

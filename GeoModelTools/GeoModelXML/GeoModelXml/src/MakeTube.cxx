/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Automatically generated code from /home/hessey/prog/gmx2geo/makeshape
#include "GeoModelXml/shape/MakeTube.h"
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"

#include <array>


using namespace xercesc;
using namespace GeoXML;

GeoIntrusivePtr<RCBase> MakeTube::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
    constexpr std::size_t nParams = 3; 
    static const std::array<std::string, nParams> parName {"rmin", "rmax", "zhalflength"};
    std::array<double, nParams> p{fetchAttributes(gmxUtil, *element, parName)};
    return const_pointer_cast(cacheShape(make_intrusive<GeoTube>(p[0], p[1], p[2])));
}

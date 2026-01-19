/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelXml/shape/MakeEllipticalTube.h"
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/GeoEllipticalTube.h"
#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"
#include <array>

using namespace xercesc;
using namespace GeoXML;


GeoIntrusivePtr<RCBase>MakeEllipticalTube::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
    constexpr std::size_t nParams = 3; 
    static const std::array<std::string, nParams> parName{"xhalflength", "yhalflength", "zhalflength"};
    const std::array<double, nParams> p{fetchAttributes(gmxUtil, *element, parName)};
    return  const_pointer_cast(cacheShape(make_intrusive<GeoEllipticalTube>(p[0], p[1], p[2])));
}

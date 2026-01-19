/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Automatically generated code from /home/hessey/prog/gmx2geo/makeshape
#include "GeoModelXml/shape/MakePara.h"
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/GeoPara.h"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"

#include <array>
using namespace xercesc;
using namespace GeoXML;


GeoIntrusivePtr<RCBase>MakePara::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
    constexpr std::size_t nParams = 6; 
    static const std::array<std::string, nParams> parName{"xhalflength", "yhalflength", "zhalflength", "alpha", "theta", "phi"};
    const std::array<double, nParams> p{fetchAttributes(gmxUtil, *element, parName)};
    return const_pointer_cast(cacheShape(make_intrusive<GeoPara>(p[0], p[1], p[2], p[3], p[4], p[5])));
}

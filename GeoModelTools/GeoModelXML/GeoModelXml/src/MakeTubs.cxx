/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Automatically generated code from /home/hessey/prog/gmx2geo/makeshape
#include "GeoModelXml/shape/MakeTubs.h"
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelXml/StringWrappers.h"
#include "GeoModelXml/GmxUtil.h"

#include <array>

using namespace xercesc;
using namespace GeoXML;

GeoIntrusivePtr<RCBase> MakeTubs::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
    constexpr std::size_t nParams = 5; 
    
    static const std::array<std::string, nParams> parName {"rmin", "rmax", "zhalflength", "sphi", "dphi"};
    const std::array<double, nParams> p{fetchAttributes(gmxUtil, *element, parName)};
    return const_pointer_cast(cacheShape(make_intrusive<GeoTubs>(p[0], p[1], p[2], p[3], p[4])));
}

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
//
// Jun 2021, Riccardo Maria BIANCHI, <riccardo.maria.bianchi@cern.ch>
//
#include "GeoModelXml/shape/MakeTorus.h"
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/GeoTorus.h"
#include "GeoModelXml/StringWrappers.h"
#include "GeoModelXml/GmxUtil.h"

#include <array>

using namespace xercesc;
using namespace GeoXML;


GeoIntrusivePtr<RCBase> MakeTorus::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
  constexpr std::size_t nParams = 5; 
  static const std::array<std::string, nParams> parName {"rmin", "rmax", "rtor", "sphi", "dphi"};
  const std::array<double, nParams> p{fetchAttributes(gmxUtil, *element, parName)};
  return const_pointer_cast(cacheShape(make_intrusive<GeoTorus>(p[0], p[1], p[2], p[3], p[4])));
}

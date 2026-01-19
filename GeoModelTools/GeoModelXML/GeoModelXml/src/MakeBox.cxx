/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Automatically generated code from /home/hessey/prog/gmx2geo/makeshape
#include "GeoModelXml/shape/MakeBox.h"
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/GeoBox.h"
#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"

#include <array>
using namespace GeoXML;
using namespace xercesc;

GeoIntrusivePtr<RCBase> MakeBox::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
  constexpr std::size_t nParams = 3; 
  static const std::array<std::string, nParams> parName{"xhalflength", "yhalflength", "zhalflength"};
  const std::array<double, nParams> p = fetchAttributes(gmxUtil, *element, parName);
  return const_pointer_cast(cacheShape(make_intrusive<GeoBox>(p[0], p[1], p[2])));
}

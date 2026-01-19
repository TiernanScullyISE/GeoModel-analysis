/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelXml/MakeElement.h"
#include "GeoModelXml/StringWrappers.h"
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/Units.h"
#include "GeoModelKernel/GeoElement.h"
#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GmxUtil.h"

using namespace xercesc;
using namespace GeoModelKernelUnits;
using namespace GeoXML;

MakeElement::MakeElement() {}

GeoIntrusivePtr<RCBase>MakeElement::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
  //
  //   Get my Z
  //
  const double zVal = gmxUtil.evaluate(fetchAttribute(*element, "Z"));
  //
  //   Get my A
  //
  const double aVal = gmxUtil.evaluate(fetchAttribute(*element, "A"));
  //
  //    Get my names
  //
  const std::string name = fetchAttribute(*element, "name");
  const std::string shortname = fetchAttribute(*element, "shortname");
  //
  //    Create it
  //
  //aVal *= gram/mole;

  GeoIntrusivePtr<const GeoElement> el{};
  if (gmxUtil.matManager) {
  	  gmxUtil.matManager->addElement(name, shortname, zVal, aVal);
	    el=gmxUtil.matManager->getElement(name);    
  } else {
  	  el = make_intrusive<GeoElement>(name, shortname, zVal, aVal*gram/mole);
  }
  return const_pointer_cast(el);
}

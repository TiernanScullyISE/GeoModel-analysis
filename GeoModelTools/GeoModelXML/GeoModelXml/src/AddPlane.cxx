/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelXml/shape/AddPlane.h"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"
#include <string>
#include <sstream>
#include <iostream>

#include <xercesc/dom/DOM.hpp>


using namespace GeoXML;
void AddPlane::process(const xercesc::DOMElement *element, double &zPlane, double &rMinPlane, double &rMaxPlane) {
  if (!gmxUtil) {
	  std::cout<<"This is AddPlane::process()!! gmxUtil is 0!!!!!"<<std::endl;
	  return;
  }
  zPlane=gmxUtil->evaluate(fetchAttribute(*element, "zplane"));
  rMinPlane=gmxUtil->evaluate(fetchAttribute(*element, "rminplane"));
  rMaxPlane=gmxUtil->evaluate(fetchAttribute(*element, "rmaxplane"));
}

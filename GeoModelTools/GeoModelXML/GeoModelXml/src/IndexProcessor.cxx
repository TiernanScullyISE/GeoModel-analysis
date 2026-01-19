/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//
//   Processor for index elements
//
#include <map>

#include <xercesc/dom/DOM.hpp>
#include "GeoModelXml/IndexProcessor.h"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"

using namespace GeoXML;

void IndexProcessor::process(const xercesc::DOMElement *element, GmxUtil &gmxUtil, GeoNodeList &/* toAdd */) {
    const std::string name{fetchAttribute(*element, "ref")};
    const std::string value{fetchAttribute(*element, "value")};
    gmxUtil.positionIndex.setFormula(name, value);
}

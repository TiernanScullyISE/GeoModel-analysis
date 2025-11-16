/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//
//   Processor for addbranch elements
//
//   Process children and get list of things to be added to the tree.
//
#include "GeoModelXml/AddbranchProcessor.h"
#include "GeoModelXml/StringWrappers.h"
#include <map>

#include <xercesc/dom/DOM.hpp>
#include "GeoModelXml/ProcessorRegistry.h"

#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GmxUtil.h"

using namespace std;
using namespace xercesc;

void AddbranchProcessor::process(const xercesc::DOMElement *element, GmxUtil &gmxUtil, GeoNodeList &toAdd) {

    for (DOMNode *child = element->getFirstChild(); child != nullptr; child = child->getNextSibling()) {
        if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
            gmxUtil.processorRegistry.find(GeoXML::nodeName(*child))->process(dynamic_cast<const DOMElement *>(child), gmxUtil, toAdd);
        }
    }
}

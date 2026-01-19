/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//
//   Example element processor. This is useful for debugging etc: it helps 
//   get all elements processed, just printing out their name. Including 
//   children of referenced tags. Can be used for example as the default element
//   processor in the processor registry. 
//
//   Parameters: 
//       The element to be processed;
//       The GmxUtil for access to the Evaluator etc.
//       A vector of *GeoGraphNode to add things to that need to be added to the GeoModel tree 
//
//
#include "GeoModelXml/ElementProcessor.h"

#include "OutputDirector.h"
#include <string>
#include "xercesc/util/XercesDefs.hpp"
#include <xercesc/dom/DOM.hpp>


#include "GeoModelXml/GeoNodeList.h"
#include "GeoModelXml/ProcessorRegistry.h"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"


using namespace GeoXML;
using namespace xercesc;

void ElementProcessor::process(const DOMElement *element, GmxUtil & /* gmxUtil*/, 
                               GeoNodeList & /* toAdd */) {
    THROW_EXCEPTION("Default element processor called for tag-name " << nodeName(*element));
}

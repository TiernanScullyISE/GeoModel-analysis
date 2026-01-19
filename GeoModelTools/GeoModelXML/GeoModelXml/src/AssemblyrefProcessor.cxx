/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//
//    Process assemblyref items: basically, just find the referenced assembly and call its processor.
//
#include "OutputDirector.h"
#include "GeoModelXml/AssemblyrefProcessor.h"
#include "GeoModelXml/StringWrappers.h"
#include "GeoModelKernel/throwExcept.h"

#include "xercesc/util/XercesDefs.hpp"
#include <xercesc/dom/DOM.hpp>


#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/GeoNodeList.h"

using namespace GeoXML;
using namespace xercesc;
void AssemblyrefProcessor::process(const DOMElement *element, GmxUtil &gmxUtil, GeoNodeList &toAdd) {
    XMLCh *ref = XMLString::transcode("ref");
    const XMLCh *idref{nullptr};
    DOMDocument *doc = element->getOwnerDocument();
    //
    //    Get the referenced element
    //
    idref = element->getAttribute(ref);
    DOMElement *elem = doc->getElementById(idref);
    XMLString::release(&ref);
    //
    //    Check it is the right sort
    //
    if (nodeName(*elem) != "assembly" && nodeName(*elem) != "set") {
        THROW_EXCEPTION("Error in xml/gmx file: assemblyref " << XMLString::transcode(idref) 
                    << " referenced a " << nodeName(*elem) << " instead of an assembly.\n");
    }
    //
    //    Process it
    //
    const std::string zeroid = fetchAttribute(*element,"zeroid");
    if (zeroid == "true") {
        gmxUtil.tagHandler.assembly.zeroId(elem);
    }
    gmxUtil.tagHandler.assembly.process(elem, gmxUtil, toAdd);
}

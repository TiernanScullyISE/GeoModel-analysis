/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//
//    Process transformationref items: basically, just find the referenced transform and call its processor.
//
#include "GeoModelXml/MakeTransformationref.h"
#include "OutputDirector.h"
#include <string>
#include <xercesc/dom/DOM.hpp>
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/GeoNodeList.h"
#include "GeoModelXml/StringWrappers.h"

using namespace std;
using namespace xercesc;

GeoIntrusivePtr<RCBase> MakeTransformationref::make(const DOMElement *element, GmxUtil &gmxUtil) const {
    XMLCh *ref = XMLString::transcode("ref");
    const XMLCh *idref;
    DOMDocument *doc = element->getOwnerDocument();
    char *toRelease;
    //
    //    Get the referenced element
    //
    idref = element->getAttribute(ref);
    DOMElement *elem = doc->getElementById(idref);
    //
    //    Check it is the right sort
    //
    toRelease = XMLString::transcode(elem->getNodeName());
    string nodeName(toRelease);
    XMLString::release(&toRelease);
    if (nodeName != string("transformation")) {
        THROW_EXCEPTION("Error in xml/gmx file: transformationref " << XMLString::transcode(idref) 
                    << " referenced a " << nodeName << " instead of a transformation.");
    }
    //
    //    Process it
    //
    XMLString::release(&ref);
    return gmxUtil.tagHandler.transformation.process(elem, gmxUtil);
}

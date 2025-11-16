/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelXml/MakeTransformation.h"
#include "GeoModelXml/StringWrappers.h"

#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GmxUtil.h"

#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelHelpers/TransformToStringConverter.h"

#include <iostream>
using namespace xercesc;
using namespace std;


GeoIntrusivePtr<RCBase> MakeTransformation::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
     GeoTrf::Transform3D hepTransform{GeoTrf::Transform3D::Identity()}; // Starts as Identity transform
    //
    //   Add my element contents
    //
    for (DOMNode *child = element->getFirstChild(); child != 0; child = child->getNextSibling()) {
        if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
            
            const std::string name = GeoXML::nodeName(*child); 
            DOMElement *el = dynamic_cast<DOMElement *>(child);
            if (name == "translation") {
                hepTransform = hepTransform * gmxUtil.tagHandler.translation.getTransform(el, gmxUtil); 
            } else if (name == "rotation") {
                hepTransform = hepTransform * gmxUtil.tagHandler.rotation.getTransform(el, gmxUtil); 
            } else if (name == "scaling") {	    
                // TODO: figure out what to do in this case 	    
                //                hepTransform = hepTransform * gmxUtil.tagHandler.scaling.getTransform(el, gmxUtil); 
            }
        }
    }
    //
    //    Create and return GeoModel transform
    //
    const std::string alignable = GeoXML::fetchAttribute(*element, "alignable");
    if (alignable.find("true") != std::string::npos) {
        auto trf = make_intrusive<GeoAlignableTransform>(hepTransform); 
   
        return trf;
    }    
    return makeTransform(hepTransform);    
}

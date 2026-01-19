/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelXml/shape/MakeShapeShift.h"

#include "OutputDirector.h"
#include <string>
#include <xercesc/dom/DOM.hpp>

#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoTransform.h"

#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GmxUtil.h"

#include "GeoModelHelpers/TransformSorter.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelHelpers/GeoShapeUtils.h"
#include "GeoModelXml/StringWrappers.h"
#include "GeoModelXml/GmxUtil.h"

using namespace xercesc;
using namespace GeoXML;

GeoIntrusivePtr<RCBase> MakeShapeShift::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
    
    unsigned int elementIndex = 0;
    GeoShape* shape{nullptr};
    GeoTrf::Transform3D hepXf{GeoTrf::Transform3D::Identity()};
    for (DOMNode *child = element->getFirstChild(); child != nullptr; child = child->getNextSibling()) {
       if (child->getNodeType() != DOMNode::ELEMENT_NODE) { 
            continue;
       }
      switch (elementIndex) {
        case 0: { // First element is first shaperef
            shape = dynamic_pointer_cast<GeoShape>(gmxUtil.tagHandler.shaperef.process(dynamic_cast<DOMElement*> (child), gmxUtil));
            break;
        } case 1: { // Second element is transformation or transformationref
            const GeoTransform *geoXf = (nodeName(*child) == "transformation")
              ? dynamic_pointer_cast<const GeoTransform>( gmxUtil.tagHandler.transformation.process(dynamic_cast<DOMElement *>(child), gmxUtil))
              : dynamic_pointer_cast<const GeoTransform>( gmxUtil.tagHandler.transformationref.process(dynamic_cast<DOMElement *>(child), gmxUtil));
            hepXf = geoXf->getTransform();
            break;
        } default: // More than 3 elements?
            THROW_EXCEPTION("Only transofrmation & geoshaperef are accepted");
      }
      ++elementIndex;        
    }
    if (!shape) {
        THROW_EXCEPTION("No shape has been given");
    }
    
    static const GeoTrf::TransformSorter sorter{};
    if (!sorter.compare(GeoTrf::Transform3D::Identity(), hepXf)) {
        return shape;
    }    
    return const_pointer_cast(cacheShape(make_intrusive<GeoShapeShift>(shape, hepXf)));
}


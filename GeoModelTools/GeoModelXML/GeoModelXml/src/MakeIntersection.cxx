/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Automatically generated code from /home/hessey/prog/gmx2geo/makeshape
// Then heavily modified
#include "GeoModelXml/shape/MakeIntersection.h"
#include <string>
#include "OutputDirector.h"
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoShapeIntersection.h"
#include "GeoModelKernel/GeoShapeShift.h"

#include "GeoModelHelpers/TransformSorter.h"
#include "GeoModelHelpers/GeoShapeUtils.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelXml/StringWrappers.h"
#include "GeoModelXml/GmxUtil.h"


using namespace GeoXML;
using namespace xercesc;
GeoIntrusivePtr<RCBase>MakeIntersection::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
    // 
    //    Process child elements; first is first shaperef; then transform; then second shaperef.
    //
    GeoIntrusivePtr<const GeoShape> first{}, second{};
    GeoTrf::Transform3D hepXf=GeoTrf::Transform3D::Identity(); 
    int elementIndex = 0;
    for (DOMNode *child = element->getFirstChild(); child != 0; child = child->getNextSibling()) {
        if (child->getNodeType() == DOMNode::ELEMENT_NODE) { // Skips text nodes
          switch (elementIndex) {
            case 0: { // First element is first shaperef
               first = dynamic_pointer_cast<GeoShape>(gmxUtil.tagHandler.shaperef.process(dynamic_cast<DOMElement *> (child), gmxUtil));
               break;
            } case 1:  { // Second element is transformation or transformationref
              GeoTrfPtr geoXf = (nodeName(*child) == "transformation") ?
                      dynamic_pointer_cast<GeoTransform>( gmxUtil.tagHandler.transformation.process(dynamic_cast<DOMElement *>(child), gmxUtil)): 
                      dynamic_pointer_cast<GeoTransform>( gmxUtil.tagHandler.transformationref.process(dynamic_cast<DOMElement *>(child), gmxUtil));
              hepXf = geoXf->getTransform();
              break;
            } case 2: { // Third element is second shaperef
              second = dynamic_pointer_cast<GeoShape>(gmxUtil.tagHandler.shaperef.process(dynamic_cast<DOMElement *> (child), gmxUtil));
              break;
            }
            default: // More than 3 elements?
                THROW_EXCEPTION("MakeIntersection: Incompatible DTD? got more than 3 child elements\n");
          }
          elementIndex++;
       }
    }

    if (!first || !second) {
        THROW_EXCEPTION("Not both shapes were defined");
    } 
    GeoIntrusivePtr<GeoShapeIntersection> isect{};
    static const GeoTrf::TransformSorter trfSorter{};
    if (true || trfSorter.compare(hepXf, GeoTrf::Transform3D::Identity())) {
        isect = make_intrusive<GeoShapeIntersection>(first, make_intrusive<GeoShapeShift>(second, hepXf));
    } else {
        isect = make_intrusive<GeoShapeIntersection>(first, second);
    }   
    return const_pointer_cast(cacheShape(isect));
}

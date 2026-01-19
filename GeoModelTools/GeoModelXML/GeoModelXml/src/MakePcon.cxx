/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Automatically generated code from /home/hessey/prog/gmx2geo/makeshape
// But then edited for AddPlane stuff
#include "GeoModelXml/shape/MakePcon.h"
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelHelpers/GeoShapeUtils.h"
#include "GeoModelKernel/throwExcept.h"

#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"
#include <array>
using namespace xercesc;
using namespace GeoXML;


GeoIntrusivePtr<RCBase> MakePcon::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
    constexpr std::size_t nParams = 2; 
    static const std::array<std::string, nParams> parName{"sphi", "dphi"};
    std::array<double, nParams> p{fetchAttributes(gmxUtil, *element, parName)};
  
    GeoIntrusivePtr<GeoPcon> pcon = make_intrusive<GeoPcon>(p[0], p[1]);
    //
    //    Add planes
    //
    double zPlane{0.}, rMinPlane{0.}, rMaxPlane{0.};
    for (DOMNode *child = element->getFirstChild(); child != nullptr; 
                  child = child->getNextSibling()) {
        if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
            if (nodeName(*child) == "addplane") {
                gmxUtil.tagHandler.addplane.process(dynamic_cast<const DOMElement *>(child), zPlane, rMinPlane, rMaxPlane);
                pcon->addPlane(zPlane, rMinPlane, rMaxPlane);
            }
        }
    }
    if (!pcon->isValid()) {
        THROW_EXCEPTION("Invalid Pcon defined "<<printGeoShape(pcon));
    }
    return const_pointer_cast(cacheShape(pcon));
}

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// Automatically generated code from /home/hessey/prog/gmx2geo/makeshape
// But then edited for AddPlane stuff
#include "GeoModelXml/shape/MakePgon.h"
#include "GeoModelKernel/GeoPgon.h"
#include <xercesc/dom/DOM.hpp>
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"

#include <array>
using namespace xercesc;
using namespace GeoXML;


GeoIntrusivePtr<RCBase>MakePgon::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
    constexpr std::size_t nParams = 3; 
    static const std::array<std::string, nParams> parName {"sphi", "dphi", "nsides"};
    const std::array<double, nParams> p{fetchAttributes(gmxUtil,*element, parName)};

    GeoIntrusivePtr<GeoPgon> pgon = make_intrusive<GeoPgon>(p[0], p[1], p[2]);
    //
    //    Add planes
    //
    double zPlane{0.}, rMinPlane{0.}, rMaxPlane{0.};
    for (DOMNode *child = element->getFirstChild(); child != 0; child = child->getNextSibling()) {
        if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
            if (nodeName(*child) == "addplane") {
                gmxUtil.tagHandler.addplane.process(dynamic_cast<const DOMElement *>(child), zPlane, rMinPlane, rMaxPlane);
                pgon->addPlane(zPlane, rMinPlane, rMaxPlane);
            }
        }
    }
    return const_pointer_cast(cacheShape(pgon));
}

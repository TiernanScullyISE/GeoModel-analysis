/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <array>

#include "GeoModelXml/shape/MakeSimplePolygonBrep.h"
#include "GeoModelKernel/GeoSimplePolygonBrep.h"
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/throwExcept.h"
#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"
#include "GeoModelHelpers/StringUtils.h"

using namespace xercesc;
using namespace GeoXML;
using namespace GeoStrUtils;


std::ostream& operator<<(std::ostream& ostr, const std::vector<double>& v) {
    ostr<<"[";
    for (const double x : v) {
        ostr<<x<<";";
    }
    ostr<<"]";
    return ostr;
}

GeoIntrusivePtr<RCBase> MakeSimplePolygonBrep::make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const {
    std::vector <double> x{tokenizeDouble(fetchAttribute(*element, "xpoints"), ";")};
    std::vector <double> y{tokenizeDouble(fetchAttribute(*element, "ypoints"), ";")};
    const double z = gmxUtil.evaluate(fetchAttribute(*element, "zhalflength"));
    
    GeoIntrusivePtr<GeoSimplePolygonBrep> poly = make_intrusive<GeoSimplePolygonBrep>(z);
    
    int nx = x.size();
    int ny = y.size();
  
    if (nx < 3 || ny < 3 || nx != ny) {
        THROW_EXCEPTION("Unequal number of x and y points, or less than 3\n\n"<<
         "xpoints was:\n"<<x<<"\nypoints was:\n"<<y<<"\n\n");
    }

    for (int i = 0; i < nx; ++i) {
        poly->addVertex(x[i], y[i]);
    }

    return const_pointer_cast(cacheShape(poly));

}

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//
//   replicaX element processor.
//

#include "GeoModelXml/ReplicaRPhiProcessor.h"

#include "OutputDirector.h"
#include <sstream>
#include <string>
#include <cstdlib>
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelHelpers/StringUtils.h"


#include "GeoModelXml/GeoNodeList.h"
#include "GeoModelXml/ProcessorRegistry.h"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"

using namespace GeoXML;

GeoNodeList ReplicaRPhiProcessor::fillReplicaTrf(const xercesc::DOMElement* element,
                                                GmxUtil& gmxUtil,
                                                const int nCopies,
                                                const bool alignable) {

    const double offsetPhi = gmxUtil.evaluate(fetchAttribute(*element, "offsetPhi"));
    const double stepPhi= gmxUtil.evaluate(fetchAttribute(*element, "stepPhi"));
    const double zVal = gmxUtil.evaluate(fetchAttribute(*element, "zValue"));
    const double radius = gmxUtil.evaluate(fetchAttribute(*element, "radius"));
    
    GeoNodeList xfList{};

    GeoIntrusivePtr<GeoAlignableTransform> geoAXf{};
    GeoIntrusivePtr<GeoTransform> geoXf{};

    //
    //    If varname not given, we get the CLHEP xForm and raise it to the power i, so NOT applied to first object.
    //    No transform (i.e. identity) for the first; so one less transform than objects
    //
    for (int i = 0; i < nCopies; ++i) {
        const double angle = offsetPhi + i * stepPhi;
        const GeoTrf::Transform3D hepXf=
                     GeoTrf::TranslateZ3D(zVal)*
                     GeoTrf::TranslateX3D(radius*std::cos(angle))*
                     GeoTrf::TranslateY3D(radius*std::sin(angle))*
                     GeoTrf::RotateZ3D(angle);
        if (alignable){
            xfList.push_back(make_intrusive<GeoAlignableTransform>(hepXf));
        } else {
            xfList.push_back(makeTransform(hepXf));
        }
    }
    return xfList;
}
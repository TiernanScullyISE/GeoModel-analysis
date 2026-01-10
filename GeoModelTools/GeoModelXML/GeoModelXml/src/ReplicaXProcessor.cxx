/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//
//   replicaX element processor.
//

#include "GeoModelXml/ReplicaXProcessor.h"

#include "OutputDirector.h"
#include <sstream>
#include <string>
#include <cstdlib>
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoVFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoDefinitions.h"

#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GeoNodeList.h"
#include "GeoModelXml/ProcessorRegistry.h"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"

using namespace GeoXML;

GeoNodeList ReplicaXProcessor::fillReplicaTrf(const xercesc::DOMElement* element,
                                              GmxUtil& gmxUtil,
                                              const int nCopies,
                                              const bool alignable) {
    const double offsetX= gmxUtil.evaluate(fetchAttribute(*element, "offset"));
    const double stepX= gmxUtil.evaluate(fetchAttribute(*element, "step"));
    
    GeoNodeList xfList{};

    for (int i = 0; i < nCopies; ++i) {
        const double step = offsetX + i * stepX;
        if (!alignable) {
            xfList.push_back(makeTransform(GeoTrf::TranslateX3D{step}));
        } else {
            xfList.push_back(make_intrusive<GeoAlignableTransform>(GeoTrf::TranslateX3D{step}));
        }
    }
    return xfList;
}
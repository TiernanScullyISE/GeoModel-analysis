/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//
//   replicaZ element processor.
//

#include "GeoModelXml/ReplicaZProcessor.h"

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
#include "GeoModelHelpers/StringUtils.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelXml/StringWrappers.h"


#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GeoNodeList.h"
#include "GeoModelXml/ProcessorRegistry.h"
#include "GeoModelXml/GmxUtil.h"



using namespace xercesc;
using namespace GeoXML;
GeoNodeList ReplicaZProcessor::fillReplicaTrf(const xercesc::DOMElement* element,
                                              GmxUtil& gmxUtil,
                                              const int nCopies,
                                              const bool alignable) {
    const double offsetX= gmxUtil.evaluate(fetchAttribute(*element, "offset"));
    const double stepX= gmxUtil.evaluate(fetchAttribute(*element, "step"));
    
    GeoNodeList xfList{};

    for (int i = 0; i < nCopies; ++i) {
        const double step = offsetX + i * stepX;
        if (!alignable) {
            xfList.push_back(makeTransform(GeoTrf::TranslateZ3D{step}));
        } else {
            xfList.push_back(make_intrusive<GeoAlignableTransform>(GeoTrf::TranslateZ3D{step}));
        }
    }
    return xfList;
}
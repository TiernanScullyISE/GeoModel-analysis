/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO_MODEL_XML_REPLICAY_PROCESSOR_H
#define GEO_MODEL_XML_REPLICAY_PROCESSOR_H
#include <xercesc/util/XercesDefs.hpp>
//
//   Processor for replicaY tags.
//
#include <string>
#include <map>

#include "GeoModelXml/IReplicaProcessor.h"
class GmxUtil;

class ReplicaYProcessor: public IReplicaProcessor {
public:
    ReplicaYProcessor() = default;

    GeoNodeList fillReplicaTrf(const xercesc::DOMElement* element,
                               GmxUtil& gmxUtil,
                               const int nCopies,
                               const bool alignable) override final;
};

#endif // REPLICAY_PROCESSOR_H

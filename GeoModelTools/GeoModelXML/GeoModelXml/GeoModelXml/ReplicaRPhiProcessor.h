/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO_MODEL_XML_REPLICARPHI_PROCESSOR_H
#define GEO_MODEL_XML_REPLICARPHI_PROCESSOR_H
#include <xercesc/util/XercesDefs.hpp>
//
//   Processor for replicaRPhi tags.
//
#include <string>
#include <map>

#include "GeoModelXml/IReplicaProcessor.h"
class GmxUtil;

class ReplicaRPhiProcessor: public IReplicaProcessor {
public:
    ReplicaRPhiProcessor() = default;
    GeoNodeList fillReplicaTrf(const xercesc::DOMElement* element,
                                       GmxUtil& gmxUtil,
                                       const int nCopies,
                                       const bool alignable) override final;
};

#endif // REPLICAX_PROCESSOR_H

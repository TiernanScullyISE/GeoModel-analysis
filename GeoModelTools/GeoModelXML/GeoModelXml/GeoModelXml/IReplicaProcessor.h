/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO_MODEL_XML_IReplicaPROCESSOR_H
#define GEO_MODEL_XML_IReplicaPROCESSOR_H
#include <xercesc/util/XercesDefs.hpp>
//
//   Processor for replicaX tags.
//
#include <string>
#include <map>

#include "GeoModelXml/ElementProcessor.h"
class GmxUtil;

class IReplicaProcessor: public ElementProcessor {
public:
    IReplicaProcessor() = default;
    void process(const xercesc::DOMElement *element, GmxUtil &gmxUtil, GeoNodeList &toAdd) override final;
    virtual GeoNodeList fillReplicaTrf(const xercesc::DOMElement* element,
                                       GmxUtil& gmxUtil,
                                       const int nCopies,
                                       const bool alignable) = 0;
private:
    std::map<std::string, GeoNodeList> m_map{}; 
};

#endif // REPLICAX_PROCESSOR_H

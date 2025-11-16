/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//
//    Base class to process one element (not its children).
//    For each element-tag, create a class inheriting from this; and supply the process method.
//    Register your processor with the BranchProcessor.
//
#ifndef GEO_MODEL_XML_ELEMENT2GEO_ITEM_H
#define GEO_MODEL_XML_ELEMENT2GEO_ITEM_H
#include <xercesc/util/XercesDefs.hpp>

#include "GeoModelHelpers/GeoDeDuplicator.h"
#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"

#include <unordered_map>
#include <string>

class RCBase;
class GmxUtil;

XERCES_CPP_NAMESPACE_BEGIN
class DOMElement;
XERCES_CPP_NAMESPACE_END
class ProcessorRegistry;
namespace HepTool {class Evaluator;}

class Element2GeoItem: public GeoDeDuplicator {

public:
    Element2GeoItem() = default;
    virtual ~Element2GeoItem() = default;
    GeoIntrusivePtr<RCBase> process(const xercesc::DOMElement *element, 
                                    GmxUtil &gmxUtil);
                                    
    virtual GeoIntrusivePtr<RCBase> make(const xercesc::DOMElement *element, GmxUtil &gmxUtil) const;
protected:
    using EntryMap = std::unordered_map<std::string, GeoIntrusivePtr<RCBase>>;
    EntryMap m_map{};  
};
#endif // ELEMENT2GEO_ITEM_H


/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelXml/StringWrappers.h"

#include "xercesc/util/XMLString.hpp"
using namespace xercesc;
namespace GeoXML{
    bool hasAttribute(const xercesc::DOMElement& element,
                      const std::string& attrName){
        XMLCh * tmpAttr = XMLString::transcode(attrName.c_str());
        const bool hasIt = element.hasAttribute(tmpAttr);
        XMLString::release(&tmpAttr);
        return hasIt;
    }
    std::string nodeName(const xercesc::DOMNode& element) {
        std::string toRet{};
        char* nodeName = XMLString::transcode(element.getNodeName());
        std::string objectName{nodeName};
        XMLString::release(&nodeName);       
        return objectName;
    }
    std::string tagName(const xercesc::DOMElement& element) {  
        char* tagName = XMLString::transcode(element.getTagName());
        std::string objectName{tagName};
        XMLString::release(&tagName);
        return objectName;
  
    }
    std::string fetchAttribute(const xercesc::DOMElement& element,
                               const std::string& attrName) {
        XMLCh * name_tmp = XMLString::transcode(attrName.c_str());
        char *name2release = XMLString::transcode(element.getAttribute(name_tmp));
        std::string toRet{name2release};
    
        XMLString::release(&name2release);
        XMLString::release(&name_tmp);
        return toRet;
    }


}
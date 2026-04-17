
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelXml/StringWrappers.h"
#include "GeoModelHelpers/StringUtils.h"
#include <xercesc/util/XMLString.hpp>

#include <cassert>
using namespace xercesc;
namespace GeoXML{
    std::string xml2Str(const XMLCh* s) {
        if (!s) return {};
        char* p = XMLString::transcode(s);
        std::string out = p ? p : "";
        XMLString::release(&p);
        return out;
    }
    
    bool hasAttribute(const xercesc::DOMElement& element,
                      const std::string& attrName){
        XMLCh * tmpAttr = XMLString::transcode(attrName.c_str());
        const bool hasIt = element.hasAttribute(tmpAttr);
        XMLString::release(&tmpAttr);
        return hasIt;
    }
    
    std::string nodeName(const xercesc::DOMNode& element) {
        return xml2Str(element.getNodeName());
    }
    
    std::string tagName(const xercesc::DOMElement& element) {  
        return xml2Str(element.getTagName());
    }
    
    std::string fetchAttribute(const xercesc::DOMElement& element,
                               const std::string& attrName) {
        XMLCh * name_tmp = XMLString::transcode(attrName.c_str());
        std::string toRet = xml2Str(element.getAttribute(name_tmp));
        XMLString::release(&name_tmp);
        return toRet;
    }
    std::vector<int> rangeList(const xercesc::DOMElement& element,
                               const std::string& attrName) {
        std::vector<int> result{};
        std::vector<std::string> parsed = GeoStrUtils::tokenize(fetchAttribute(element, attrName)," ");
        for (const std::string& k : parsed) {
            std::vector<std::string> tmp_parsed = GeoStrUtils::tokenize(k,"-");
            if (tmp_parsed.size()==1) result.push_back(GeoStrUtils::atoi(tmp_parsed[0]));
            else if (tmp_parsed.size()==2) {
                int i1=GeoStrUtils::atoi(tmp_parsed[0]);
                int i2=GeoStrUtils::atoi(tmp_parsed[1]);
                assert(i1<i2);
                for (int l=i1;l<=i2;++l) {
                    result.push_back(l);
                }
            }
        }
        return result;
    }
}
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GEOMODELXML_STRINGWRAPPERS_H
#define GEOMODELXML_STRINGWRAPPERS_H

#include <string>
#include <xercesc/dom/DOM.hpp>

namespace GeoXML{

    /** @brief Translates the Node name into a std::string */
    std::string nodeName(const xercesc::DOMNode& element);
    /** @brief Fetches the tag name of a Node and returns it to string */
    std::string tagName(const xercesc::DOMElement& element);
    
    /** @brief Returns whether the element has an attribute */
    bool hasAttribute(const xercesc::DOMElement& element,
                      const std::string& attrName);
    /** @brief Fetches the value of a attribute from an element */
    std::string fetchAttribute(const xercesc::DOMElement& element,
                               const std::string& attrName);

}

#endif  

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GEOMODELXML_STRINGWRAPPERS_H
#define GEOMODELXML_STRINGWRAPPERS_H

#include "GeoModelXml/GmxUtil.h"
#include <string>
#include <array>
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
    /** @brief Extract the parameters from the XML element using a parName list
     *         & fill them into an array where the i-th value corresponds to the i-th
     *         parName
     * @param util: Reference to the GmxUtil evaluating simple mathematical expressions
     * @param element: Xml object from which the parameters shall be extracted
     * @param parNames: Array of parameter names to be extracted */
    template <std::size_t nPars>
    std::array<double, nPars> fetchAttributes(GmxUtil& util,
                                              const xercesc::DOMElement& element,
                                              const std::array<std::string, nPars>& parNames) {
        std::array<double, nPars> result{};
        for (std::size_t p = 0 ; p < nPars; ++p) {
          result[p] = util.evaluate(fetchAttribute(element, parNames[p]));
        }
        return result;
    }
    /** @brief Extracts a list of integers from a XML document. The integers may
     *         either encoded element wise, e.g. 1 2 3 4 5 or in an interval 1-5
     *         or combined, e.g. "2 3 5 7-9" 
     * @param element: XML object from which the attribute is fetched and the 
     *                 list is then compiled
     * @param attrName: Name of the xml attribute from which the primary string is
     *                  fetched*/
    std::vector<int> rangeList(const xercesc::DOMElement& element,
                               const std::string& attrName);

}

#endif  

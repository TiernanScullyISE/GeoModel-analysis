/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <string>


#include "GeoModelXml/Element2GeoItem.h"
#include "GeoModelXml/StringWrappers.h"
#include "GeoModelXml/GmxUtil.h"

#include "OutputDirector.h"

#include "xercesc/util/XercesDefs.hpp"
#include <xercesc/dom/DOM.hpp>

#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelKernel/GeoAlignableTransform.h"

GeoIntrusivePtr<RCBase> Element2GeoItem::process(const xercesc::DOMElement *element, 
                                                 GmxUtil &gmxUtil) {

    const std::string name =  GeoXML::fetchAttribute(*element, "name");

    GeoIntrusivePtr<RCBase> item{nullptr};
    // Unnamed item or an item that can be duplicated; cannot store in the map; make a new one
    if (name.empty()) {
        return make(element, gmxUtil);
    }
    EntryMap::iterator entry = m_map.find(name);
    /// Element not yet created
    if (entry == m_map.end()) { 
        auto item = make(element,gmxUtil);
        if (!item) {
            THROW_EXCEPTION("Item not made "<<name<<", "<< GeoXML::nodeName(*element));
        }
        if (dynamic_pointer_cast<GeoAlignableTransform>(item) != nullptr) {
            return item;
        }
        const auto insert_itr = m_map.insert(std::make_pair(name, item));
        if (!insert_itr.second) {
            THROW_EXCEPTION("Failed to create element "<<name<<", "<< GeoXML::nodeName(*element));
        }
        return insert_itr.first->second;
    }
    /// Return the existing one
    return entry->second;
}

GeoIntrusivePtr<RCBase> Element2GeoItem::make(const xercesc::DOMElement *element, GmxUtil & /* gmxUtil */) const {
    THROW_EXCEPTION("Oh oh: called base class make() method of Element2GeoType object; tag " << GeoXML::nodeName(*element));
}

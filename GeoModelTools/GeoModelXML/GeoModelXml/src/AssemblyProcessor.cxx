/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//
//   Processor for assembly elements
//
//   Add name to list.
//   Create a physvol using special logvol GmxUtil.getAssemblyLV() add it to list.
//      ...by using material "special::ether" Geo2G4 turns this into a G4Assembly
//   Process children and get list of things to be added to the physvol.
//   Add them to the physvol.
//
#include "GeoModelXml/AssemblyProcessor.h"
#include "GeoModelXml/StringWrappers.h"

#include <map>
#include <iostream>
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/GeoNodeList.h"
#include "xercesc/util/XMLString.hpp"

// using namespace CLHEP;

using namespace std;
using namespace xercesc;

void AssemblyProcessor::process(const DOMElement *element, GmxUtil &gmxUtil, GeoNodeList &toAdd) {
    GeoIntrusivePtr<GeoLogVol> lv{};
    GeoIntrusivePtr<GeoNameTag> physVolName{};

    gmxUtil.positionIndex.incrementLevel();

    const std::string name = GeoXML::fetchAttribute(*element, "name");
    gmxUtil.positionIndex.addToLevelMap(name,gmxUtil.positionIndex.level());
//
//    Look for the assembly in the map; if not yet there, add it
//
    std::map<std::string, AssemblyStore>::iterator entry = m_map.find(name); 
    if (entry == m_map.end()) { // Not in registry; make a new item
        //
        //    Name
        AssemblyStore& store {m_map[name]};
        physVolName = nameTag(name); 
        store.name = physVolName;
    } else { // Already in the registry; use it.
        physVolName = entry->second.name;
    }
    lv = gmxUtil.getAssemblyLV();
    toAdd.push_back(physVolName);
    gmxUtil.positionIndex.setCopyNo(m_map[name].id);
    toAdd.push_back(geoId(m_map[name].id++)); 
//
//    Process the assembly's children
//
    GeoNodeList childrenAdd;
    for (DOMNode *child = element->getFirstChild(); child != 0; child = child->getNextSibling()) {
        if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement *el = dynamic_cast<DOMElement *> (child);
            const std::string name = GeoXML::nodeName(*el);
            gmxUtil.processorRegistry.find(name)->process(el, gmxUtil, childrenAdd);
        }
    }
//
//    Make a new PhysVol and add everything to it, then add it to the list of things for my caller to add
//
    const std::string alignable = GeoXML::fetchAttribute(*element, "alignable");
    if (alignable == "true") {

        auto pv = make_intrusive<GeoFullPhysVol>(cacheVolume(lv));
        for (const auto& node : childrenAdd) {
            pv->add(node);
        }
        toAdd.push_back(pv); // NB: the *PV is third item added, so reference as toAdd[2].
    }
    else {
        auto pv = make_intrusive<GeoPhysVol>(cacheVolume(lv));
        for (const auto& node : childrenAdd) {
            pv->add(node);
        }
        toAdd.push_back(cacheVolume(pv));
    }

    gmxUtil.positionIndex.decrementLevel();

    return;
}

void AssemblyProcessor::zeroId(const xercesc::DOMElement *element) {
   const std::string name = GeoXML::fetchAttribute(*element, "name");
    
//    Look for the assembly in the map; if not yet there, add it
//
    
    std::map<std::string, AssemblyStore>::iterator entry = m_map.find(name);
    if (entry != m_map.end()) {
        entry->second.id = 0;
    }
/* else ... Not an error: it is usually just about to be made with id = 0; no action needed. */
}

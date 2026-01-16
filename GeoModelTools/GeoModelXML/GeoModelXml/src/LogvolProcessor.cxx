/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//
//   Processor for logvol elements
//
//   Add name to list.
//   Create a physvol using my logvol; add it to list.
//   Process children and get list of things to be added to the physvol.
//   Add them to the physvol.
//
//
// Updates:
// - 2022/02, Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
//            * Removed the automatic cration of a GeoNameTag for each volume, to save memory
//            * Added support for the GeoNameTag node
//


#include "GeoModelXml/LogvolProcessor.h"
#include "GeoModelXml/StringWrappers.h"
#include "OutputDirector.h"

#include <map>
#include <format>

#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoVFullPhysVol.h"
#include "GeoModelKernel/GeoVolumeTagCatalog.h"
#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/GeoNodeList.h"
#include "xercesc/util/XMLString.hpp"

#include "GeoModelKernel/throwExcept.h"
// using namespace CLHEP;

using namespace std;
using namespace xercesc;

std::string getNodeType(const GeoGraphNode* node) {
    if ( dynamic_cast<const GeoPhysVol*>(node) )
        return "GeoPhysVol";
    if ( dynamic_cast<const GeoFullPhysVol*>(node) )
        return "GeoFullPhysVol";
    if ( dynamic_cast<const GeoIdentifierTag*>(node) )
        return "GeoIdentifierTag";
    if ( dynamic_cast<const GeoNameTag*>(node) )
        return "GeoNameTag";
    if ( dynamic_cast<const GeoLogVol*>(node) )
        return "GeoLogVol";
    if ( dynamic_cast<const GeoShape*>(node) )
        return "GeoShape";
    if ( dynamic_cast<const GeoMaterial*>(node) )
        return "GeoMaterial";
    if ( dynamic_cast<const GeoTransform*>(node) )
        return "GeoTransform";
    return "UnidentifiedNode";
}
void LogvolProcessor::process(const DOMElement *element, GmxUtil &gmxUtil, GeoNodeList &toAdd) {
  GeoLogVolPtr lv{};
  GeoIntrusivePtr<GeoNameTag> nameTag_physChildVolName{};//USed for "sensitive" PhysVols 
  GeoIntrusivePtr<GeoNameTag> nameTag_physVolName{};//Actually the logVol name, which gets used for the PhysVols if they have "named" attribute (and aren't sensitive)

  gmxUtil.positionIndex.incrementLevel();

  // get the name of the LogVol
  const std::string name = GeoXML::fetchAttribute(*element, "name");
  gmxUtil.positionIndex.addToLevelMap(name,gmxUtil.positionIndex.level());
  
  // get the value for the "named" option;
  // if "true", add a GeoNameTag to the GeoModel tree
  const std::string named = GeoXML::fetchAttribute(*element, "named");
  bool isNamed = (named == "true");
  
  // get the value for the "identifier" option;
  // if "true", add a GeoIdentifierTag to the GeoModel tree
  const std::string idStr = GeoXML::fetchAttribute(*element, "identifier");
  bool hasIdentifier = idStr == "true";
  //
  std::string envelope = GeoXML::fetchAttribute(*element, "envelope");
  bool is_envelope=(envelope == "true");
  
  bool sensitive = GeoXML::hasAttribute(*element, "sensitive");
 
//
//    Look for the logvol in the map; if not yet there, add it
//
  std::map<std::string, LogVolStore>::iterator entry = m_map.find(name);
  if (entry == m_map.end()) { // Not in registry; make a new item
    //
    //    Name
    //   
    LogVolStore* store{&m_map[name]};
    if(isNamed) {
        nameTag_physVolName = nameTag(name);
        store->name = nameTag_physVolName;
    }   
    //
    //    Get the shape.
    //
    DOMDocument *doc = element->getOwnerDocument();
    XMLCh * shape_tmp = XMLString::transcode("shape");
    const XMLCh *shape = element->getAttribute(shape_tmp);
    DOMElement *refShape = doc->getElementById(shape);
    // Check it is a shape... its parent should be a <shapes>. DTD cannot do this for us.
    DOMNode *parent = refShape->getParentNode();
    if (XMLString::compareIString(parent->getNodeName(), XMLString::transcode("shapes")) != 0) {
        char* shape_s = XMLString::transcode (shape);
        THROW_EXCEPTION("Processing logvol " << name <<
            ". Error in gmx file. An IDREF for a logvol shape did not refer to a shape.\n" <<
            "Shape ref was " << shape_s << "; exiting");
        XMLString::release (&shape_s);
    }
    //
    //    What sort of shape?
    //
    std::string shapeType = GeoXML::nodeName(*refShape);
    auto shGeo = dynamic_pointer_cast<const GeoShape>(gmxUtil.geoItemRegistry.find(shapeType)->process(refShape, gmxUtil));
    //
    //    Get the material
    //
    XMLCh * material_tmp = XMLString::transcode("material");
    const XMLCh *material = element->getAttribute(material_tmp);
    DOMElement *refMaterial = doc->getElementById(material);
    // Check it is a material... its parent should be a <materials>. DTD cannot do this for us.
    parent = refMaterial->getParentNode();
    XMLCh * materials_tmp = XMLString::transcode("materials");
    if (XMLString::compareIString(parent->getNodeName(), materials_tmp) != 0) {
        char* material_s = XMLString::transcode (material);
        std::string materialName{material_s};
        XMLString::release (&material_s);
        THROW_EXCEPTION("Processing logvol " << name <<
            ". Error in gmx file. An IDREF for a logvol material did not refer to a material.\n" <<
            "Material ref was " << materialName << "; exiting");
    }
    std::string nam_mat= XMLString::transcode(material);

    GeoIntrusivePtr<const GeoMaterial> matGeo{};

    if (gmxUtil.matManager) {
        if (!gmxUtil.matManager->isMaterialDefined(nam_mat)) {
            auto tempMat=dynamic_pointer_cast<GeoMaterial>(gmxUtil.tagHandler.material.process(refMaterial, gmxUtil));
            // we let GMX create the material and store it in the MM
            gmxUtil.matManager->addMaterial(tempMat);
        } matGeo = gmxUtil.matManager->getMaterial(nam_mat);
    } else {
        matGeo = dynamic_pointer_cast<const GeoMaterial>(gmxUtil.tagHandler.material.process(refMaterial, gmxUtil));
    }

    if (!matGeo) {
        THROW_EXCEPTION("Cannot process "<<nam_mat);
    }    
    //
    //    Make the LogVol and add it to the map ready for next time
    //
    lv = make_intrusive<GeoLogVol>(name, shGeo, matGeo);
    store->logVol = lv;

    XMLString::release(&material_tmp);
    XMLString::release(&materials_tmp);
  }
  else { // Already in the registry; use it.
    //msglog << MSG::DEBUG << "LogVol w/ name '" << name << "' already present, picking it from cache..." << endmsg;
    if(isNamed) {
        nameTag_physVolName = entry->second.name;
    }
    lv = entry->second.logVol;
  }

  //
  //    Process the logvol children (side effect: sets formulae for indexes before calculating them)
  //
  // RMB: Note -- here the code looks for "children of the LogVol" but this is not true: they are PhysVol children. In fact, they are later added to the newly created PhysVol... needs to be clearified/updated
  GeoNodeList childrenAdd;
  for (DOMNode *child = element->getFirstChild(); child != nullptr; child = child->getNextSibling()) {
      if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
        DOMElement *el = dynamic_cast<DOMElement *> (child);
        std::string name = GeoXML::nodeName(*el);
        gmxUtil.processorRegistry.find(name)->process(el, gmxUtil, childrenAdd);
      }
  }
  //
  //   Make a list of things to be added
  //
  if(isNamed && !sensitive) {
    toAdd.push_back(nameTag_physVolName);//If sensitive, it gets a different name in a moment...
  }

  int sensId = 0;
  std::map<std::string, int> index;
  if (sensitive) {
    gmxUtil.positionIndex.setCopyNo(m_map[name].id++);
    gmxUtil.positionIndex.indices(index, gmxUtil.eval);
    sensId = gmxUtil.gmxInterface().sensorId(index);
    std::string newName = name;
    for(auto& [nodeName, nodeIdx]: index) {
      newName += std::format("_{:}_{:}", nodeName, nodeIdx);
    }
    nameTag_physChildVolName = nameTag(newName);//Make sensitive always have a name, to extra Id information from
    toAdd.push_back(nameTag_physChildVolName);
    if(hasIdentifier) { //TODO: check if all "sensitive" volumes must have an identifier. If that's the case, then we can remove this "if" here
        toAdd.push_back(geoId(sensId));
    }
  } else {
      if(hasIdentifier) {
          toAdd.push_back(geoId(m_map[name].id)); // Normal copy number
          gmxUtil.positionIndex.setCopyNo(m_map[name].id++);
      }
  }

  //
  //    Make a new PhysVol/FullPhysVol and add everything to it, then add it to the list of things for my caller to add
  //
  
  // get the value for the "alignable" option
  std::string alignable = GeoXML::fetchAttribute(*element, "alignable");
  

  if (sensitive || alignable == "true") {
    //msglog << MSG::DEBUG << "Handling a FullPhysVol (i.e., an 'alignable' or 'sensitive' volume) ..." << endmsg;
    auto pv = make_intrusive<GeoFullPhysVol>(cacheVolume(lv));
    if (is_envelope) GeoVolumeTagCatalog::VolumeTagCatalog()->addTaggedVolume("Envelope",name,pv);
    for (const auto& node : childrenAdd) {
	     pv->add(node);
    }
    toAdd.push_back(pv); // NB: the *PV is third item added, so reference as toAdd[2].
    //
    //    Add sensitive volumes to detector manager via GmxInterface
    //
    if (sensitive) {
      std::string sensitiveName = GeoXML::fetchAttribute(*element, "sensitive");
      //splitting sensors where we would like multiple DetectorElements per GeoVFullPhysVol (e.g.ITk Strips)
      int splitLevel = 1;
	    if (GeoXML::hasAttribute(*element, "splitLevel")) {
        splitLevel = gmxUtil.evaluate(GeoXML::fetchAttribute(*element, "splitLevel").c_str());
        for(int i=0;i<splitLevel;i++){
          std::string field = "eta_module";//eventually specify in Xml the field to split in?
          std::pair<std::string,int> extraIndex(field,i);
          gmxUtil.gmxInterface().addSplitSensor(sensitiveName, index,extraIndex, sensId, pv, splitLevel);
        }
	    }
	    else gmxUtil.gmxInterface().addSensor(sensitiveName, index, sensId, pv);
    }
  }
  else {
      //msglog << MSG::DEBUG << "Handling a standard PhysVol..." << endmsg;
      auto pv = make_intrusive<GeoPhysVol>(cacheVolume(lv));
      if (is_envelope) GeoVolumeTagCatalog::VolumeTagCatalog()->addTaggedVolume("Envelope",name,pv);
      //msglog << MSG::DEBUG << "Now, looping over all the children of the LogVol (in the GMX meaning)..." << endmsg; 
      for (const auto & node : childrenAdd) {
        pv->add(node);
        //msglog << MSG::DEBUG << "LVProc, PV child: " << *node << " -- " << getNodeType(*node) << endmsg;
      }
      //msglog << MSG::DEBUG << "End of loop over children." << endmsg;
      toAdd.push_back(cacheVolume(pv));
  }
  gmxUtil.positionIndex.decrementLevel();
  if(gmxUtil.positionIndex.level()==-1){ //should mean that we are at the end of processing the geometry
    gmxUtil.positionIndex.printLevelMap();
  }
  return;
}

void LogvolProcessor::zeroId(const xercesc::DOMElement *element) {

  
  const std::string name = GeoXML::fetchAttribute(*element, "name");
  //
  //    Look for the logvol in the map; if not yet there, add it
  //
  map<string, LogVolStore>::iterator entry;
  if ((entry = m_map.find(name)) != m_map.end()) {
    entry->second.id = 0;
  }
  /* else: Not an error: it is usually just about to be made with id = 0; no action needed. */
}



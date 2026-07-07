/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//
//   replicaZ element processor.
//

#include "GeoModelXml/IReplicaProcessor.h"

#include "OutputDirector.h"
#include <sstream>
#include <string>
#include <cstdlib>
#include <cassert>
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelHelpers/StringUtils.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelXml/StringWrappers.h"

#include "GeoModelXml/GeoNodeList.h"
#include "GeoModelXml/ProcessorRegistry.h"
#include "GeoModelXml/GmxUtil.h"



using namespace xercesc;
using namespace GeoXML;

void IReplicaProcessor::process(const DOMElement *element, 
                                GmxUtil &gmxUtil, 
                                GeoNodeList &toAdd) {
    XMLCh *ref = XMLString::transcode("ref");
    const XMLCh *idref;
    DOMDocument *doc = element->getOwnerDocument();

    const bool alignable = hasAttribute(*element, "alignable");
    //
    //    How many copies?
    //
    const int nCopies = gmxUtil.evaluate(fetchAttribute(*element, "n"));
    std::vector<int> elementsToSkip{};
    if (hasAttribute(*element, "skip")) {
        elementsToSkip = rangeList(*element, "skip");
    }
    //
    //    See if it is in the map; if so, xfList is already done. If not, fill xfList.
    //
    std::string name = fetchAttribute(*element, "name");
    std::map<std::string, GeoNodeList>::iterator entry = m_map.find(name);
    GeoNodeList xfList{};
    if (entry == m_map.end()) { // Not in registry; make a new item
        xfList = fillReplicaTrf(element, gmxUtil, nCopies, alignable);
        assert(xfList.size() == static_cast<std::size_t>(nCopies));
        if (!alignable) {
            m_map[name] = xfList;
        }
    } else {
        xfList = entry->second;
    }
    //
    //    Get object to be copied
    //
    DOMElement *object = element->getLastElementChild();
    std::string nodeName = GeoXML::nodeName(*object);
    ElementProcessor *objectProcessor = gmxUtil.processorRegistry.find(nodeName);
    //
    //    Zero its copy number. Only needed if an item is used in 2 or more multicopies;
    //    harmless in case of only one use.
    //
    if (nodeName == "logvolref") {
        idref = object->getAttribute(ref);
        DOMElement *elem = doc->getElementById(idref);
        gmxUtil.tagHandler.logvol.zeroId(elem);
    } else if (nodeName == "assemblyref") {
        idref = object->getAttribute(ref);
        DOMElement *elem = doc->getElementById(idref);
        gmxUtil.tagHandler.assembly.zeroId(elem);
    } else if (nodeName == "transform") { // Object is either an assemlyref or logvolref, and there is only one of these
        XMLCh * logvolref_tmp = XMLString::transcode("logvolref");
        DOMNodeList *lvList = element->getElementsByTagName(logvolref_tmp);
        if (lvList->getLength() > 0) {
            const XMLCh *idref =  dynamic_cast<DOMElement *> (lvList->item(0))->getAttribute(ref);
            DOMElement *lv = doc->getElementById(idref);
            gmxUtil.tagHandler.logvol.zeroId(lv);
            XMLString::release(&logvolref_tmp);
        } else {
            XMLCh * assemblyref_tmp = XMLString::transcode("assemblyref") ;
            DOMNodeList *asList = element->getElementsByTagName(assemblyref_tmp);
            if (asList->getLength() > 0) {
                const XMLCh *idref =  dynamic_cast<DOMElement *> (asList->item(0))->getAttribute(ref);
                DOMDocument *doc = element->getOwnerDocument();
                DOMElement *as = doc->getElementById(idref);
                gmxUtil.tagHandler.assembly.zeroId(as);
                XMLString::release(&assemblyref_tmp);
            } else {
                THROW_EXCEPTION("ReplicaYProcessor: error in " << name 
                            << ". <transform> object was neither assemblyref nor logvolref\nExiting ");
            }
        }
    }
    //
    //    If alignable, add transformation to DetectorManager via GmxInterface.
    //    (NB. Alignable is messy because it involves both a transformation and an object as well as
    //    the multicopy or transform element to tell us when to insert it and what level of alignment 
    //    is wanted; and passing info from one routine to another when you try to keep a uniform interface is 
    //    difficult; and we only have partial GeoModel trees so we cannot use GeoModel tree traversal at this 
    //    moment (comes back to no way of knowing if we have a GeoFullPV or GeoPV)).
    //
    //    Also: no one is using it at the time of writing, so it is ***TOTALLY*** untested.
    //    It is done now to (i) make sure there ought to be a solution (ii) implement (imperfectly) 
    //    a way now while things are fresh in my mind.
    //
    int level{0};
    if (alignable) {
        level = GeoStrUtils::atoi(fetchAttribute(*element,"alignable"));
    }
    //
    //    Add transforms and physvols etc. to list to be added
    //
    std::map<std::string, int> index;
    for (int copy = 0; copy < nCopies; ++copy) {
        if (elementsToSkip.size()  && std::ranges::find(elementsToSkip,copy)!=elementsToSkip.end()) {
            continue;
        }
        toAdd.push_back(xfList[copy]);
        int lastTransform = toAdd.size() - 1;
        objectProcessor->process(object, gmxUtil, toAdd);
        if (alignable) {
            if(gmxUtil.gmxInterface().msgLvl(LogLevel::DEBUG)){
                msglog << MSG::DEBUG << "copy = " << copy << "; level = " << level << endmsg;
                msglog << MSG::DEBUG << "\nAdd Alignable named " << endmsg;
                msglog << MSG::DEBUG << (dynamic_pointer_cast<GeoNameTag>(toAdd[lastTransform + 1]))->getName() << endmsg;
                msglog << MSG::DEBUG << " with id " << endmsg;
                msglog << MSG::DEBUG << (dynamic_pointer_cast<GeoIdentifierTag>(toAdd[lastTransform + 2]))->getIdentifier() << endmsg;
            }
            gmxUtil.positionIndex.incrementLevel(); // Logvol has unfortunately already decremented this; temp. restore it
            gmxUtil.positionIndex.indices(index, gmxUtil.eval);
            //splitting sensors where we would like multiple DetectorElements per GeoVFullPhysVol (e.g.ITk Strips)
            int splitLevel = 1;
            if (hasAttribute(*element, "splitLevel")) {
                splitLevel = gmxUtil.evaluate(fetchAttribute(*element, "splitLevel"));
                for(int i=0;i<splitLevel;++i) {
                    std::string field = "eta_module";//eventually specify in Xml the field to split in?
                    std::pair<std::string,int> extraIndex(field,i);
                    gmxUtil.gmxInterface().addSplitAlignable(level, index, extraIndex,
                                                    dynamic_pointer_cast<GeoVFullPhysVol>(toAdd[lastTransform + 3]),
                                                    dynamic_pointer_cast<GeoAlignableTransform>(toAdd[lastTransform]));
                }
            }
            else {
                gmxUtil.gmxInterface().addAlignable(level, index, 
                                                     dynamic_pointer_cast<GeoVFullPhysVol>(toAdd[lastTransform + 3]),
                                                     dynamic_pointer_cast<GeoAlignableTransform>(toAdd[lastTransform]));
            }
            gmxUtil.positionIndex.decrementLevel(); // Put it back where it was
            index.clear();
        }
    }
    XMLString::release(&ref);
}

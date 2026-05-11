/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//
//   multicopy element processor.
//
//   First time round, a loop creates all the Geo-transforms needed and stores them. First and subsequent calls, 
//   a second loop adds these to the toAdd list, along with copies of the object. The object-handler stores and 
//   deals with the object. This si needed to make sure sensitive volumes and alignable volumes are always added 
//   correctly. 
//
//   The copy number of the copied object can be zeroed each time the multicopy-element is processed if the 
//   zeroid attribute is set true. This has to be done here - if the attribute is given on a logvol or assembly ref, 
//   it would alwasy be zero.
//
//   There are two ways of making the n transformations:
//        loopvar attribute not set:    The transformation is raised to the power i-1
//        loopvar set to a vector name: In the loop, before each transformation is created, the vector generic name is set
//                                      equal to the next element of the vector.
//
#include "GeoModelXml/MulticopyProcessor.h"
#include "GeoModelXml/StringWrappers.h"

#include "OutputDirector.h"
#include <sstream>
#include <string>
#include <cstdlib>
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoVFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoDefinitions.h"

#include "xercesc/util/XMLString.hpp"
#include "GeoModelXml/GeoNodeList.h"
#include "GeoModelXml/ProcessorRegistry.h"
#include "GeoModelXml/GmxUtil.h"

#include "GeoModelKernel/throwExcept.h"
#include "GeoModelHelpers/StringUtils.h"

using namespace xercesc;
using namespace std;


void MulticopyProcessor::process(const DOMElement *element, GmxUtil &gmxUtil, GeoNodeList &toAdd) {
   
    const std::string name = GeoXML::fetchAttribute(*element, "name");
    //char *toRelease;
    XMLCh *ref = XMLString::transcode("ref");
    const XMLCh *idref{};
    DOMDocument *doc = element->getOwnerDocument();

    bool alignable = GeoXML::hasAttribute(*element, "alignable");
    //
    //    How many copies?
    //
    const int nCopies = gmxUtil.evaluate(GeoXML::fetchAttribute(*element, "n"));
    //
    //    See if it is in the map; if so, xfList is already done. If not, fill xfList.
    //

    std::map<std::string, GeoNodeList>::iterator entry;
    GeoNodeList xfList{};
    // Check that the item is not already in the registry, or is alignable and so a new transform must *always* be made to allow unique alignment corrections; make a new item
    //See also handling in TransformProcessor/Element2GeoItem
    if ((entry = m_map.find(name)) == m_map.end()) { 
        //
        //    Add empty node list to the map
        //       
         //
        //    Loopvar Variable name
        //
        std::string varname{}, firstElement{};
        bool hasVarName = GeoXML::hasAttribute(*element, "loopvar");
        if (hasVarName) {
            varname = GeoXML::fetchAttribute(*element, "loopvar");
            // Check it is a vector
            firstElement = varname + "_0";
            if (!gmxUtil.eval.findVariable(firstElement.c_str())) {
                THROW_EXCEPTION("Error in .gmx file. Processing multicopy element with name " << name << 
                ". Found loopvar set to " << varname << ", but no vector with that name has been defined.");
            }
        }
        //
        //    Get the transformation-element
        //
        DOMElement *elXf = element->getFirstElementChild();
        std::string nodeName = GeoXML::nodeName(*elXf);
        Element2GeoItem *xFormProcessor{nullptr};
        if (hasVarName) {
            if (nodeName == "transformation") { // OK
                xFormProcessor = &(gmxUtil.tagHandler.transformation);
            } else { // Not OK
                THROW_EXCEPTION("Error in .gmx file. Processing multicopy element with name " << name <<
                ". \nIt gives loopvar therefore should have a <transformation> and not a <transformationref> (despite the DTD)\n");
        
            }
        } else {
            xFormProcessor = nodeName == "transformation"?(Element2GeoItem*) &(gmxUtil.tagHandler.transformation): 
                                                          (Element2GeoItem*) &(gmxUtil.tagHandler.transformationref);
        }
        //
        //    Produce all the transformations
        //
        GeoIntrusivePtr<GeoAlignableTransform> geoAXf{nullptr};
        GeoIntrusivePtr<GeoTransform> geoXf{nullptr};
        if (hasVarName) {
            for (int i = 0; i < nCopies; ++i) {
                gmxUtil.eval.setVariable(varname.c_str(), (varname + "_" + std::to_string(i)).c_str());
                if (alignable) {
                    geoAXf = dynamic_pointer_cast<GeoAlignableTransform>(xFormProcessor->make(elXf, gmxUtil));
                    xfList.push_back(geoAXf);
                } else {
                    geoXf = dynamic_pointer_cast<GeoTransform>( xFormProcessor->make(elXf, gmxUtil));
                    xfList.push_back(geoXf);
                }
                gmxUtil.eval.removeVariable(varname.c_str()); // Avoids a warning status in evaluator
            }
        } else {
            //
            //    If varname not given, we get the CLHEP xForm and apply it (n-1) times sequentially to place the copies
            //    First copy gets the Identity transform (i.e. placed in "default position" for object)
            //
            //Is there a better way to just get the Transform3D?
            GeoTrf::Transform3D hepXf0 = dynamic_pointer_cast<GeoTransform>(xFormProcessor->make(elXf, gmxUtil))->getTransform();
            GeoTrf::Transform3D hepXf{GeoTrf::Transform3D::Identity()}; // Identity initially for the first copy
            for (int i = 0; i < nCopies; ++i) {
                if (alignable){
                   geoAXf = make_intrusive<GeoAlignableTransform>(hepXf); 
                   xfList.push_back(geoAXf);
                }
                else{
                   geoXf = make_intrusive<GeoTransform>(hepXf) ; 
                   xfList.push_back(geoXf);
                }
                hepXf = hepXf0 * hepXf; //multiply by hepXf0 again for each copy
            }
        }
        if (!alignable) {
            m_map[name] = xfList;
        }
    } else { //if it is already in the registry, use existing version (if allowed)
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
    }
    else if (nodeName == "assemblyref") {
        idref = object->getAttribute(ref);
        DOMElement *elem = doc->getElementById(idref);
        gmxUtil.tagHandler.assembly.zeroId(elem);
    }
    else if (nodeName == "transform") { // Object is either an assemlyref or logvolref, and there is only one of these
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
                THROW_EXCEPTION(" error in " << name << ". <transform> object was neither assemblyref nor logvolref\n"
                             << "Exiting program");
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
        level =  GeoStrUtils::atoi(GeoXML::fetchAttribute(*element, "alignable"));
    }
    //
    //    Add transforms and physvols etc. to list to be added
    //
    std::map<std::string, int> index;
    for (int copy = 0; copy < nCopies; ++copy) {
        toAdd.push_back(xfList[copy]);
        int lastTransform = toAdd.size() - 1;
        objectProcessor->process(object, gmxUtil, toAdd);
        if (alignable) {
            //Loop over nodes to find the ones we expect to have. We loop only over the new items added in the last iteration (from the end of the vector to "lastTransform" which represents the size before the latest iteration)
            int nameTagIndex{-1}, idTagIndex{-1}, fpvIndex {-1}, gatIndex{-1};            
            //reverse interator
            for(int iNodeI=toAdd.size() -1; iNodeI>=lastTransform; --iNodeI){
                auto fpv = dynamic_pointer_cast<GeoVFullPhysVol>(toAdd[iNodeI]);
                if(fpv) {
                    fpvIndex=iNodeI;
                    continue;
                } 
                auto idTag = dynamic_pointer_cast<GeoIdentifierTag>(toAdd[iNodeI]);
                if(idTag) {
                    idTagIndex=iNodeI;
                    continue;
                }
                auto nameTag = dynamic_pointer_cast<GeoNameTag>(toAdd[iNodeI]);
                if(nameTag) {
                    nameTagIndex=iNodeI;
                    continue;
                } 
                auto gat = dynamic_pointer_cast<GeoAlignableTransform>(toAdd[iNodeI]);
                if(gat) {
                    gatIndex=iNodeI;
                    continue;
                }
            } 
            if((idTagIndex!=-1) && (nameTagIndex!=-1)){
                msglog << MSG::DEBUG << "copy = " << copy << "; level = " << level << endmsg;
                msglog << MSG::DEBUG << "Add Alignable named ";
                msglog << MSG::DEBUG << dynamic_pointer_cast<GeoNameTag>(toAdd[nameTagIndex])->getName();
                msglog << MSG::DEBUG <<" with id ";
                msglog << MSG::DEBUG << dynamic_pointer_cast<GeoIdentifierTag>(toAdd[idTagIndex])->getIdentifier() << endmsg;  
            }
            else{
                msglog << MSG::DEBUG << "copy = " << copy << "; level = " << level << endmsg;
                msglog << MSG::DEBUG << "Add Alignable (no name/ID detected)" << endmsg;
            }

            if(gatIndex == -1) msglog << MSG::WARNING <<"no GeoAlignableTransform found!"<<endmsg;
            if(fpvIndex == -1) msglog << MSG::WARNING <<"no GeoFullPhysVol found!"<<endmsg;

            gmxUtil.positionIndex.incrementLevel(); // Logvol has unfortunately already decremented this; temp. restore it
            gmxUtil.positionIndex.indices(index, gmxUtil.eval);
            //splitting sensors where we would like multiple DetectorElements per GeoVFullPhysVol (e.g.ITk Strips)
            if (GeoXML::hasAttribute(*element, "splitLevel")) {
                int splitLevel = gmxUtil.evaluate(GeoXML::fetchAttribute(*element, "splitLevel").c_str());
                for(int i=0;i<splitLevel;i++){
                    std::string field = "eta_module";//eventually specify in Xml the field to split in?
                    std::pair<std::string,int> extraIndex(field,i);
                    gmxUtil.gmxInterface().addSplitAlignable(level, index, extraIndex,
                                                             dynamic_pointer_cast<GeoVFullPhysVol>(toAdd[fpvIndex]),
                                                             dynamic_pointer_cast<GeoAlignableTransform>(toAdd[gatIndex]));
                }
            }
            else gmxUtil.gmxInterface().addAlignable(level, index, 
                                                     dynamic_pointer_cast<GeoVFullPhysVol>(toAdd[fpvIndex]),
                                                     dynamic_pointer_cast<GeoAlignableTransform>(toAdd[gatIndex]));
            gmxUtil.positionIndex.decrementLevel(); // Put it back where it was
        }
    }

    XMLString::release(&ref);
}

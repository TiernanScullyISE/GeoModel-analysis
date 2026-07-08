/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//
//   replicaX element processor.
//

#include "GeoModelXml/ReplicaXYArraysProcessor.h"

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
#include "GeoModelXml/StringWrappers.h"


using namespace xercesc;
using namespace std;

void ReplicaXYarraysProcessor::process(const DOMElement *element, GmxUtil &gmxUtil, GeoNodeList &toAdd) {
    char *toRelease;
    XMLCh *ref = XMLString::transcode("ref");
    XMLCh * alignable_tmp = XMLString::transcode("alignable");
    const XMLCh *idref;
    DOMDocument *doc = element->getOwnerDocument();

    bool alignable = element->hasAttribute(alignable_tmp);
    
    XMLCh * name_tmp = XMLString::transcode("name");
    toRelease = XMLString::transcode(element->getAttribute(name_tmp));
    string name(toRelease);
    XMLString::release(&toRelease);
    XMLString::release(&name_tmp);
    //
    //    How many copies?
    //
    int nCopies{0};
    XMLCh * n_tmp = XMLString::transcode("n");
    toRelease = XMLString::transcode(element->getAttribute(n_tmp));
    nCopies = gmxUtil.evaluate(toRelease);
    XMLString::release(&toRelease);
    XMLString::release(&n_tmp);
    
    //
    //    Xcoordinates/Ycoordinates arrays
    //
    string x_varname,y_varname, firstElement;
    vector<double> xPos{}, yPos{};
    XMLCh * loopvar_tmp = XMLString::transcode("xCoordinates");
    bool hasVarname = (element->getAttributeNode(loopvar_tmp) != 0);
    if (hasVarname) {
        toRelease = XMLString::transcode(element->getAttribute(loopvar_tmp));
        x_varname = toRelease;
        XMLString::release(&toRelease);
        XMLString::release(&loopvar_tmp);
        // Check it is a vector
        firstElement = x_varname + "_0";
        if (!gmxUtil.eval.findVariable(firstElement.c_str())) {
            THROW_EXCEPTION("Error in .gmx file. Processing multicopy element with name " << name << 
            ". Found xCoordinates set to " << x_varname << ", but no vector with that name has been defined.");           
        }
        
        for (int i=0;i<nCopies;++i) {
          ostringstream tempstr;
          tempstr<<x_varname + "_"<<i;
          if (!gmxUtil.eval.findVariable((tempstr.str()).c_str())){
            THROW_EXCEPTION("Error whem evaluating xPos, "<<tempstr.str()<<" not found!!");
          }
          xPos.push_back(gmxUtil.evaluate(tempstr.str().c_str()));
        }
        
    }
    loopvar_tmp = XMLString::transcode("yCoordinates");
    hasVarname = (element->getAttributeNode(loopvar_tmp) != 0);
    if (hasVarname) {
        toRelease = XMLString::transcode(element->getAttribute(loopvar_tmp));
        y_varname = toRelease;
        XMLString::release(&toRelease);
        XMLString::release(&loopvar_tmp);
        // Check it is a vector
        firstElement = y_varname + "_0";
        if (!gmxUtil.eval.findVariable(firstElement.c_str())) {
            THROW_EXCEPTION("Error in .gmx file. Processing multicopy element with name " << name << 
            ". Found yCoordinates set to " << y_varname << ", but no vector with that name has been defined.");
        }
       
        for (int i=0;i<nCopies;++i) {
            ostringstream tempstr;
            tempstr<<y_varname + "_"<<i;
            if (!gmxUtil.eval.findVariable(tempstr.str().c_str())) {
              THROW_EXCEPTION("Error whem evaluating xPos, "<<tempstr.str()<<" not found!!");            
            }         
            yPos.push_back(gmxUtil.evaluate(tempstr.str().c_str()));
        }        
    }
    //
    //  z value
    // 
    double zVal=0;
    XMLCh * z_tmp = XMLString::transcode("zValue");
    toRelease = XMLString::transcode(element->getAttribute(z_tmp));
    zVal = gmxUtil.evaluate(toRelease);
    XMLString::release(&toRelease);
    XMLString::release(&z_tmp);

    //
    //    See if it is in the map; if so, xfList is already done. If not, fill xfList.
    //
    map<string, GeoNodeList>::iterator entry;
    GeoNodeList *xfList{nullptr};
    if ((entry = m_map.find(name)) == m_map.end()) { // Not in registry; make a new item
        //
        //    Add empty node list to the map
        //        
        xfList = &m_map[name];
        //
        //    Produce all the transformations
        //
        GeoIntrusivePtr<GeoAlignableTransform> geoAXf{};
        GeoIntrusivePtr<GeoTransform> geoXf{};
        //
        //    If varname not given, we get the CLHEP xForm and raise it to the power i, so NOT applied to first object.
        //    No transform (i.e. identity) for the first; so one less transform than objects
        //
       GeoTrf::Transform3D hepXf0=GeoTrf::Transform3D::Identity();
        if (alignable) {
            geoAXf = make_intrusive<GeoAlignableTransform>(hepXf0) ;
            hepXf0 = geoAXf->getTransform();
        } else {
            geoXf = makeTransform (hepXf0);
            hepXf0 = geoXf->getTransform();
        }
        GeoTrf::Transform3D hepXf=hepXf0; 

        for (int i = 0; i < nCopies; ++i) {

            hepXf = GeoTrf::Translate3D(xPos[i], yPos[i], zVal);
                        
            if (gmxUtil.gmxInterface().logLevel(LogLevel::VERBOSE)) msglog << MSG::VERBOSE << "alignable = " << alignable << endmsg;
            if (alignable) {
                geoAXf = make_intrusive<GeoAlignableTransform>(hepXf);
                xfList->push_back(geoAXf);

                if (gmxUtil.gmxInterface().logLevel(LogLevel::VERBOSE)) msglog << MSG::VERBOSE << "Created alignable transform for copy " << i << endmsg;
                
            } else {
                geoXf = makeTransform(hepXf);
                xfList->push_back(geoXf);
            }
            if (gmxUtil.gmxInterface().logLevel(LogLevel::VERBOSE)){
                msglog << MSG::VERBOSE << "Copy " << i 
                     << " x=" << xPos[i] 
                     << " y=" << yPos[i] 
                     << endmsg;
            }

        }
    }else {
        xfList = &entry->second;
    }
    //

    //
    //    Get object to be copied
    //
    DOMElement *object = element->getLastElementChild();
    toRelease = XMLString::transcode(object->getNodeName());
    string nodeName(toRelease);
    XMLString::release(&toRelease);
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
            THROW_EXCEPTION("ReplicaXProcessor: error in " << name << ". <transform> object was neither assemblyref nor logvolref\n"
                      << "Exiting ");
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
        const auto val = GeoXML::fetchAttribute(*element, "alignable");
        if (!val.empty()) {
            level = std::stoi(val);
        }
    }
    //
    //    Add transforms and physvols etc. to list to be added
    //
    map<string, int> index;
    static std::map<std::string, int> layerCounters;
    static std::set<std::string> initializedLayers;

    for (int copy = 0; copy < nCopies; ++copy) {

        toAdd.push_back((*xfList)[copy]);
        int lastTransform = toAdd.size() - 1;

        objectProcessor->process(object, gmxUtil, toAdd);

        if (alignable) {

            int fpvIndex{-1}, gatIndex{-1};

            for (int iNodeI = toAdd.size() - 1; iNodeI >= lastTransform; --iNodeI) {

                if (fpvIndex == -1) {
                    auto fpv = dynamic_pointer_cast<GeoVFullPhysVol>(toAdd[iNodeI]);
                    if (fpv) fpvIndex = iNodeI;
                }

                if (gatIndex == -1) {
                    auto gat = dynamic_pointer_cast<GeoAlignableTransform>(toAdd[iNodeI]);
                    if (gat) gatIndex = iNodeI;
                }
            }

            // Safety checks
            if (fpvIndex == -1 || gatIndex == -1) {
                if (gmxUtil.gmxInterface().logLevel(LogLevel::WARNING)){
                msglog << MSG::WARNING
                       << "ReplicaXYarraysProcessor: skipping alignable, missing "
                       << (fpvIndex == -1 ? "GeoVFullPhysVol " : "")
                       << (gatIndex == -1 ? "GeoAlignableTransform" : "")
                       << endmsg;
                }
                continue;
            }
            
            gmxUtil.positionIndex.incrementLevel();
            gmxUtil.positionIndex.indices(index, gmxUtil.eval);

            std::string layerKey =
                std::to_string(index["endcap"]) + "_" +
                std::to_string(index["layer"]);

            if (initializedLayers.find(layerKey) == initializedLayers.end()) {
                layerCounters[layerKey] = 0;
                initializedLayers.insert(layerKey);
            }

            int& counter = layerCounters[layerKey];
            counter++;

            index["moduleInLayer"] = counter;

            if (GeoXML::hasAttribute(*element, "splitLevel")) {
                int splitLevel = gmxUtil.evaluate(
                    GeoXML::fetchAttribute(*element, "splitLevel")
                );

                for (int i = 0; i < splitLevel; i++) {

                    std::pair<std::string,int> extraIndex("split", i);

                    gmxUtil.gmxInterface().addSplitAlignable(
                        level,
                        index,
                        extraIndex,
                        dynamic_pointer_cast<GeoVFullPhysVol>(toAdd[fpvIndex]),
                        dynamic_pointer_cast<GeoAlignableTransform>(toAdd[gatIndex])
                    );
                }

            } else {

                if (gmxUtil.gmxInterface().logLevel(LogLevel::DEBUG)){
                    msglog << MSG::DEBUG
                            << "XY Replica ALIGNABLE: copy=" << copy
                            << " level=" << level
                            << "     moduleInLayer=" << index["moduleInLayer"]
                            << endmsg;
                }       
                gmxUtil.gmxInterface().addAlignable(
                    level,
                    index,
                    dynamic_pointer_cast<GeoVFullPhysVol>(toAdd[fpvIndex]),
                    dynamic_pointer_cast<GeoAlignableTransform>(toAdd[gatIndex])
                );
            }

            gmxUtil.positionIndex.decrementLevel();
            index.clear();

        }
    }
    XMLString::release(&ref);
    XMLString::release(&alignable_tmp);
}

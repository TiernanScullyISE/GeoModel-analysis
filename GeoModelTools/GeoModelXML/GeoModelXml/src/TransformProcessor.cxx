/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//
//   Processor for transform elements
//
//   Decide if I need alignable transforms from my second child
//   Add transform from first child to list
//   Create physvol using my second child; add it to list.
//
//   Updates:
//   - 2022 Apr, Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
//               Commented call to addAlignable
//
#include "GeoModelXml/TransformProcessor.h"


#include "GeoModelXml/GmxUtil.h"
#include "GeoModelXml/StringWrappers.h"

#include "GeoModelHelpers/StringUtils.h"

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <xercesc/dom/DOM.hpp>
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoVFullPhysVol.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "xercesc/util/XMLString.hpp"


//  using namespace CLHEP;
using namespace std;
using namespace xercesc;

void TransformProcessor::process(const DOMElement *element, GmxUtil &gmxUtil, GeoNodeList &toAdd) {
    
    const bool alignable = GeoXML::hasAttribute(*element, "alignable");
    //
    //    Do second element first, to find what sort of transform is needed (shape or logvol etc.?)
    //
    GeoNodeList objectsToAdd;
    DOMElement *object = element->getLastElementChild();
  
    std::string tagName = GeoXML::tagName(*object);
    gmxUtil.processorRegistry.find(tagName)->process(object, gmxUtil, objectsToAdd);
    //
    //    Get the transformation
    //
    DOMElement *transformation = element->getFirstElementChild();
    tagName = GeoXML::tagName(*transformation); // transformation or transformationref
 
    // TODO:  ******* Should check here that an alignable transform is given an alignable transformation and object; to be done
    // If alignable is true, this is passed on to Element2GeoItem::process in order to allow multiple transforms to be created with the same name
    // This is necessary because we need an AlignableTransform for each element we want to align, even if the initial transform is identical.
    // Without this, the de-duplication will return an already-exisiting transform if one already exists with that name
 
    toAdd.push_back(dynamic_pointer_cast<GeoGraphNode>(gmxUtil.geoItemRegistry.find(tagName)->process(transformation, gmxUtil)));
    //
    //    Add transformation to DetectorManager via GmxInterface, if it is alignable
    //
    if (alignable) { 
        int level = GeoStrUtils::atoi(GeoXML::fetchAttribute(*element, "alignable"));
        map<string, int> index;
        gmxUtil.positionIndex.incrementLevel(); // Logvol has unfortunately already decremented this; temp. restore it
        gmxUtil.positionIndex.indices(index, gmxUtil.eval);

        //Checking all objects to find right one not so efficient - Define const int in LogvolProcessor?
        //sanity check... see if we find a FullPhysVol somewhere in the object list...
        GeoIntrusivePtr<GeoVFullPhysVol>  fpv;
        for(GeoIntrusivePtr<GeoGraphNode> iFpv: objectsToAdd){ //maybe this needs to be in reverse???
            fpv = dynamic_pointer_cast<GeoVFullPhysVol> (iFpv);
            if (fpv) break; //if we find it, use it...
        }

        //now check that the AlignableTransform is actually valid!
        auto gat = dynamic_pointer_cast<GeoAlignableTransform>(toAdd.back());
        if(fpv && gat) {
            //splitting sensors where we would like multiple DetectorElements per GeoVFullPhysVol (e.g.ITk Strips)
            int splitLevel = 1;
	        if (GeoXML::hasAttribute(*element, "splitLevel")) {
                splitLevel = gmxUtil.evaluate(GeoXML::fetchAttribute(*element, "splitLevel").c_str());
                for(int i=0;i<splitLevel; ++i) {
                    std::string field = "eta_module";//eventually specify in Xml the field to split in?
                    std::pair<std::string,int> extraIndex(field, i);         
                    gmxUtil.gmxInterface().addSplitAlignable(level, index, extraIndex, fpv, gat);
                }
            } else { 
                gmxUtil.gmxInterface().addAlignable(level, index, fpv, gat);
            }
        } else{
             std::cout<<"WARNING:";
             if(!gat) std::cout<<" No valid AlignableTransform";
             if(!fpv) std::cout<<" No valid FullPhysicalVolume";
             std::cout<<" found for ";
            for (map<string, int>::iterator i = index.begin(); i != index.end(); ++i) {
            std::cout << i->second << "   ";
            }
            std::cout<<std::endl;
        }

        gmxUtil.positionIndex.decrementLevel(); 
    }
    //
    //    And add the name and physvol etc. after the transformation
    //
    toAdd.insert(toAdd.end(), objectsToAdd.begin(), objectsToAdd.end());
}

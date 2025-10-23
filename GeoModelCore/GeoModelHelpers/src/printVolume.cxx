/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelKernel/GeoFullPhysVol.h"

#include "GeoModelHelpers/printVolume.h"
#include "GeoModelHelpers/GeoShapeUtils.h"
#include "GeoModelHelpers/getChildNodesWithTrf.h"
#include "GeoModelHelpers/StringUtils.h"
#include "GeoModelHelpers/TransformToStringConverter.h"
#include <sstream>

constexpr unsigned indentStep =2;

std::string printVolume(const PVConstLink& volume, const unsigned childIndent);

std::string printVolume(const PVConstLink& volume) {
    return printVolume(volume, indentStep);
}
std::string printVolume(const PVConstLink& volume, const unsigned childIndent) {
    std::stringstream outStr{};
    
    if (volume->isShared()) {
        outStr<<"shared volume ("<<(volume->refCount()-1)<<") -- ";
    } else {
        const GeoVPhysVol* pv = volume;
        if(typeid(*pv) == typeid(GeoFullPhysVol)) {
            outStr<<"fullPhysVol -- ";
            outStr<<"abs. position: "<<GeoTrf::toString(static_cast<const GeoFullPhysVol&>(*volume).getAbsoluteTransform(),true)<<", ";
        }
        outStr<<"rel. position: "<<GeoTrf::toString(volume->getX(), true)<<", "; 
    }
    outStr<<"logical volume: "<<volume->getLogVol()->getName()<<", ";
    outStr<<"material: "<<volume->getLogVol()->getMaterial()->getName()<<", ";
    outStr<<"shape: "<<printGeoShape(volume->getLogVol()->getShape());
    if (!volume->getNChildVols()) {
        return outStr.str();
    }
    outStr<<", # children: "<<volume->getNChildVols()<<std::endl;
    std::vector<GeoChildNodeWithTrf> children = getChildrenWithRef(volume, false);
    for (unsigned int ch = 0 ; ch < children.size(); ++ch) {
        const GeoChildNodeWithTrf& child{children[ch]};
        outStr<<GeoStrUtils::whiteSpaces(childIndent, " ")<<(ch+1)<<"): ";
        if (child.nodeName != child.volume->getLogVol()->getName()){
            outStr<<"<"<<child.nodeName<<">  ";
        }
        if (child.volume->isShared()) {
            outStr<<"rel. position: "<<GeoTrf::toString(child.transform, true)<<", "; 
        }
        outStr<<printVolume(children[ch].volume, childIndent + indentStep)<<std::endl;
    }
    return outStr.str();
}

std::string printNodes(const PVConstLink& volume) {
    std::stringstream outStr{};
    for (unsigned ch = 0 ; ch < volume->getNChildNodes(); ++ch) {
        const GeoGraphNode* chNode = *volume->getChildNode(ch);
        outStr<<"  --- "<<(ch+1)<<"):  "<<typeid(*chNode).name()<<std::endl;
    }
    return outStr.str();
}

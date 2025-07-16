/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoNodeAction.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoCountVolAction.h"
#include "GeoModelKernel/GeoCountVolAndSTAction.h"
#include "GeoModelKernel/GeoAccessVolumeAction.h"

#include <algorithm>


GeoPhysVol::GeoPhysVol(const GeoLogVol* LogVol):
      GeoVPhysVol{LogVol}{}


void GeoPhysVol::exec(GeoNodeAction *action) const {
  //    
  // Put this node on the head of the path:    
  //    
  action->getPath()->push(this);
  if(action->getDepthLimit()
     && action->getPath()->getLength() - 1 > action->getDepthLimit()) {
    action->getPath ()->pop ();
    return;
  }
  //    
  // Perform the action on this node:    
  //    
  action->handlePhysVol(this);
  if(action->shouldTerminate()) {
    action->getPath()->pop();
    return;
  }
  //    
  // Pass this on to the children.    
  //    
  if(action->getDepthLimit()
     && action->getPath()->getLength() > action->getDepthLimit()) {
  }
  else {
    // FIXME: m_daughters access is now protected in other methods, but having the lock here makes a deadlock
    // std::scoped_lock lk(m_muxVec);
    // TODO: Think more thouroughly about thread-safe of this class...!!
    for(const GeoIntrusivePtr<GeoGraphNode>& child : m_daughters) {
      child->exec(action);
      if(action->shouldTerminate()) {
          action->getPath()->pop();
          return;
      }
    }
  }
  //    
  // Take this node back off the head of the path:    
  //    
  action->getPath()->pop();
}

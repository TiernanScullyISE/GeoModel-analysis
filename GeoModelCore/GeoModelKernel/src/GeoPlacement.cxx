/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelKernel/GeoPlacement.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoVSurface.h"
#include "GeoModelKernel/GeoTransform.h"



void GeoPlacement::dockTo(GeoVPhysVol* parent) {
  std::unique_lock guard{m_mutex};
   if (!m_parentPtr && m_uniqueParent) {
      m_parentPtr = parent;
   } else {
      m_parentPtr = nullptr;
      m_uniqueParent = false;
   }
}


bool GeoPlacement::isShared() const {
  std::shared_lock guard{m_mutex};
  return !m_uniqueParent;
}
GeoIntrusivePtr<const GeoVPhysVol> GeoPlacement::getParent() const {
  std::shared_lock guard{m_mutex};
  return GeoIntrusivePtr<const GeoVPhysVol>{m_parentPtr};
}


GeoTrf::Transform3D GeoPlacement::getX(const GeoVAlignmentStore* store) const {
  //
  // Check we are not shared:
  //
  if (isShared()) {
    THROW_EXCEPTION("Transform requested from shared volume");
  }
  GeoTrf::Transform3D xform{GeoTrf::Transform3D::Identity()};

  if (!m_parentPtr) {
      return xform;
  }
  //
  // Get the address of the first graph node, from the parent:
  //
  const GeoGraphNode * const * fence =  m_parentPtr->getChildNode(0);
  const GeoGraphNode * const * node1 =  m_parentPtr->findChildNode(this);
  
  for(const GeoGraphNode * const * current = node1 - 1; current>=fence; current--) {

    // If this happens, we are done, compute & return--------------------//
    //                                                                   //
    if (dynamic_cast<const GeoVPhysVol *>(*current)) break;              //
    //-------------------------------------------------------------------//

    // If this happens, we are done, compute & return--------------------//
    //                                                                   //
    if (dynamic_cast<const GeoVSurface *>(*current)) break;              //
    //-------------------------------------------------------------------//

    // If this happens, we are done, compute & return--------------------//
    //                                                                   //
    if (dynamic_cast<const GeoSerialTransformer *>(*current)) break;     //
    //-------------------------------------------------------------------//

    const GeoTransform *xf = dynamic_cast<const GeoTransform *> (*current);
    
    //-------------------------------------------------------------------//
    // If this happens, accumulate into transform                        //
    if (xf) xform  = xf->getTransform(store)*xform;                      //
    //-------------------------------------------------------------------//
  }
  return xform;  
}

GeoTrf::Transform3D GeoPlacement::getDefX(const GeoVAlignmentStore* store) const {
  //
  // Check we are not shared:
  //
  if (isShared()) THROW_EXCEPTION("Transform requested from shared volume");
  GeoTrf::Transform3D xform{GeoTrf::Transform3D::Identity()};

  if (!m_parentPtr) {
      return xform;
  }

  //
  // Get the address of the first graph node, from the parent:
  //
  const GeoGraphNode * const * fence =  m_parentPtr->getChildNode(0);
  const GeoGraphNode * const * node1 =  m_parentPtr->findChildNode(this);
  
  for(const GeoGraphNode * const * current = node1 - 1; current>=fence; current--) {

    // If this happens, we are done, compute & return--------------------//
    //                                                                   //
    if (dynamic_cast<const GeoVPhysVol *>(*current)) break;              //
    //-------------------------------------------------------------------//

    // If this happens, we are done, compute & return--------------------//
    //                                                                   //
    if (dynamic_cast<const GeoVSurface *>(*current)) break;              //
    //-------------------------------------------------------------------//

    // If this happens, we are done, compute & return--------------------//
    //                                                                   //
    if (dynamic_cast<const GeoSerialTransformer *>(*current)) break;     //
    //-------------------------------------------------------------------//

    const GeoTransform *xf = dynamic_cast<const GeoTransform *> (*current);
    
    //-------------------------------------------------------------------//
    // If this happens, accumulate into transform                        //
    if (xf) xform  = xf->getDefTransform(store)*xform;                   //
    //-------------------------------------------------------------------//
  }
  return xform;
  
}

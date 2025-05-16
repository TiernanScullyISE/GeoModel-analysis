/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoAccessSurfaceAction.h"

#include <string>

GeoAccessSurfaceAction::GeoAccessSurfaceAction(unsigned int Index, const GeoVAlignmentStore* store)
  : m_transform(GeoTrf::Transform3D::Identity())
  , m_defTransform(GeoTrf::Transform3D::Identity())
  , m_index(Index)
  , m_alignStore(store)
{
  setDepthLimit (1);
  m_pendingTransformList.reserve(1);
}

GeoAccessSurfaceAction::~GeoAccessSurfaceAction() = default;

void GeoAccessSurfaceAction::handleTransform (const GeoTransform *xform)
{
  m_pendingTransformList.push_back (xform);
}


// in this action, we only care about virtual surface
// get rid of transformations corresponding to physics volume
void GeoAccessSurfaceAction::handlePhysVol (const GeoPhysVol *vol)
{
  if(getPath()->getLength()-1 == 0)
    return;

  else{
    m_pendingTransformList.erase (m_pendingTransformList.begin (),
				  m_pendingTransformList.end ());    
  }
}


// in this action, we only care about virtual surface
// get rid of transformations corresponding to full physics volume
void GeoAccessSurfaceAction::handleFullPhysVol (const GeoFullPhysVol *vol)
{
  if(getPath()->getLength()-1 == 0)
    return;

  else{
    m_pendingTransformList.erase (m_pendingTransformList.begin (),
				  m_pendingTransformList.end ());    
  }
}


void GeoAccessSurfaceAction::handleVSurface (const GeoVSurface *surf)
{
  // Virtual Surface is the child of a Physical Volume, so no need to check.        
  //
  /*     
  if(getPath()->getLength()-1 == 0)
    return;
  //
  */
       
  // check if the index equals the counter:     
  //       
  if(m_index == m_counter) {
    m_surface = surf;
    unsigned int listSize = m_pendingTransformList.size ();
    if(listSize == 0) {
      // do nothing.... 
    }
    else if(listSize == 1) {
      m_transform = m_pendingTransformList[0]->getTransform(m_alignStore);
      m_defTransform = m_pendingTransformList[0]->getDefTransform(m_alignStore);
    }
    else {
      for(unsigned int t = 0; t < m_pendingTransformList.size (); t++) {
	m_transform = m_transform * m_pendingTransformList[t]->getTransform(m_alignStore);
	m_defTransform = m_defTransform * m_pendingTransformList[t]->getDefTransform(m_alignStore);
      }
    }
    terminate();
  }
  else {
    m_pendingTransformList.erase (m_pendingTransformList.begin (),
				  m_pendingTransformList.end ());
  }
  //     
  // Increment the counter:     
  //     
  m_counter++;
}

VSConstLink GeoAccessSurfaceAction::getSurface () const
{
  return m_surface;
}

const GeoTrf::Transform3D & GeoAccessSurfaceAction::getTransform () const
{
  return m_transform;
}

const GeoTrf::Transform3D & GeoAccessSurfaceAction::getDefTransform () const
{
  return m_defTransform;
}

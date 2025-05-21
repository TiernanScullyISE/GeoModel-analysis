/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOCOUNTSURFACEACTION_H
#define GEOMODELKERNEL_GEOCOUNTSURFACEACTION_H

#include "GeoModelKernel/GeoNodeAction.h"

class GeoCountSurfaceAction final: public GeoNodeAction
{
 public:
  GeoCountSurfaceAction ();
  virtual ~GeoCountSurfaceAction() = default;

  //	Handles a virtual surface.
  virtual void handleVSurface (const GeoVSurface *) override;
  
  //	The virtual surface count.
  unsigned int getCount () const;


  unsigned int m_count{0};
};

inline unsigned int GeoCountSurfaceAction::getCount () const
{
  return m_count;
}

#endif

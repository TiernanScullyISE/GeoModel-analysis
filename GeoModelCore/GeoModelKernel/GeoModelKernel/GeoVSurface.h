/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 * 
 *
 * Created on: April, 2024
 * Author: Rui XUE <r.xue@cern.ch><rux23@pitt.edu>
 *
 */
#ifndef GEOMODELKERNEL_GeoVSurface_H
#define GEOMODELKERNEL_GeoVSurface_H

/**
 * @class GeoVSurface
 * 
 * @brief An abstract class of virtual surfaces
 *
 */
#include "GeoModelKernel/GeoVSurfaceShape.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelKernel/GeoPlacement.h"
#include "GeoModelKernel/GeoNodePositioning.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/throwExcept.h"
#include <iostream>

#include <exception>
#include <sstream>


class GeoVSurface : public GeoPlacement, public GeoNodePositioning {
 public:
    using VSConstLink = GeoIntrusivePtr<const GeoVSurface>;
    
    GeoVSurface(const GeoVSurfaceShape *SurfShape):
      GeoNodePositioning{this}, m_surfaceshape{SurfShape}{}
    
    // method from GeoPlacement
    virtual void exec(GeoNodeAction *action) const;

    /// Returns the number of child nodes, virtual surface has no child, return 0
    unsigned int getNChildVols() const {return 0;}    
    
    //	Returns the shape of the virtual surface.
    const GeoVSurfaceShape* getShape () const {
        return m_surfaceshape;
    }

    constexpr bool typeQuery(GeoGraphNodeType in) const final { return in == GeoGraphNodeType::NodeEndpoint; }
    
    //  Justify whether the point is on the surface
    bool isOnSurface(const double Px, const double Py, const double Pz) const;
  
 protected:
  virtual ~GeoVSurface() = default;
  
 private:
   GeoIntrusivePtr<const GeoVSurfaceShape>  m_surfaceshape;
};

using VSConstLink = GeoVSurface::VSConstLink;
#endif

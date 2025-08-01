
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GEOMODELKERNEL_GEOPLACEMENT_H
#define GEOMODELKERNEL_GEOPLACEMENT_H
#include "GeoModelKernel/GeoVAlignmentStore.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoGraphNode.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"

#include <optional>
#include <shared_mutex>

/** 
 *  @brief: Basic geo graphNode which contains placement information w.r.t. its mother volume
*/
class GeoPlacement: public GeoGraphNode {

public:
  
    /** @brief:  Default constructor */
    GeoPlacement() = default;
    /** @brief: Returns the transformation to go from frame of the GeoPlacement into the frame of the
     *          mother volume. The transformation takes alignment displacements into account */
    GeoTrf::Transform3D getX(const GeoVAlignmentStore* store=nullptr) const;
    /** @brief: Returns the transformation to go from frame of the GeoPlacement into the frame of the
     *          mother volume. The method ignores all alignment deformations */    
    GeoTrf::Transform3D getDefX(const GeoVAlignmentStore* store=nullptr) const;

    /// Returns true if the physical volume is accessed by more than one parent.
    /// Should check this before trusting the parent pointer.
    bool isShared() const;

    void dockTo (GeoVPhysVol*  parent) override final;    
    /// Gets the parent, if the parent is unique, and otherwise returns a nullptr pointer.
    GeoIntrusivePtr<const GeoVPhysVol>  getParent() const;


  protected:
      virtual ~GeoPlacement() = default;
  private:
      mutable std::shared_mutex m_mutex{};
      const GeoVPhysVol* m_parentPtr{nullptr};
      bool m_uniqueParent{true};

      

};


#endif

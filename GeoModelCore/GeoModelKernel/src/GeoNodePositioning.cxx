/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoNodePositioning.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelKernel/GeoVPhysVol.h"

#include <mutex>
#include <thread>
#include <cassert>


GeoNodePositioning::GeoNodePositioning(const GeoPlacement* node):
    m_node{node} {
    
}

  GeoTrf::Transform3D GeoNodePositioning::accumulateTrfs(GeoVAlignmentStore* store, 
                                                         const AccumlType type) const {
    GeoTrf::Transform3D tProd{GeoTrf::Transform3D::Identity()};
    
    if(m_node->isShared()) {
      THROW_EXCEPTION("GeoPlacement node is shared");
    }
    GeoIntrusivePtr<const GeoPlacement> child{m_node}, parent{m_node->getParent()};
    while (parent) {
      const GeoTrf::Transform3D transform{type == AccumlType::Aligned ? child->getX(store) : child->getDefX(store)};
      tProd = transform * tProd;
      child = parent;
      if(child->isShared()) {
        THROW_EXCEPTION("GeoPlacement node is shared.");
      } else {
        parent = child->getParent();
      }
    }
    return tProd;
}

const GeoTrf::Transform3D& 
  GeoNodePositioning::getAbsoluteTransform(GeoVAlignmentStore* store) const {

  if (!store) {
      std::unique_lock guard{m_mutex};
      if (!m_absTransf) {
          m_absTransf = std::make_unique<GeoTrf::Transform3D>(accumulateTrfs(store, AccumlType::Aligned));
      }
      assert(m_absTransf.get() != nullptr);
      return *m_absTransf;
  }

  const GeoTrf::Transform3D* cachedTrf{store->getAbsPosition(this)};
  if (cachedTrf) {
      return *cachedTrf;
  }
  store->setAbsPosition(this, accumulateTrfs(store, AccumlType::Aligned));
  cachedTrf = store->getAbsPosition(this);
  
  assert(cachedTrf != nullptr);

  return *cachedTrf;
 
}


const GeoTrf::Transform3D& GeoNodePositioning::getCachedAbsoluteTransform(const GeoVAlignmentStore* store) const {
  if(!store) {
      std::shared_lock guard{m_mutex};
      if (m_absTransf) return *m_absTransf;
  } else {
      const GeoTrf::Transform3D* storedPos = store->getAbsPosition(this);    
      assert(storedPos != nullptr);    
      if(storedPos) return *storedPos;
  }
  THROW_EXCEPTION("Failed to find the cached absolute transform ");
}

void GeoNodePositioning::clearPositionInfo() const {
    std::unique_lock guard{m_mutex};
    m_absDefTransf.reset();
    m_absTransf.reset();
}


const GeoTrf::Transform3D& GeoNodePositioning::getDefAbsoluteTransform(GeoVAlignmentStore* store) const {
   if (!store) {
      std::unique_lock guard{m_mutex};
      if (!m_absDefTransf) {
          m_absDefTransf = std::make_unique<GeoTrf::Transform3D>(accumulateTrfs(store, AccumlType::Default));
    }
    assert(m_absDefTransf.get() != nullptr);
    return *m_absDefTransf;
  }

  const GeoTrf::Transform3D* cachedTrf{store->getDefAbsPosition(this)};
  if (cachedTrf) {
    return *cachedTrf;
  }
  store->setDefAbsPosition(this, accumulateTrfs(store, AccumlType::Default));
  cachedTrf = store->getDefAbsPosition(this);
  
  assert(cachedTrf != nullptr);

  return *cachedTrf;
}


const GeoTrf::Transform3D& 
  GeoNodePositioning::getCachedDefAbsoluteTransform(const GeoVAlignmentStore* store) const {
  if(!store) {
    std::shared_lock guar{m_mutex};
    if(m_absDefTransf) return *m_absDefTransf;
  } else {
    const GeoTrf::Transform3D* storedPos = store->getDefAbsPosition(this);

    assert(storedPos != nullptr);
    
    if(storedPos) return *storedPos;
  }
  THROW_EXCEPTION("Failed to find the cached default absolute transform. ");
}




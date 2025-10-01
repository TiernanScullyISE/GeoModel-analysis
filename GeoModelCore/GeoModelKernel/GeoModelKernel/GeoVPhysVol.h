/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOVPHYSVOL_H
#define GEOMODELKERNEL_GEOVPHYSVOL_H

#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelKernel/GeoDefinitions.h"

#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPlacement.h"
#include "GeoModelKernel/GeoGraphNode.h"


#include <string>
#include <shared_mutex>
#include <optional>

class GeoVolumeAction;
class GeoVAlignmentStore;

class GeoVPhysVol: public GeoPlacement {
 public:
    using PVLink = GeoIntrusivePtr<GeoVPhysVol>;
    using PVConstLink = GeoIntrusivePtr<const GeoVPhysVol>;

    GeoVPhysVol(const GeoLogVol* LogVol);
  
    constexpr bool typeQuery(GeoGraphNodeType in) const final { return in == GeoGraphNodeType::NodeEndpoint; }

    /// Returns the index of a specific daughter volume.
    std::optional<unsigned int> indexOf(const PVConstLink& daughter) const;

    /// Returns the logical volume.
    const GeoLogVol* getLogVol() const {
       return m_logVol;
    }

    /// Returns the number of child physical volumes.
    unsigned int getNChildVols() const;

    /// Returns the ith child volume
    PVConstLink  getChildVol(unsigned int index) const;

    /// Returns the transform to the ith volume.
    GeoTrf::Transform3D getXToChildVol(unsigned int index, const GeoVAlignmentStore* store=nullptr) const;

    /// Returns the default transform to the ith volume.
    GeoTrf::Transform3D getDefXToChildVol(unsigned int index, const GeoVAlignmentStore* store=nullptr) const;

    /// Returns the name of the child.
    std::string getNameOfChildVol(unsigned int i) const;

    /// Applies an action to the volume. The volume executes the action and the applies
    /// it to the children, either from the top down or the bottom up, depending upon
    /// the configuration of the action.
    void apply(GeoVolumeAction* action) const;

 
    /// Returns the id of the child.
    std::optional<int> getIdOfChildVol(unsigned int i) const;

    /// Returns the number of child physical volumes and Serial Transformers.
    unsigned int getNChildVolAndST() const;

    /// Returns the number of child nodes
    unsigned int getNChildNodes() const;
    
    const GeoGraphNode* const* getChildNode (unsigned int i) const;
    const GeoGraphNode* const* findChildNode(const GeoGraphNode *n) const;

    /// Adds a Graph Node to the Geometry Graph
    void add(const GeoIntrusivePtr<GeoGraphNode>& graphNode);
  protected:
    virtual ~GeoVPhysVol() = default;

 private:
    GeoIntrusivePtr<const GeoLogVol> m_logVol{};
  protected:
    std::vector<GeoIntrusivePtr<GeoGraphNode>> m_daughters{};
    mutable std::shared_mutex m_muxVec{};

};

using PVLink = GeoVPhysVol::PVLink;
using PVConstLink = GeoVPhysVol::PVConstLink;

using GeoPVLink = PVLink;
using GeoPVConstLink = PVConstLink;

#endif

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOTRANSFORM_H
#define GEOMODELKERNEL_GEOTRANSFORM_H 

/**
 * @class GeoTransform
 *
 * @brief A basic geometrical (Euclidean) transform.  Can return a GeoTrf::Transform3D.
 * Reference counted.  There is no (mis)alignment present in this class.  For a transform
 * that can be  (mis)aligned, see GeoAlignableTransform
 * 
 */

#include "GeoModelKernel/GeoGraphNode.h"
#include "GeoModelKernel/GeoDefinitions.h"

class GeoVAlignmentStore;

class GeoTransform : public GeoGraphNode {
 public:
    GeoTransform(const GeoTrf::Transform3D& transform);


    /// Gets the total transformation.
    virtual GeoTrf::Transform3D getTransform(const GeoVAlignmentStore* store=nullptr) const;

    /// Gets the default transformation (no alignment correction)
    const GeoTrf::Transform3D& getDefTransform(const GeoVAlignmentStore* store=nullptr) const;

    ///	Executes a GeoNodeAction.
    virtual void exec(GeoNodeAction *action) const override final;

    constexpr bool typeQuery(GeoGraphNodeType in) const final { return in == GeoGraphNodeType::GeoTransform; }
  
 protected:
    virtual ~GeoTransform()  = default;

 private:
    // The Euclidean (Rigid Body) transform.
    GeoTrf::Transform3D m_transform{GeoTrf::Transform3D::Identity()};
};

#endif

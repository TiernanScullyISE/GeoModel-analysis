/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GeoAccessSurfaceAction_h
#define GeoAccessSurfaceAction_h 1

#include "GeoModelKernel/GeoNodeAction.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include <vector>

class GeoVSurface;
class GeoVPhysVol;
class GeoTransform;
class GeoVAlignmentStore;
using VSConstLink = GeoIntrusivePtr<const GeoVSurface>;
/**
 * @class GeoAccessSurfaceAction
 *
 * @brief Accesses virtual surfaces, returning the ith surface and also its transformation relative to the parent.
 */

class GeoAccessSurfaceAction final : public GeoNodeAction
{
 public:
  GeoAccessSurfaceAction (unsigned int Index, const GeoVAlignmentStore* store);
  virtual ~GeoAccessSurfaceAction() = default;

  /// Handles a Transform.
  virtual void handleTransform (const GeoTransform *xform) override;
  
  /// Handles a physical volume, to refresh the pendingTransformlist
  virtual void handlePhysVol (const GeoPhysVol *vol) override;
  
  /// Handles a full physical volume, to refresh the pendingTransformlist
  virtual void handleFullPhysVol (const GeoFullPhysVol *vol) override;  
  
  /// Handles a virtual surface.
  virtual void handleVSurface (const GeoVSurface *surface) override;

  /// Returns the selected virtual surface or nullptr if not found.
  VSConstLink getSurface () const;

  /// Returns the transformation to the surface.
  const GeoTrf::Transform3D & getTransform () const;

  /// Returns the default ransformation to the surface.
  const GeoTrf::Transform3D & getDefTransform () const;

 private:
  /// Returns a pointer to the ith virtual surface under this one.
  VSConstLink m_surface{};

  /// The transformation to the ith surface.
  GeoTrf::Transform3D m_transform{GeoTrf::Transform3D::Identity()};

  /// The default transformation to the ith surface.
  GeoTrf::Transform3D m_defTransform{GeoTrf::Transform3D::Identity()};

  /// The surface which we are interested in seeking.
  unsigned int m_index{0};

  /// The surface which we are interested in seeking.
  unsigned int m_counter{0};

  /// List of Pending Transformations.
  std::vector<const GeoTransform *>  m_pendingTransformList{};

  const GeoVAlignmentStore* m_alignStore{nullptr};
};

#endif

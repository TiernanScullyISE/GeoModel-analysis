/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelHelpers/GeoVolumeUtils.h"
#include "GeoModelKernel/GeoVFullPhysVol.h"



/// @brief GeoNodePositioning is the class which handles
///        the absolute placement of a GeoVPhysVol. Its constructor
///        is protected though -> create helper class to make it public
class GeoVolumePositioner : public GeoNodePositioning {
 public:
  explicit GeoVolumePositioner(const GeoVPhysVol* physVol)
      : GeoNodePositioning{physVol} {}
};
GeoTrf::Transform3D volumePosInSpace(const PVConstLink& physVol,
                                     GeoVAlignmentStore* store) {
  if (auto fullPhys = dynamic_pointer_cast<const GeoVFullPhysVol>(physVol);
      fullPhys != nullptr) {
    return fullPhys->getAbsoluteTransform(store);
  }
  GeoVolumePositioner positioner{physVol};
  return positioner.getAbsoluteTransform(store);
}

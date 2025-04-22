/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOVFULLPHYSVOL_H
#define GEOMODELKERNEL_GEOVFULLPHYSVOL_H

#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoNodePositioning.h"


class GeoVFullPhysVol : public GeoVPhysVol, public GeoNodePositioning {
 public:
    GeoVFullPhysVol(const GeoLogVol* logVol);

    /// Returns the absolute name of this node.
    const std::string& getAbsoluteName() const;

    /// Returns the identification bits.
    unsigned int getId() const;

 protected:
  virtual ~GeoVFullPhysVol() = default;

 private:
  /// The absolute name of this volume.
  mutable std::string m_absName{};

  /// An identifier. This is locally cached in a full physical volume.
  mutable std::unique_ptr<std::optional<int>> m_id{nullptr};

 
};

#endif

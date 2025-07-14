/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GEOMODELHELPERS_VOLUMEUTILS_H
#define GEOMODELHELPERS_VOLUMEUTILS_H

#include "GeoModelHelpers/cloneVolume.h"
#include "GeoModelHelpers/defineWorld.h"
#include "GeoModelHelpers/getChildNodesWithTrf.h"




/** @brief Returns the volume's absolute position w.r.t to the root
 *         node. The method only works if the volume is uniquely used
 *         in the GeoModel tree, i.e. the GeoPhysVol is not inserted in
 *         multiple places
 *   @param physVol: Reference to the physical volume which position shall be evaluated
 *   @param store: Optional pointer to the alignment store, if alignment correction shall be 
 *                 considered. */
GeoTrf::Transform3D volumePosInSpace(const PVConstLink& physVol,
                                     GeoVAlignmentStore* store = nullptr);

#endif
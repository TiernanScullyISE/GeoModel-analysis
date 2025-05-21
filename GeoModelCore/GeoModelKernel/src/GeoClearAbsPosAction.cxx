/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoClearAbsPosAction.h"


void GeoClearAbsPosAction::handleFullPhysVol (const GeoFullPhysVol *vol) {
  vol->clearPositionInfo();
}

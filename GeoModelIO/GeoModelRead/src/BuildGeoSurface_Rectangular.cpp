/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BuildGeoSurface_Rectangular.h"

#include "GeoModelKernel/GeoRectSurface.h"
#include "GeoModelHelpers/variantHelpers.h"

namespace GeoModelIO {

void BuildGeoSurface_Rectangular::buildSurface(const DBRowEntry row) {
    const unsigned surfID = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "SurfaceRect_ID");
    //const double surfArea = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "SurfaceRect_Area");
    // shape parameters
    const double XHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "SurfaceRect_XHalfLength");
    const double YHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "SurfaceRect_YHalfLength");

    auto surface = make_intrusive<GeoRectSurface>(XHalfLength, YHalfLength);
    storeNewSurface(surfID, std::move(surface));
}
}
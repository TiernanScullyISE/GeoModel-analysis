/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "BuildGeoSurface_Diamond.h"

#include "GeoModelKernel/GeoDiamondSurface.h"
#include "GeoModelHelpers/variantHelpers.h"

namespace GeoModelIO {

void BuildGeoSurface_Diamond::buildSurface(const DBRowEntry row) {
    const unsigned surfID = GeoModelHelpers::variantHelper::getFromVariant_Int(row[1], "SurfaceDiamond_ID");
    //const double surfArea = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "SurfaceDiamond_Area");
    // shape parameters
    const double X_bottom_half = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "SurfaceDiamond_XBottomHalf");
    const double X_mid_half = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "SurfaceDiamond_XMidHalf");
    const double X_top_half = GeoModelHelpers::variantHelper::getFromVariant_Double(row[4], "SurfaceDiamond_XTopHalf");
    const double Y_bottom_half = GeoModelHelpers::variantHelper::getFromVariant_Double(row[5], "SurfaceDiamond_YBottomHalf");
    const double Y_top_half = GeoModelHelpers::variantHelper::getFromVariant_Double(row[6], "SurfaceDiamond_YTopHalf");

    auto surface = make_intrusive<GeoDiamondSurface>(X_bottom_half, X_mid_half, X_top_half, Y_bottom_half, Y_top_half);
    storeNewSurface(surfID, std::move(surface));
}
}
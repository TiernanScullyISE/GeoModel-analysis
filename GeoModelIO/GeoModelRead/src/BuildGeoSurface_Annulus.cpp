/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BuildGeoSurface_Annulus.h"

#include "GeoModelKernel/GeoAnnulusSurface.h"
#include "GeoModelHelpers/variantHelpers.h"

namespace GeoModelIO {

void BuildGeoSurface_Annulus::buildSurface(const DBRowEntry row) {
    const unsigned surfID = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "SurfaceAnnulus_ID");
    // shape area
    const double surfArea = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "SurfaceAnnulus_Area");
    // shape parameters
    const double Ox = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "SurfaceAnnulus_Ox");
    const double Oy = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "SurfaceAnnulus_Oy");
    const double radius_in = GeoModelHelpers::variantHelper::getFromVariant_Double(row[4], "SurfaceAnnulus_RadiusIn");
    const double radius_out = GeoModelHelpers::variantHelper::getFromVariant_Double(row[5], "SurfaceAnnulus_RadiusOut");
    const double phi = GeoModelHelpers::variantHelper::getFromVariant_Double(row[6], "SurfaceAnnulus_Phi");

    auto surface = make_intrusive<GeoAnnulusSurface>(Ox, Oy, radius_in, radius_out, phi);
    storeNewSurface(surfID, std::move(surface));
}
}
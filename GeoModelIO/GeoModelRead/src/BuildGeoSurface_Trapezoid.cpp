/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BuildGeoSurface_Trapezoid.h"

#include "GeoModelKernel/GeoTrapezoidSurface.h"
#include "GeoModelHelpers/variantHelpers.h"
namespace GeoModelIO {

void BuildGeoSurface_Trapezoid::buildSurface(const DBRowEntry row) {
    const unsigned surfID = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "SurfaceTrapezoid_ID");
    // shape area
    // const double surfArea = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "SurfaceTrapezoid_Area");
    // shape parameters
    const double XHalfLengthMin = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "SurfaceTrapezoid_XHalfLengthMin");
    const double XHalfLengthMax = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "SurfaceTrapezoid_XHalfLengthMax");    
    const double YHalfLength = GeoModelHelpers::variantHelper::getFromVariant_Double(row[4], "SurfaceTrapezoid_YHalfLength");

    auto surface =  make_intrusive<GeoTrapezoidSurface>(XHalfLengthMin, XHalfLengthMax, YHalfLength);
    storeNewSurface(surfID, std::move(surface));
}
}
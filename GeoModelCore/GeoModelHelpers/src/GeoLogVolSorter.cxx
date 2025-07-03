/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelHelpers/GeoLogVolSorter.h"
#include "GeoModelKernel/throwExcept.h"
#include "GeoModelHelpers/GeoShapeSorter.h"
#include "GeoModelHelpers/GeoMaterialSorter.h"

bool GeoLogVolSorter::operator()(const GeoLogVol* a, const GeoLogVol* b) const{
    return compare(a, b) < 0;
}
int GeoLogVolSorter::compare(const GeoLogVol*a, const GeoLogVol* b) const{
    if (!a || !b) {
        THROW_EXCEPTION("Nullptr given to the comparator");
    }
    static const GeoMaterialSorter matSorter{};
    if (const int matComp = matSorter.compare(a->getMaterial(), b->getMaterial())){
        return matComp;
    }
    static const GeoShapeSorter shapeSorter{};
    return shapeSorter.compare(a->getShape(), b->getShape());
}

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GeoModelHelpers/GeoMaterialSorter.h"

namespace {
    // Tolerance in density to treat both material equally
    constexpr double equivTol = 1.e-6;
}


bool GeoMaterialSorter::operator()(const GeoMaterial* a, const GeoMaterial* b) const{
    return compare(a,b) < 0;
}
bool GeoMaterialSorter::operator()(const GeoElement* a, const GeoElement* b) const{
    return compare(a,b) < 0;
}

int GeoMaterialSorter::compare(const GeoMaterial* a, const GeoMaterial* b) const {
    if (a == b) {
        return 0;
    }
    if (a->getNumElements() != b->getNumElements()) {
        return a->getNumElements() < b->getNumElements() ? -1 : 1;
    }
    const double densityCmp = a->getDensity() - b->getDensity();
    if (std::abs(densityCmp) > equivTol) {
        return densityCmp < 0. ? -1 : 1; 
    }

    for (unsigned e = 0; e < a->getNumElements(); ++e) {
        const double fracComp = a->getFraction(e) - b->getFraction(e);
        if (std::abs(fracComp) > equivTol) {
            return fracComp < 0.? -1: 1;
        }
        const int eleCmp = compare(a->getElement(e), b->getElement(e));
        if (eleCmp) {
            return eleCmp;
        }
    }
    return 0;
}
int GeoMaterialSorter::compare(const GeoElement* a, const GeoElement* b) const {
    if (a == b) {
        return 0;
    }
    if (a->getZ() != b->getZ()) {
        return a->getZ() < b->getZ() ? -1 : 1;
    }
    if (a->getA() != b->getA()) {
        return a->getA() < b->getA() ? -1 : 1;
    } 
    return 0;
}
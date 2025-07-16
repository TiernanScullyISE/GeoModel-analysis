/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GEOMODELREAD_BUILDGEOVSURFACE_Rectangular_H
#define GEOMODELREAD_BUILDGEOVSURFACE_Rectangular_H

#include "GeoModelRead/BuildGeoVSurface.h"

namespace GeoModelIO {

class BuildGeoSurface_Rectangular : public BuildGeoVSurface {
    public:
        using BuildGeoVSurface::BuildGeoVSurface;
    private:
        void buildSurface(const DBRowEntry row) override final;
};
}


#endif
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelHelpers/defineWorld.h"
#include "GeoModelHelpers/cloneVolume.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/Units.h"

GeoIntrusivePtr<GeoPhysVol> createGeoWorld(const double worldBoxX, 
                                           const double worldBoxY,
                                           const double worldBoxZ) {
    

    constexpr double gr =   GeoModelKernelUnits::gram;
    constexpr double mole = GeoModelKernelUnits::mole;
    constexpr double cm3 =  GeoModelKernelUnits::cm3;

    // Define the chemical elements
    GeoIntrusivePtr<GeoElement>  Nitrogen{make_intrusive<GeoElement>("Nitrogen" ,"N"  ,  7.0 ,  14.0031 *gr/mole)};
    GeoIntrusivePtr<GeoElement>  Oxygen{make_intrusive<GeoElement>("Oxygen"   ,"O"  ,  8.0 ,  15.9949 *gr/mole)};
    GeoIntrusivePtr<GeoElement>  Argon{make_intrusive<GeoElement>("Argon"    ,"Ar" , 18.0 ,  39.9624  *gr/mole)};
    GeoIntrusivePtr<GeoElement>  Hydrogen{make_intrusive<GeoElement>("Hydrogen" ,"H"  ,  1.0 ,  1.00782503081372 *gr/mole)};

    constexpr double densityOfAir=0.001290 *gr/cm3;
    GeoIntrusivePtr<GeoMaterial> air{make_intrusive<GeoMaterial>("GeoModelAir", densityOfAir)};
    air->add(Nitrogen  , 0.7494);
    air->add(Oxygen, 0.2369);
    air->add(Argon, 0.0129);
    air->add(Hydrogen, 0.0008);
    air->lock();


    GeoIntrusivePtr<GeoBox> worldBox{make_intrusive<GeoBox>(worldBoxX, worldBoxY, worldBoxZ)};
    GeoIntrusivePtr<GeoLogVol> worldLog{make_intrusive<GeoLogVol>("WorldLog", worldBox, air)};
    GeoIntrusivePtr<GeoPhysVol> world{make_intrusive<GeoPhysVol>(worldLog)};

    return world;    
}

GeoIntrusivePtr<GeoPhysVol> resizeGeoWorld(GeoIntrusivePtr<GeoPhysVol> world,
                                           bool cloneVolumes) {
    if (!world) return world;

    //resize the world volume to the real needed volume
    
    // get number of children volumes
    unsigned int nChild=world->getNChildVols();
    
    //Dimensions of the bounding boxes
    double xmin=0.,ymin=0.,zmin=0.,xmax=0.,ymax=0.,zmax=0.;
    double xworld=0.,yworld=0.,zworld=0.;
        
    // loop over all children volumes
    std::array<GeoTrf::Vector3D, 8> vv{};
    for (auto i = 0; i < nChild; ++i) {
      GeoTrf::Transform3D trans = world->getXToChildVol(i);
      PVConstLink childVol = world->getChildVol(i);
        childVol->getLogVol()->getShape()->extent(xmin, ymin, zmin, xmax, ymax, zmax);
        vv[0] = trans * GeoTrf::Vector3D(xmin, ymin, zmin);
        vv[1] = trans * GeoTrf::Vector3D(xmax, ymin, zmin);
        vv[2] = trans * GeoTrf::Vector3D(xmin, ymax, zmin);
        vv[3] = trans * GeoTrf::Vector3D(xmax, ymax, zmin);
        vv[4] = trans * GeoTrf::Vector3D(xmin, ymin, zmax);
        vv[5] = trans * GeoTrf::Vector3D(xmax, ymin, zmax);
        vv[6] = trans * GeoTrf::Vector3D(xmin, ymax, zmax);
        vv[7] = trans * GeoTrf::Vector3D(xmax, ymax, zmax);
        for (auto k = 0; k < 8; ++k)
        {
          xworld = std::max(xworld, std::abs(vv[k].x()));
          yworld = std::max(yworld, std::abs(vv[k].y()));
          zworld = std::max(zworld, std::abs(vv[k].z()));
        }
    }
    GeoIntrusivePtr<GeoPhysVol> resizedWorld{createGeoWorld(xworld, yworld, zworld)};
    for (unsigned int ch = 0; ch < world->getNChildNodes(); ++ch) {
        const GeoGraphNode * node = *(world->getChildNode(ch));
        if (cloneVolumes && (typeid(*node) == typeid(GeoFullPhysVol) ||
                            typeid(*node) == typeid(GeoPhysVol))) {
            const GeoVPhysVol* subVol{dynamic_cast<const GeoVPhysVol*>(node)};
            GeoVPhysVol* nonConstVol{const_cast<GeoVPhysVol*>(subVol)};
            resizedWorld->add(cloneVolume(nonConstVol));
        } else {
            resizedWorld->add(const_cast<GeoGraphNode*>(node));
        }
    }  
 
    return resizedWorld;
}

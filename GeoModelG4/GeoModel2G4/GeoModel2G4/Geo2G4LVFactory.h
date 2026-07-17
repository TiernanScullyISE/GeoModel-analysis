/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO2G4_Geo2G4LVFactory_h
#define GEO2G4_Geo2G4LVFactory_h

#include "GeoModel2G4/Geo2G4SolidFactory.h"
#include "GeoMaterial2G4/Geo2G4MaterialFactory.h"

#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/Units.h"

#include <map>

class G4LogicalVolume;

class Geo2G4LVFactory 
{
  public:
    Geo2G4LVFactory() = default;
    ~Geo2G4LVFactory() = default;
    G4LogicalVolume* Build(const PVConstLink thePhys, bool& descend);

  private:
    using LeafVMap = std::map<const GeoLogVol*, G4LogicalVolume*>;
    using BranchVMap = std::map<const GeoVPhysVol*, G4LogicalVolume*>;
    using FullPVMap = std::map<const GeoFullPhysVol*, G4LogicalVolume*>;

    Geo2G4SolidFactory m_theSolidFactory{};
    Geo2G4MaterialFactory m_theMaterialFactory{};
    LeafVMap m_sharedLeafLV{};
    BranchVMap m_sharedBranchLV{};
    FullPVMap m_clonedLV{};
};

#endif

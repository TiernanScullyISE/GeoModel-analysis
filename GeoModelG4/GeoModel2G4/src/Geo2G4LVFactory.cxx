/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModel2G4/Geo2G4LVFactory.h"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"

G4LogicalVolume* Geo2G4LVFactory::Build(const PVConstLink thePhys,
                                        bool& descend)
{
  const GeoFullPhysVol* fullPV = dynamic_cast<const GeoFullPhysVol*>(thePhys.get());
  const GeoFullPhysVol* clonePV = nullptr;

  const GeoLogVol* theLog = thePhys->getLogVol();
  G4LogicalVolume *theG4Log = nullptr;
  G4Material* theG4Mat = nullptr;
  G4VSolid* theG4Solid = nullptr;

  descend = true;
  bool putLeaf = false;
  bool putBranch = false;
  bool putFullPV = false;

  // Check if it is a leaf node of Geo tree
  if(thePhys->getNChildVols() == 0)
    {
      descend=false;
      auto lv = m_sharedLeafLV.find(theLog);
      if(lv != m_sharedLeafLV.end()) {
        return lv->second;
      }
      else { // here supposed to be ---> else if(theLog->refCount() > 1)
        putLeaf = true;
      }
    }
  // Work with the Full Physical Volumes
  else if(fullPV)
    {
      clonePV = fullPV->cloneOrigin();
      auto lv = m_clonedLV.find(clonePV);
      if (lv == m_clonedLV.end()) {
        if(clonePV) putFullPV = true;
      }
      else {
        descend = false;
        return lv->second;
      }
    }
  else
    {
      auto lv = m_sharedBranchLV.find(thePhys.get());
      if(lv == m_sharedBranchLV.end()) {
        putBranch = true;
      }
      else {
        descend = false;
        return lv->second;
      }
    }

  // Actually build the G4Log
  theG4Mat=m_theMaterialFactory.Build(theLog->getMaterial());
  theG4Solid = m_theSolidFactory.Build(theLog->getShape(),theLog->getName());
  theG4Log = new G4LogicalVolume(theG4Solid,
                                 theG4Mat,
                                 theLog->getName(),
                                 0,0,0);

  if(putLeaf) m_sharedLeafLV.insert({theLog,theG4Log});
  if(putBranch) m_sharedBranchLV.insert({thePhys.get(),theG4Log});
  if(putFullPV) m_clonedLV.insert({clonePV,theG4Log});

  return theG4Log;
}

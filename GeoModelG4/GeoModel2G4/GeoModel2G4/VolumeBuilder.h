/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO2G4_VolumeBuilder_H
#define GEO2G4_VolumeBuilder_H

#include "GeoModel2G4/Geo2G4LVFactory.h"
#include "GeoModelKernel/GeoVPhysVol.h"

#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

#include <string>
#include <iostream>
#include <map>

class Geo2G4AssemblyVolume;
class GeoMaterial;
class GeoElement;

class VolumeBuilder
{
 public:
  VolumeBuilder(const std::string& k): m_key(k) {}
  VolumeBuilder() = delete;
  ~VolumeBuilder() = default;

  std::string GetKey() const {return m_key;}

  // flag controlling Parameterization to Parameterization translation
  void SetParam(bool flag){m_paramOn = flag;}
  bool GetParam(){return m_paramOn;}

  G4LogicalVolume* Build(PVConstLink pv);

 private:
  std::string m_key{};
  bool m_paramOn{false};

  mutable bool               m_getMatEther{true};
  mutable const GeoMaterial* m_matEther{nullptr};
  mutable const GeoMaterial* m_matHypUr{nullptr};

  Geo2G4LVFactory m_lvFactory{};

  Geo2G4AssemblyVolume* BuildAssembly(PVConstLink pv);
  /// Prints info when some PhysVol contains both types (PV and ST) of daughters
  void PrintSTInfo(std::string volume) const;
  ///
  void getMatEther() const;
};
#endif

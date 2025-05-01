/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GeoModelWrite/WriteGeoModel.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include <gtest/gtest.h>

#define GTEST_COUT std::cerr << "[ MESSAGE  ] "
namespace GeoModelIO{

//Simple testing 'friend' class to expose private methods of WriteGeoModel
//for testing
class WriteGeoModelTest{
  public:
    WriteGeoModelTest(WriteGeoModel & wgm):m_pWgm(&wgm){
      //
    }
    std::string getAddressStringFromPointer(auto p){
      return m_pWgm->getAddressStringFromPointer(p);
    }
  private:
    WriteGeoModel * m_pWgm;
};

}
using namespace GeoModelIO;

TEST(WriteGeoModel, CanBeDefaultConstructed) {
  EXPECT_NO_THROW(WriteGeoModel  wgm);
}

TEST(WriteGeoModel, CanGetAddressStringFromPointer){
  WriteGeoModel wgm;
  WriteGeoModelTest  wgmt(wgm);
  auto pElement = make_intrusive<GeoElement>("Rhodium","Rh", 45.,102.9);
  std::string addrStr;
  EXPECT_NO_THROW(addrStr = wgmt.getAddressStringFromPointer(pElement.get()));
  GTEST_COUT<<addrStr<<std::endl;
  //
  auto pMaterial = make_intrusive<GeoMaterial>("MyMaterial",1000.);
  EXPECT_NO_THROW(addrStr = wgmt.getAddressStringFromPointer(pMaterial.get()));
  GTEST_COUT<<addrStr<<std::endl;
  //
  //GeoShape
  auto pBox = make_intrusive<GeoBox>(1., 1., 1.);
  EXPECT_NO_THROW(addrStr = wgmt.getAddressStringFromPointer(pBox.get()));
  GTEST_COUT<<addrStr<<std::endl;
  //
  //GeoLogVol
  auto pLogVol = make_intrusive<GeoLogVol>("Null",nullptr, nullptr);
  EXPECT_NO_THROW(addrStr = wgmt.getAddressStringFromPointer(pLogVol.get()));
  GTEST_COUT<<addrStr<<std::endl;
  //
  //GeoPhysVol
  auto pPhysVol = make_intrusive<GeoPhysVol>(nullptr);
  EXPECT_NO_THROW(addrStr = wgmt.getAddressStringFromPointer(pPhysVol.get()));
  GTEST_COUT<<addrStr<<std::endl;
  //
  //GeoVPhysVol
  auto pVPhysVol = make_intrusive<GeoVPhysVol>(nullptr);
  EXPECT_NO_THROW(addrStr = wgmt.getAddressStringFromPointer(pVPhysVol.get()));
  GTEST_COUT<<addrStr<<std::endl;
  //
  //GeoVSurface
  auto pVSurface = make_intrusive<GeoVSurface>(nullptr);
  EXPECT_NO_THROW(addrStr = wgmt.getAddressStringFromPointer(pVSurface.get()));
  GTEST_COUT<<addrStr<<std::endl;
  //
  //GeoVSurfaceShape
  auto pTrapezSurf = make_intrusive<GeoTrapezoidSurface>(1., 1.,1.);
  EXPECT_NO_THROW(addrStr = wgmt.getAddressStringFromPointer(pTrapezSurf.get()));
  GTEST_COUT<<addrStr<<std::endl;
  //
  //GeoSerialIdentifier
  auto pGeoSerialId = make_intrusive<GeoSerialIdentifier>(1);
  EXPECT_NO_THROW(addrStr = wgmt.getAddressStringFromPointer(pGeoSerialId.get()));
  GTEST_COUT<<addrStr<<std::endl;
  //etc etc; there are no 'special cases' but there is a limited list of things 
  //allowed to be converted, so the following would not compile:
  //
  //int * pI{};
  //EXPECT_NO_THROW(addrStr = wgmt.getAddressStringFromPointer(pI));
}






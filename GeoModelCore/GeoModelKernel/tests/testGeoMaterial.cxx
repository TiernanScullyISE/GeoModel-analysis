/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include <gtest/gtest.h>
#include <stdexcept>

#define GTEST_COUT std::cerr << "[ MESSAGE  ] "

using namespace GeoModelKernelUnits;

class SampleMaterial{
  private:
    const std::string m_name{"Rhodium"};
    const double m_dense{13.0*gram/centimeter3};
    GeoIntrusivePtr<GeoMaterial>  m_ptr{};
  public:
    GeoMaterial * ptr() const {return m_ptr.get();}
    SampleMaterial():m_ptr(make_intrusive<GeoMaterial>(m_name, m_dense)){
      //
    };
};


TEST(GeoMaterial, CanBeConstructedOnHeap) {
  GeoIntrusivePtr<GeoMaterial>  pG{};
  const std::string name{"Shaun"};
  const double dense{13.0*gram/centimeter3};
  EXPECT_NO_THROW(pG = make_intrusive<GeoMaterial>(name, dense));
}

TEST(GeoMaterial, CannotLockAMaterialWithNoElements) {
  SampleMaterial s;
  auto * pMat = s.ptr();
  EXPECT_THROW(pMat->lock(), std::runtime_error);
}

TEST(GeoMaterial, CannotAccessUnlockedMaterialProperties) {
  SampleMaterial s;
  auto * pMat = s.ptr();
  EXPECT_THROW(pMat->getNumElements(), std::runtime_error);
  EXPECT_THROW(pMat->getDeDxConstant(), std::runtime_error);
  EXPECT_THROW(pMat->getDeDxI0(),std::runtime_error);
  EXPECT_THROW(pMat->getDeDxMin(),std::runtime_error);
  EXPECT_THROW(pMat->getRadLength(),std::runtime_error);
  EXPECT_THROW(pMat->getIntLength(),std::runtime_error);
  EXPECT_THROW(pMat->getNumElements(),std::runtime_error);
  EXPECT_THROW(pMat->getElement(0),std::runtime_error);
  EXPECT_THROW(pMat->getFraction(0),std::runtime_error);
}

TEST(GeoMaterial, CanAccessTrivialUnlockedMaterialProperties) {
  SampleMaterial s;
  const double expectedDensity{13.0*gram/centimeter3};
  auto * pMat = s.ptr();
  EXPECT_EQ(pMat->getName(), "Rhodium");
  EXPECT_EQ(pMat->getDensity(), expectedDensity); 
  int id{};
  //id seems to vary run to run
  EXPECT_NO_THROW(id = pMat->getID());
  GTEST_COUT <<"id is "<< id << std::endl;
}







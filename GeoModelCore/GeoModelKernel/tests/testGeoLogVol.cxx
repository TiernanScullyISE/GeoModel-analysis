/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelKernel/GeoTube.h"
#include <gtest/gtest.h>

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


TEST(GeoLogVol, CanBeConstructedOnHeap) {
  SampleMaterial s;
  auto pTube = make_intrusive<GeoTube>(0.5, 0.7, 1.0);
  auto pMaterial = s.ptr();
  EXPECT_NO_THROW(auto pLogVol =  make_intrusive<GeoLogVol>("myTube", pTube, pMaterial));
}


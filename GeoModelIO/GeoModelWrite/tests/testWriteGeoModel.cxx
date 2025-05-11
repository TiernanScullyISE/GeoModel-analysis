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



}
using namespace GeoModelIO;

TEST(WriteGeoModel, CanBeDefaultConstructed) {
  EXPECT_NO_THROW(WriteGeoModel  wgm);
}







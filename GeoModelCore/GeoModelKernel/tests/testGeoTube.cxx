/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GeoModelKernel/GeoTube.h"
#include <gtest/gtest.h>


TEST(GeoTube, CanBeConstructedOnHeap) {
  GeoTube * pG{};
  EXPECT_NO_THROW(pG = new GeoTube(0.5, 0.7, 1.0));
  pG->ref();
  pG->unref(); //should be destroyed here
}


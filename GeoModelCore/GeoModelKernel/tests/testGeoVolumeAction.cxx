/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GeoModelKernel/GeoVolumeAction.h"
#include <gtest/gtest.h>
#include <type_traits>


TEST(GeoVolumeAction, CanBeDefaultConstructed) {
  EXPECT_NO_THROW([[maybe_unused]] GeoVolumeAction dummy);
}

TEST(GeoVolumeAction, ParametrisedCtorIsOk) {
  EXPECT_NO_THROW([[maybe_unused]] GeoVolumeAction dummy{GeoVolumeAction::Type::BOTTOM_UP});
}

TEST(GeoVolumeAction, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoVolumeAction>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoVolumeAction>);
}

TEST(GeoVolumeAction, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoVolumeAction>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoVolumeAction>);
}


TEST(GeoVolumeAction, DefaultPropertiesAreOk) {
  const GeoVolumeAction g;
  EXPECT_EQ(g.shouldTerminate(), false);
  EXPECT_NE(g.getState(), nullptr); 
  EXPECT_EQ(g.getType(), GeoVolumeAction::Type::TOP_DOWN);
  //non-const
  GeoVolumeAction h;
  EXPECT_EQ(h.getType(), GeoVolumeAction::Type::TOP_DOWN);
  EXPECT_NE(h.getState(), nullptr);
  EXPECT_EQ(h.shouldTerminate(), false);
}

TEST(GeoVolumeAction, terminateSetsTerminateTrue) {
  GeoVolumeAction g;//must be non-const
  EXPECT_NO_THROW(g.terminate());
  EXPECT_EQ(g.shouldTerminate(), true);
}







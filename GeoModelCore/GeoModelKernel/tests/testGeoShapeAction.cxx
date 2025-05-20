#include "GeoModelKernel/GeoShapeAction.h"
#include <gtest/gtest.h>
#include <type_traits>



TEST(GeoShapeAction, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoShapeAction>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoShapeAction>);
}

TEST(GeoShapeAction, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoShapeAction>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoShapeAction>);
}

TEST(GeoShapeAction, DefaultConstructedHasExpectedProperties) {
  GeoShapeAction g;
  EXPECT_FALSE(g.shouldTerminate());
  EXPECT_FALSE(g.getDepthLimit());//optional; no value by default
}
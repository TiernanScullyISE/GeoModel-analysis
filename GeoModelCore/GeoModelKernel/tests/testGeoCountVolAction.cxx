#include "GeoModelKernel/GeoCountVolAction.h"
#include <gtest/gtest.h>
#include <type_traits>




TEST(GeoCountVolAction, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoCountVolAction>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoCountVolAction>);
}

TEST(GeoCountVolAction, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoCountVolAction>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoCountVolAction>);
}
TEST(GeoCountVolAction, DefaultConstructedHasExpectedProperties) {
  GeoCountVolAction g;
  EXPECT_EQ(g.getCount(), 0);
}

#include "GeoModelKernel/GeoCountVolAndSTAction.h"
#include <gtest/gtest.h>
#include <type_traits>




TEST(GeoCountVolAndSTAction, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoCountVolAndSTAction>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoCountVolAndSTAction>);
}

TEST(GeoCountVolAndSTAction, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoCountVolAndSTAction>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoCountVolAndSTAction>);
}

#include "GeoModelKernel/GeoCountSurfaceAction.h"
#include <gtest/gtest.h>
#include <type_traits>




TEST(GeoCountSurfaceAction, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoCountSurfaceAction>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoCountSurfaceAction>);
}

TEST(GeoCountSurfaceAction, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoCountSurfaceAction>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoCountSurfaceAction>);
}

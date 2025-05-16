#include "GeoModelKernel/GeoAccessSurfaceAction.h"
#include <gtest/gtest.h>
#include <type_traits>




TEST(GeoAccessSurfaceAction, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoAccessSurfaceAction>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoAccessSurfaceAction>);
}

TEST(GeoAccessSurfaceAction, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoAccessSurfaceAction>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoAccessSurfaceAction>);
}

#include "GeoModelKernel/GeoSurfaceCursor.h"
#include <gtest/gtest.h>
#include <type_traits>




TEST(GeoSurfaceCursor, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoSurfaceCursor>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoSurfaceCursor>);
}

TEST(GeoSurfaceCursor, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoSurfaceCursor>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoSurfaceCursor>);
}

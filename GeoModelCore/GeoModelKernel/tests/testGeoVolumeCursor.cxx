#include "GeoModelKernel/GeoVolumeCursor.h"
#include <gtest/gtest.h>
#include <type_traits>




TEST(GeoVolumeCursor, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoVolumeCursor>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoVolumeCursor>);
}

TEST(GeoVolumeCursor, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoVolumeCursor>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoVolumeCursor>);
}

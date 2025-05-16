#include "GeoModelKernel/GeoVG4ExtensionSolid.h"
#include <gtest/gtest.h>
#include <type_traits>




TEST(GeoVG4ExtensionSolid, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoVG4ExtensionSolid>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoVG4ExtensionSolid>);
}

TEST(GeoVG4ExtensionSolid, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoVG4ExtensionSolid>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoVG4ExtensionSolid>);
}

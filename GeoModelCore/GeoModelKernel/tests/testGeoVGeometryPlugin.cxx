#include "GeoModelKernel/GeoVGeometryPlugin.h"
#include <gtest/gtest.h>
#include <type_traits>




TEST(GeoVGeometryPlugin, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoVGeometryPlugin>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoVGeometryPlugin>);
}

TEST(GeoVGeometryPlugin, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoVGeometryPlugin>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoVGeometryPlugin>);
}

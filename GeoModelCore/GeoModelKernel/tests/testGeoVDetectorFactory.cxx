#include "GeoModelKernel/GeoVDetectorFactory.h"
#include <gtest/gtest.h>
#include <type_traits>




TEST(GeoVDetectorFactory, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoVDetectorFactory>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoVDetectorFactory>);
}

TEST(GeoVDetectorFactory, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoVDetectorFactory>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoVDetectorFactory>);
}

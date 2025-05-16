#include "GeoModelKernel/GeoClearAbsPosAction.h"
#include <gtest/gtest.h>
#include <type_traits>




TEST(GeoClearAbsPosAction, CannotBeMoveOrCopyConstructed) {
  EXPECT_FALSE( std::is_copy_constructible_v<GeoClearAbsPosAction>);
  EXPECT_FALSE( std::is_move_constructible_v<GeoClearAbsPosAction>);
}

TEST(GeoClearAbsPosAction, CannotBeMoveOrCopyAssigned) {
  EXPECT_FALSE(std::is_copy_assignable_v<GeoClearAbsPosAction>);
  EXPECT_FALSE(std::is_move_assignable_v<GeoClearAbsPosAction>);
}

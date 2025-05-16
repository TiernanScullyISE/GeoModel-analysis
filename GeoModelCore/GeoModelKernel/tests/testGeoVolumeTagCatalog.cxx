#include "GeoModelKernel/GeoVolumeTagCatalog.h"
#include <gtest/gtest.h>
#include <type_traits>




TEST(GeoVolumeTagCatalog, CanBeMoveAndCopyConstructed) {
  EXPECT_TRUE( std::is_copy_constructible_v<GeoVolumeTagCatalog>);
  EXPECT_TRUE( std::is_move_constructible_v<GeoVolumeTagCatalog>);
}

TEST(GeoVolumeTagCatalog, CanBeMoveAndCopyAssigned) {
  EXPECT_TRUE(std::is_copy_assignable_v<GeoVolumeTagCatalog>);
  EXPECT_TRUE(std::is_move_assignable_v<GeoVolumeTagCatalog>);
}

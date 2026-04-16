/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GeoModelKernel/CellBinning.h"

#include <exception>
#include <gtest/gtest.h>


#define GTEST_COUT std::cerr << "[ MESSAGE  ] "

class CellBinningFixture : public ::testing::Test {
protected:
    CellBinning cb{1., 10., 10, 1};
};
class InvertedCellBinning : public ::testing::Test {
protected:
    CellBinning cb{10., 1., 10, 1};
};

TEST(CellBinning, CanBeConstructed) {
  EXPECT_NO_THROW(CellBinning cb(1.,10.,10,1.));
}


TEST(CellBinning, CanBeMoveAndCopyConstructed) {
  EXPECT_TRUE( std::is_copy_constructible_v<CellBinning>);
  EXPECT_TRUE( std::is_move_constructible_v<CellBinning>);
}

TEST(CellBinning, CanBeMoveAndCopyAssigned) {
  EXPECT_TRUE(std::is_copy_assignable_v<CellBinning>);
  EXPECT_TRUE(std::is_move_assignable_v<CellBinning>);
}

TEST(CellBinning, EndValCanBeSmallerThanStartVal) {
  EXPECT_NO_THROW(CellBinning cb(10., 1., 10, 1));
}

TEST(CellBinning, NumBinsCannotBeZero) {
  EXPECT_THROW(CellBinning cb(10., 1., 0, 1), std::range_error);
}

TEST(CellBinning, NumBinsMustBePositive) {
  EXPECT_THROW(CellBinning cb(10., 1., -20, 1), std::range_error);
}

TEST_F(CellBinningFixture, lowerBinIsOk){
  EXPECT_EQ(cb.binLower(2), 1.9);
}

TEST_F(CellBinningFixture, upperBinIsOk){
  EXPECT_EQ(cb.binUpper(2), 2.8);
}

TEST_F(CellBinningFixture, centreBinIsOk){
  EXPECT_EQ(cb.binCenter(2), 2.35);
}

TEST_F(CellBinningFixture, startBinIsOk){
  EXPECT_EQ(cb.getStart(), 1.0);
}
TEST_F(CellBinningFixture, endBinIsOk){
  EXPECT_EQ(cb.getEnd(), 10.0);
}
TEST_F(CellBinningFixture, numDivisionsIsOk){
  EXPECT_EQ(cb.getNumDivisions(), 10);
}
TEST_F(CellBinningFixture, getDeltaIsOk){
  EXPECT_EQ(cb.getDelta(), 0.9);
}
TEST_F(CellBinningFixture, firstDivisionNumberIsOk){
  EXPECT_EQ(cb.getFirstDivisionNumber(), 1);
}
//inverted start and end vals; is it still ok?
TEST_F(InvertedCellBinning, lowerBinIsOk){
  EXPECT_EQ(cb.binLower(2), 9.1);
}
TEST_F(InvertedCellBinning, upperBinIsOk){
  EXPECT_EQ(cb.binUpper(2), 8.2);
}
TEST_F(InvertedCellBinning, centreBinIsOk){
  EXPECT_EQ(cb.binCenter(2), 8.65);
}
TEST_F(InvertedCellBinning, getDeltaIsOk){
  EXPECT_EQ(cb.getDelta(), -0.9);
}

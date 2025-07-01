
#include "GeoModelDBManager/GMDBManager.h"

#include "gtest/gtest.h"
#include <variant>
#include <string>

#define GTEST_COUT std::cerr << "[ MESSAGE  ] "

class GMDBManagerTest : public ::testing::Test {
protected:
    GMDBManager db{":memory:"};

    void SetUp() override {
        // Open in-memory DB and setup schema using internal interface or direct calls.
        db.execQuery("CREATE TABLE test (id INTEGER, val REAL, name TEXT);");
        db.execQuery("INSERT INTO test VALUES (1, 3.14, 'Alice');");
        db.execQuery("INSERT INTO test VALUES (2, 2.71, 'Bob');");

    }

    void TearDown() override {
        //nop
    }
};

TEST_F(GMDBManagerTest, GetTableRecords_VecVecDataReturnsTypedRows) {
    DBRowsList results = db.getTableRecords_VecVecData("test");

    ASSERT_EQ(results.size(), 2);

    const auto& row1 = results[0];
    ASSERT_EQ(row1.size(), 3);

    EXPECT_TRUE(std::holds_alternative<int>(row1[0]));
    EXPECT_EQ(std::get<int>(row1[0]), 1);

    EXPECT_TRUE(std::holds_alternative<double>(row1[1]));
    GTEST_COUT <<std::get<double>(row1[1])<<"\n";
    EXPECT_NEAR(std::get<double>(row1[1]), 3.14, 1e-6);

    EXPECT_TRUE(std::holds_alternative<std::string>(row1[2]));
    EXPECT_EQ(std::get<std::string>(row1[2]), "Alice");

    const auto& row2 = results[1];
    EXPECT_TRUE(std::holds_alternative<int>(row2[0]));
    EXPECT_EQ(std::get<int>(row2[0]), 2);

    EXPECT_TRUE(std::holds_alternative<double>(row2[1]));
    EXPECT_NEAR(std::get<double>(row2[1]), 2.71, 1e-6);

    EXPECT_TRUE(std::holds_alternative<std::string>(row2[2]));
    EXPECT_EQ(std::get<std::string>(row2[2]), "Bob");
}

TEST_F(GMDBManagerTest, GetTableRecords_VecVecDataThrowsOnNullColumn) {
    db.execQuery("CREATE TABLE test_null (id INTEGER, name TEXT);");
    db.execQuery("INSERT INTO test_null VALUES (1, NULL);");

    EXPECT_THROW(db.getTableRecords_VecVecData("test_null"), std::runtime_error);
}

TEST_F(GMDBManagerTest, GetTableRecords_VecVecDataThrowsOnBlobColumn) {
    db.execQuery("CREATE TABLE test_blob (data BLOB);");
    db.execQuery("INSERT INTO test_blob VALUES (x'DEADBEEF');");

    EXPECT_THROW(db.getTableRecords_VecVecData("test_blob"), std::runtime_error);
}
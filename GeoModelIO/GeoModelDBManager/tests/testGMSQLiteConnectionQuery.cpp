
#include "GeoModelDBManager/GMSQLiteConnection.h"
#include "GeoModelDBManager/definitions.h"
#include "sqlite3.h"
#include "gtest/gtest.h"
#include <variant>
#include <string>


#define GTEST_COUT std::cerr << "[ MESSAGE  ] "

class GMSQLiteConnectionTest : public ::testing::Test {
protected:
    GMSQLiteConnection db{":memory:"};

    void SetUp() override {        
        // Create a test table
        db.exec("CREATE TABLE test (id INTEGER, value REAL, name TEXT, nullable TEXT);");
        // Insert some test data, including NULL
        db.exec("INSERT INTO test VALUES (1, 3.14, 'foo', NULL);");
        db.exec("INSERT INTO test VALUES (2, 2.71, 'bar', 'not null');");
    }

    void TearDown() override {
        //nop
    }
};

TEST_F(GMSQLiteConnectionTest, ResultOfQueryReturnsCorrectTypes) {
    auto results = db.resultOfQuery("SELECT id, value, name, nullable FROM test ORDER BY id;");

    ASSERT_EQ(results.size(), 2);

    // Check first row
    const auto& row1 = results[0];
    ASSERT_EQ(row1.size(), 4);

    // id: int 1
    EXPECT_TRUE(std::holds_alternative<int>(row1[0]));
    EXPECT_EQ(std::get<int>(row1[0]), 1);

    // value: double 3.14
    EXPECT_TRUE(std::holds_alternative<double>(row1[1]));
    GTEST_COUT <<std::get<double>(row1[1])<<"\n";
    EXPECT_NEAR(std::get<double>(row1[1]), 3.14, 1e-6);

    // name: string "foo"
    EXPECT_TRUE(std::holds_alternative<std::string>(row1[2]));
    EXPECT_EQ(std::get<std::string>(row1[2]), "foo");

    // nullable: string "NULL"
    EXPECT_TRUE(std::holds_alternative<std::string>(row1[3]));
    EXPECT_EQ(std::get<std::string>(row1[3]), "NULL");

    // Check second row
    const auto& row2 = results[1];
    EXPECT_TRUE(std::holds_alternative<int>(row2[0]));
    EXPECT_EQ(std::get<int>(row2[0]), 2);

    EXPECT_TRUE(std::holds_alternative<double>(row2[1]));
    EXPECT_NEAR(std::get<double>(row2[1]), 2.71, 1e-6);

    EXPECT_TRUE(std::holds_alternative<std::string>(row2[2]));
    EXPECT_EQ(std::get<std::string>(row2[2]), "bar");

    EXPECT_TRUE(std::holds_alternative<std::string>(row2[3]));
    EXPECT_EQ(std::get<std::string>(row2[3]), "not null");
}

TEST_F(GMSQLiteConnectionTest, ThrowsOnBadSql) {
    EXPECT_THROW(db.resultOfQuery("SELECT * FROM non_existing_table;"), std::runtime_error);
}

TEST_F(GMSQLiteConnectionTest, ThrowsOnUnsupportedType) {
    // Create a table with BLOB column, which your implementation doesn't support
    db.exec("CREATE TABLE blob_test (data BLOB);");
    db.exec("INSERT INTO blob_test VALUES (x'1234');");

    EXPECT_THROW(db.resultOfQuery("SELECT data FROM blob_test;"), std::runtime_error);
}

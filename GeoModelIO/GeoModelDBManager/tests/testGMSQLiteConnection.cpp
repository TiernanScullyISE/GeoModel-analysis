#include "GeoModelDBManager/GMSQLiteConnection.h"
#include "sqlite3.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <stdexcept>
#include <iostream>


#define GTEST_COUT std::cerr << "[ MESSAGE  ] "


using namespace std::string_literals;
const std::string dbFileName{"sqliteDb.db"};

TEST(GMSQLiteConnectionTest, CanCreateNewDatabase) {
  const bool createNew{true};
  EXPECT_NO_THROW(GMSQLiteConnection db(dbFileName, createNew));
  EXPECT_TRUE(std::filesystem::exists(dbFileName));
}

TEST(GMSQLiteConnectionTest, CanOpenExistingDatabase) {
  //database created in previous test, not yet deleted
  EXPECT_NO_THROW(GMSQLiteConnection db(dbFileName, false));
}

TEST(GMSQLiteConnectionTest, RawPointerIsNotNull) {
  GMSQLiteConnection db(dbFileName);//opens existing db
  EXPECT_NE(db.ptr(), nullptr);
}

TEST(GMSQLiteConnectionTest, OpeningInexistentDbThrows) {
  //ensure file removal
  if (std::filesystem::exists(dbFileName))  std::filesystem::remove(dbFileName);
  EXPECT_THROW(GMSQLiteConnection db(dbFileName, false), std::runtime_error);
}

TEST(GMSQLiteConnectionTest, InsertData) {
  std::string testDB = "test_insert.sqlite";
  {
    GMSQLiteConnection conn(testDB, true);
    conn.exec("CREATE TABLE IF NOT EXISTS test_table (id INTEGER, name TEXT);");
    conn.exec("INSERT INTO test_table (id, name) VALUES (1, 'Alice');");
  }
  // Check if DB file was created
  ASSERT_TRUE(std::filesystem::exists(testDB));
}

TEST(GMSQLiteConnectionTest, ReadBackData) {
  std::string testDB = "test_insert.sqlite";
  sqlite3* db = nullptr;
  ASSERT_EQ(sqlite3_open(testDB.c_str(), &db), SQLITE_OK);
  sqlite3_stmt* stmt;
  const char* sql = "SELECT id, name FROM test_table WHERE id = 1;";
  ASSERT_EQ(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr), SQLITE_OK);
  ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
  int id = sqlite3_column_int(stmt, 0);
  const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
  EXPECT_EQ(id, 1);
  EXPECT_STREQ(name, "Alice");
  sqlite3_finalize(stmt);
  sqlite3_close(db);
  std::filesystem::remove(testDB);  // Cleanup
}

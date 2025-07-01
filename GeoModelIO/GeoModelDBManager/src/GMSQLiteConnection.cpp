#include "GeoModelDBManager/GMSQLiteConnection.h"
#include "GeoModelKernel/throwExcept.h"
#include "sqlite3.h"

#include <filesystem>
#include <stdexcept>

GMSQLiteConnection::GMSQLiteConnection(const std::string& filename, bool recreate)
    : m_filename(filename){
  if (recreate && std::filesystem::exists(filename)) {
    std::filesystem::remove(filename);
  }
  openDatabase(filename, recreate);//throws if not possible
}

GMSQLiteConnection::~GMSQLiteConnection() {
  closeDatabase();
}

void 
GMSQLiteConnection::openDatabase(const std::string& filename, bool recreate) {
  int flag = SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX;
  if (recreate) flag |= SQLITE_OPEN_CREATE;
  const int rc = sqlite3_open_v2(filename.c_str(),&m_db, flag, nullptr);
  if (rc != SQLITE_OK) {
    THROW_EXCEPTION( "Could not open SQLite database: "<<sqlite3_errmsg(m_db));
  }
}

void 
GMSQLiteConnection::closeDatabase() {
  if (m_db) {
    sqlite3_close(m_db);
    m_db = nullptr;
  }
}

void 
GMSQLiteConnection::exec(const std::string& sql) const {
  char* errMsg = nullptr;
  sqlite3_stmt* pStatement{};
  if (sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
    std::string msg = errMsg ? errMsg : "(unknown error)";
    sqlite3_free(errMsg);
    THROW_EXCEPTION(msg);
  }
}

DBRowsList 
GMSQLiteConnection::resultOfQuery(const std::string& sql) const {
  DBRowsList results;
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    THROW_EXCEPTION("GMSQLiteConnection::resultOfQuery: Failed to prepare statement: "<< sql);
  }
  int numCols = sqlite3_column_count(stmt);
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    DBRowEntry row;
    for (int i = 0; i < numCols; ++i) {
      switch (sqlite3_column_type(stmt, i)) {
        case SQLITE_INTEGER:
          row.emplace_back(static_cast<int>(sqlite3_column_int(stmt, i)));
          break;
        case SQLITE_FLOAT:
          row.emplace_back(sqlite3_column_double(stmt, i));
          break;
        case SQLITE_TEXT:
          row.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
          break;
        case SQLITE_NULL:
          row.emplace_back("NULL");  // or std::monostate if supported
          break;
        default:
          THROW_EXCEPTION("GMSQLiteConnection::resultOfQuery: Unsupported column type");
      }
    }
    results.push_back(std::move(row));
  }
  sqlite3_finalize(stmt);
  return results;
}

sqlite3* 
GMSQLiteConnection::ptr() const {
  return m_db;
}

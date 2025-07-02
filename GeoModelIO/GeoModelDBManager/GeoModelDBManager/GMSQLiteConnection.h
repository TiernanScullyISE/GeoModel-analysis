#ifndef GMSQLITE_CONNECTION_H
#define GMSQLITE_CONNECTION_H


#include "GeoModelDBManager/definitions.h"
#include <string>
class sqlite3;

class GMSQLiteConnection {
public:
   /**
    * @brief Constructor
    *
    * Constructor sets up connection with db and opens it
    * @param filename - absolute path to db file
    * @param recreate - if true, creates the file
    */
    GMSQLiteConnection(const std::string& filename, bool recreate = false);
    ~GMSQLiteConnection();

    //explicitly delete copy & move assignment and c'tors
    GMSQLiteConnection(const GMSQLiteConnection&) = delete;
    GMSQLiteConnection& operator=(const GMSQLiteConnection&) = delete;
    GMSQLiteConnection(const GMSQLiteConnection&&) = delete;
    GMSQLiteConnection& operator=(const GMSQLiteConnection&&) = delete;
    
    //Execute simple SQL statements which return no result
    void exec(const std::string& sql) const;
    
    //Execute simple SQL statements which return a result
    DBRowsList resultOfQuery(const std::string& sql) const;
    std::string errmsg() const;
    sqlite3* ptr() const; 

private:
    sqlite3* m_db{};
    std::string m_filename;

    void openDatabase(const std::string& filename, bool recreate);
    void closeDatabase();
};

#endif

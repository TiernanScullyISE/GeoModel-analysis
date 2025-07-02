/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * author: 2017, Riccardo Maria Bianchi @ CERN -
 * <riccardo.maria.bianchi@cern.ch> major updates:
 *  - Aug 2018, R.M.Bianchi
 *  - Jun 2020, R.M.Bianchi
 *  - Aug 2020, R.M.Bianchi - Added support to publish lists of FullPhysVol and
 *                            AlignableTransform nodes
 * - Jun 2022 - Riccardo Maria Bianchi, <riccardo.maria.bianchi@cern.ch>,
 *              Added support for "Verbose" output
 * - Jul 2023 - Riccardo Maria Bianchi, <riccardo.maria.bianchi@cern.ch>,
 *              Populate the caches in the constructor, so all print methods are
 *              available as soon as the input file is opened by the GMDBManager
 * - Apr 2024 - Riccardo Maria Bianchi, <riccardo.maria.bianchi@cern.ch>,
 *              New DB format to extend the storage of numbers as REAL instead of TEXT,
 *              New methods to handle vector<variants> as input of records,
 *              Set precision to all conversions Double --> String
 * - Jul 2024 - Rui Xue, <r.xue@cern.ch><rux23@pitt.edu>,
 *              Added support for virtual surfaces, including:
 *              GeoRectSurface, GeoTrapezoidSurface, GeoAnnulusSurface, GeoDiamondSurface
 */

#include <GeoModelDBManager/GMDBManager.h>

#include "GeoModelKernel/throwExcept.h"
#include "GeoModelHelpers/StringUtils.h"
#include "GeoModelDBManager/GMSQLiteConnection.h"
// include SQLite
#include <sqlite3.h>

// C++ includes
#include <cstdlib> /* exit, EXIT_FAILURE */

#include <mutex>
#include <sstream>
#include <utility>
#include <format>

static std::string dbversion =
    "1.0.0";  // New format with REAL columns for numeric values


class GMDBManager::Imp {
   public:
    // constructor
    explicit Imp(GMDBManager* dbm)
        : theManager(dbm), dbSqlite(dbm->getDBFilePath(), true), m_SQLiteErrMsg(nullptr) {}

    // The class
    GMDBManager* theManager;

    // Pointer to SQLite connection
    GMSQLiteConnection dbSqlite;

    /// Variable to store error messages from SQLite
    char* m_SQLiteErrMsg;

    sqlite3_stmt* selectAllFromTable(const std::string_view tableName) const;
    sqlite3_stmt* selectAllFromTableSortBy(const std::string_view tableName,
                                           const std::string_view sortColumn = "") const;
    sqlite3_stmt* selectAllFromTableChildrenPositions() const;
    bool checkTableFromDB_imp(const std::string& tableName) const;
};

GMDBManager::GMDBManager(const std::string& path)
    : m_loglevel(0), 
    m_dbpath(path), 
    m_d(new Imp(this)) {

    // FIXME: TODO: we should check the existence of the file, otherwise SQLite
    // will create a new file from scratch

    // set verbosity level
    m_loglevel = 0;
    if (const char* env_p = std::getenv("GEOMODEL_GEOMODELIO_LOGLEVEL")) {
        std::cout << "GeoModelDBManager -- You set the verbosity level to: "
                  << env_p << '\n';
        m_loglevel = std::stoi(env_p);

    }

    /// get info from the input DB, if populated,
    /// and create caches storing those pieces of information
    /// This call is only useful when reading an existing DB file.
    /// If the file is new (that is, when we use GMDBManager to write a piece of
    /// geometry into a new file, the call below does nothing.
    createTableDataCaches();
}

GMDBManager::~GMDBManager() {
    delete m_d;
    m_d = nullptr;
}

void GMDBManager::printAllMaterials() const { printAllRecords("Materials"); }
void GMDBManager::printAllElements() const { printAllRecords("Elements"); }

void GMDBManager::printAllShapes() const { 
    printAllRecords("Shapes_Box"); 
    printAllRecords("Shapes_EllipticalTube"); 
    printAllRecords("Shapes_Tube"); 
    printAllRecords("Shapes_Cons"); 
    printAllRecords("Shapes_Para");
    printAllRecords("Shapes_Trap"); 
    printAllRecords("Shapes_Trd"); 
    printAllRecords("Shapes_Tubs"); 
    printAllRecords("Shapes_TwistedTrap"); 
    printAllRecords("Shapes_Pcon"); 
    printAllRecords("Shapes_Pgon"); 
    printAllRecords("Shapes_SimplePolygonBrep"); 
    printAllRecords("Shapes_Intersection"); 
    printAllRecords("Shapes_Shift"); 
    printAllRecords("Shapes_Subtraction"); 
    printAllRecords("Shapes_Union"); 
}

void GMDBManager::printAllShapesData() const { 
    printAllRecords("Shapes_Pcon_Data");
    printAllRecords("Shapes_Pgon_Data");
    printAllRecords("Shapes_SimplePolygonBrep_Data");
}

void GMDBManager::printAllSerialDenominators() const {
    printAllRecords("SerialDenominators");
}

void GMDBManager::printAllSerialIdentifiers() const {
    printAllRecords("SerialIdentifiers");
}

void GMDBManager::printAllIdentifierTags() const {
    printAllRecords("IdentifierTags");
}

void GMDBManager::printAllLogVols() const { printAllRecords("LogVols"); }
void GMDBManager::printAllPhysVols() const { printAllRecords("PhysVols"); }
void GMDBManager::printAllFullPhysVols() const {
    printAllRecords("FullPhysVols");
}
void GMDBManager::printAllSerialTransformers() const {
    printAllRecords("SerialTransformers");
}
void GMDBManager::printAllFunctions() const { printAllRecords("Functions"); }
void GMDBManager::printAllTransforms() const { printAllRecords("Transforms"); }
void GMDBManager::printAllAlignableTransforms() const {
    printAllRecords("AlignableTransforms");
}
void GMDBManager::printAllNameTags() const { printAllRecords("NameTags"); }
void GMDBManager::printAllPublishedFullPhysVols(
    const std::string& suffix) const {
    if ("" == suffix)
        printAllRecords("PublishedFullPhysVols");
    else {
        std::string tableName = "PublishedFullPhysVols";
        tableName += "_";  // separator
        printAllRecords(tableName + suffix);
    }
}
void GMDBManager::printAllPublishedAlignableTransforms(
    const std::string& suffix) const {
    if ("" == suffix)
        printAllRecords("PublishedAlignableTransforms");
    else {
        std::string tableName = "PublishedAlignableTransforms";
        tableName += "_";  // separator
        printAllRecords(tableName + suffix);
    }
}
void GMDBManager::printAllChildrenPositions() const {
    printAllRecords("ChildrenPositions");
}
void GMDBManager::printAllNodeTypes() const {
    printAllRecords("GeoNodesTypes");
}
void GMDBManager::printRootVolumeId() const { printAllRecords("RootVolume"); }
void GMDBManager::printDBVersion() const { printAllRecords("dbversion"); }

void GMDBManager::createTableDataCaches() {
    getAllDBTables();
    getAllDBTableColumns();
}

void GMDBManager::printAllRecords(const std::string& tableName) const {
    // --- print table name
    std::cout << "---\n" ;
    std::cout << "'" << tableName << "' in db:\n";
    // --- check if we stored table's data
    if (m_tableNames.find(tableName) == m_tableNames.end()) {
        std::cout << "\n*** ERROR!! No table '" << tableName
                  << "' found! ***\n\n";
        return;
    }
    // --- print table column names
    std::cout << "- " << GeoStrUtils::chainUp(m_tableNames.at(tableName), ", ")
              << "\n";
    // --- print records
    std::vector<std::vector<std::string>> records;
    records = getTableRecords_String(tableName);
    if (records.size()) {
        for (auto& row : records) {
            std::cout << "* ";
            for (auto& item : row) {
                std::cout << item << ", ";
            }
            std::cout << "\n";
        }
    }
    std::cout << "---" << std::endl;
    // TODO: I want to have a symbol like '---' to mean empty line when
    // query gives 0 results.
}

std::vector<std::vector<std::string>> GMDBManager::getTableRecords_String(
    const std::string_view tableName) const {

    // container to be returned
    std::vector<std::vector<std::string>> records;
    // get the query statetement ready to be executed
    sqlite3_stmt* stmt = nullptr;

    if ("ChildrenPositions" == tableName) {
        stmt = m_d->selectAllFromTableChildrenPositions();
    } else {
        stmt = m_d->selectAllFromTable(tableName);
    }
    // execute the query and loop over all rows and all columuns
    if (stmt) {
        int ctotal = sqlite3_column_count(
            stmt);  // Count the Number of Columns in the Table
        int res = 0;
        while (1) {
            res = sqlite3_step(stmt);  // Execute SQL Statement.
            if (res == SQLITE_ROW) {
                std::vector<std::string>
                    nodeParams;  // stores the data items contained in a
                                 // single row
                for (int i = 0; i < ctotal;
                     i++)  // Loop times the number of columns in the table
                {
                    std::string s;
                    const char* cc = (char*)sqlite3_column_text(
                        stmt,
                        i);  // Read each Column in the row as text FIXME:
                             // is there a method to get the right type,
                             // e.g. double, instead of text?
                    if (cc == NULL)
                        s = "NULL";
                    else
                        s = cc;
                    nodeParams.push_back(s);
                }
                records.push_back(nodeParams);
            }

            if (res == SQLITE_DONE || res == SQLITE_ERROR) {
                if (res == SQLITE_ERROR) {
                    std::string errmsg(m_d->dbSqlite.errmsg());
                    sqlite3_finalize(stmt);
                    THROW_EXCEPTION(errmsg);
                }
                break;
            }
        }
    }
    // finalize
    sqlite3_finalize(stmt);
    return records;
}

// New version with variant
DBRowsList GMDBManager::getTableRecords_VecVecData(
    const std::string_view tableName) const
{
    
    // container to be returned
    DBRowsList records;
    // get the query statetement ready to be executed
    sqlite3_stmt *stmt = nullptr;

    if ("ChildrenPositions" == tableName) {
        stmt = m_d->selectAllFromTableChildrenPositions();
    } else {
        stmt = m_d->selectAllFromTable(tableName);
    }
    // execute the query and loop over all rows and all columuns
    if (stmt) {
        int ctotal = sqlite3_column_count(
            stmt); // Count the Number of Columns in the Table
        int res = 0;
        unsigned nRows{0};
        while (1) {
            res = sqlite3_step(stmt); // Execute SQL Statement.
            if (res == SQLITE_ROW) {
                DBRowEntry
                    nodeParams; // stores the data items contained in a
                                // single row
                // Loop times the number of columns in the table
                for (int i = 0; i < ctotal; i++) {
                    int datacode = sqlite3_column_type(stmt, i);
                    // debug msg
                    // if (0==nRows) std::cout << "table: " << tableName << ", col " << i << "/" << ctotal << " -- typecode: " << datacode << std::endl;

                    int valI{0};
                    double valD{0.};
                    std::string valS;

                    // ** INT **
                    if (SQLiteColumnTypes::INT_TYPE == datacode) {
                        valI = sqlite3_column_int(stmt, i);
                        // if (0==nRows) std::cout << "valI: " << valI << std::endl;// debug msg
                        nodeParams.push_back(valI);
                    }
                    // ** FLOAT **
                    else if (SQLiteColumnTypes::FLOAT_TYPE == datacode) {
                        valD = sqlite3_column_double(stmt, i);
                        // if (0==nRows) std::cout << "valD: " << valD << std::endl;// debug msg
                        nodeParams.push_back(valD);
                    }
                    // ** TEXT **
                    else if (SQLiteColumnTypes::TEXT_TYPE == datacode) {
                        const char *cc = (char *)sqlite3_column_text(stmt, i);
                        if (cc == nullptr) {
                            valS = "NULL";
                        } else {
                            valS = cc;
                        }
                        nodeParams.push_back(valS);
                    }
                    // ** BLOB **
                    else if (SQLiteColumnTypes::BLOB_TYPE == datacode) {
                        THROW_EXCEPTION("ERROR!!! The 'BLOB' data format is not supported yet!!");
                    }
                    // ** NULL **
                    else if (SQLiteColumnTypes::NULL_TYPE == datacode) {
                        THROW_EXCEPTION("ERROR!!! 'NULL' format detected. Check that!");
                    } else {
                        THROW_EXCEPTION("ERROR!!! You should NOT get here!! Unsupported SQLite data typecode: " << datacode << " -- Check this!!");
                    }
                }
                records.push_back(nodeParams);
                ++nRows;
            }

            if (res == SQLITE_DONE || res == SQLITE_ERROR) {
                if (res == SQLITE_ERROR) {
                    std::string errmsg(m_d->dbSqlite.errmsg());
                    sqlite3_finalize(stmt);
                    THROW_EXCEPTION(errmsg);
                }
                break;
            }
        }
    }
    // finalize
    sqlite3_finalize(stmt);
    return records;
}
// New version with variant
DBRowEntry GMDBManager::getTableRecords_VecData(
    const std::string_view tableName) const
{
    if (!checkTableFromCache(tableName)) {
        THROW_EXCEPTION("ERROR!!! Table name '" + std::string(tableName) + "' does not exist in cache! (It has not been loaded from the DB)");
    }
    // container to be returned
    DBRowEntry records;
    // get the query statetement ready to be executed
    sqlite3_stmt *stmt = nullptr;

    if ("ChildrenPositions" == tableName)
    {
        stmt = m_d->selectAllFromTableChildrenPositions();
    }
    else
    {
        stmt = m_d->selectAllFromTable(tableName);
    }
    // execute the query and loop over all rows and all columuns
    if (stmt)
    {
        // Count the Number of Columns in the Table
        int ctotal = sqlite3_column_count(stmt);
        // Note: for this case, we should have only one column of data;
        // so, we throw an error if we get more than two columns in total
        if (ctotal > 2)
        {
            THROW_EXCEPTION("ERROR! Table '" << tableName << "' is supposed to have two columns only, one for the ID and one for actual data; but it has '"
                      << ctotal << "' columns! Check that!!");
        }
        int res = 0;
        while (1)
        {
            res = sqlite3_step(stmt); // Execute SQL Statement.
            if (res == SQLITE_ROW)
            {
                // NOTE: 
                // we are only interested in returning 
                // the 'data' column, that is column '1'
                // (while column '0' contains the ID)
                int colData = 1;
                    int datacode = sqlite3_column_type(stmt, 1);
                    // debug msg
                    // std::cout << "table: " << tableName << ", col " << colData << "/" << ctotal << " -- typecode: " << datacode << std::endl;

                    int valI{0};
                    double valD{0.};
                    std::string valS;
                    if (SQLiteColumnTypes::INT_TYPE == datacode)
                    {
                        valI = sqlite3_column_int(stmt, colData);
                        records.push_back(valI);
                    }
                    else if (SQLiteColumnTypes::FLOAT_TYPE == datacode)
                    {
                        valD = sqlite3_column_double(stmt, colData);
                        records.push_back(valD);
                    }
                    else if (SQLiteColumnTypes::TEXT_TYPE == datacode)
                    {
                        const char *cc = (char *)sqlite3_column_text(stmt, colData);
                        if (cc == NULL)
                        {
                            valS = "NULL";
                        }
                        else
                        {
                            valS = cc;
                        }
                        records.push_back(valS);
                    }
                    else if (SQLiteColumnTypes::BLOB_TYPE == datacode)
                    {
                        THROW_EXCEPTION("ERROR!!! The 'BLOB' data format is not supported yet!!");
                    }
                    else if (SQLiteColumnTypes::NULL_TYPE == datacode)
                    {
                        std::cout << "ERROR!!! 'NULL' format detected. Check that!" << std::endl;
                    }
                    else
                    {
                        THROW_EXCEPTION("ERROR!!! You should NOT get here!! Unsupported SQLite data typecode: " << datacode << " -- Check this!!");
                    }
            }

            if (res == SQLITE_DONE || res == SQLITE_ERROR)
            {
                if (res == SQLITE_ERROR)
                {
                    std::string errmsg(m_d->dbSqlite.errmsg());
                    sqlite3_finalize(stmt);
                    THROW_EXCEPTION(errmsg);
                }
                break;
            }
        }
    }
    // finalize
    sqlite3_finalize(stmt);
    return records;
}

std::vector<std::vector<std::string>> GMDBManager::getTableFromNodeType_String(
    const std::string& nodeType)
{
    std::vector<std::vector<std::string>> out;
    std::string tableName = getTableNameFromNodeType(nodeType);
    if (tableName.empty())
    {
        std::mutex coutMutex;
        coutMutex.lock();
        std::string errMsg = "\t ===> WARNING! The geometry input file does not contain a "
                             "table for the " 
                             + nodeType 
                             + " nodes. That means that you are probably using an "
                             + "old geometry file."
                             + "Unless you know exactly what you are doing, "
                             + "please expect to see incomplete geometries or crashes.\n";
        std::cout << errMsg << std::endl;
        coutMutex.unlock();
    }
    else
    {
        // if (!checkTable(tableName))
        if (!checkTableFromDB(tableName))
        {
            THROW_EXCEPTION("ERROR!!! Table name '" << tableName << "' does not exist in the DB!");
        }
        out = getTableRecords_String(tableName);
    }
    return out;
}

DBRowsList GMDBManager::getTableFromNodeType_VecVecData(
    const std::string& nodeType)
{
    DBRowsList out;
    std::string tableName = getTableNameFromNodeType(nodeType);
    
    if (tableName.empty())
    {
        if (m_loglevel > 1)
        {
            std::mutex coutMutex;
            coutMutex.lock();
            std::cout << "\t ===> WARNING! The geometry input file does not contain a table for the '"
                      << nodeType
                      << "' nodes. That means that you are probably using an "
                      << "old geometry file. Unless you know exactly what you are doing, "
                      << "please expect to see incomplete geometries or crashes.\n"
                      << std::endl;
            coutMutex.unlock();
        }
    }
    else
    {
        if (!checkTableFromDB(tableName))
        {
            THROW_EXCEPTION("ERROR!!! Table name '" + tableName + "' does not exist in the DB!");
        }
        out = getTableRecords_VecVecData(tableName);
    }
    return out;
}
DBRowsList GMDBManager::getTableFromTableName_VecVecData(
    const std::string& tableName)
{
    DBRowsList out;
    if (tableName.empty())
    {
        std::mutex coutMutex;
        coutMutex.lock();
        std::cout << 
            "\t ===> WARNING! The geometry input file does not contain a "
            "table for the " 
            << tableName 
            << "nodes. That means that you are probably using an "
            << "old "
            << "geometry file. Unless you know exactly what you are doing, "
            << "please "
            << "expect to see incomplete geometries or crashes.\n"
            << std::endl;
        coutMutex.unlock();
    }
    else
    {
        if (!checkTableFromDB(tableName))
        {
            THROW_EXCEPTION("ERROR!!! Table name '" + tableName + "' does not exist in the DB!");
        }
        out = getTableRecords_VecVecData(tableName);
    }
    return out;
}
DBRowEntry GMDBManager::getTableFromTableName_VecData(
    const std::string& tableName)
{
    DBRowEntry out;
    if (tableName.empty())
    {
        std::mutex coutMutex;
        coutMutex.lock();
        std::cout << "\t ===> WARNING! The geometry input file does not contain a "
            << "table "
            << "for the "
            << tableName 
            << "nodes. That means that you are probably using an "
            << "old geometry file. Unless you know exactly what you are doing, "
            << "please expect to see incomplete geometries or crashes.\n" 
            << std::endl;
        coutMutex.unlock();
    }
    else
    {
        if (!checkTableFromDB(tableName))
        {
            THROW_EXCEPTION("ERROR!!! Table name '" + tableName + "' does not exist in the DB!");
        }
        out = getTableRecords_VecData(tableName);
    }
    return out;
}

std::vector<double> GMDBManager::getTableFromTableName_VectorDouble(const std::string& tableName)
{
    DBRowEntry inputRecords = getTableFromTableName_VecData(tableName);
    std::vector<double> outRecords;
    for (const auto &rec : inputRecords)
    {
        try
        {
            // std::cout << "variant data: " << GeoStrUtils::to_string_with_precision(std::get<double>(rec)) << '\n';
            outRecords.push_back(std::get<double>(rec));
        }
        catch (std::bad_variant_access const &ex)
        {
            std::cout << ex.what() << ": w contained int/text, not double\n";
        }
    }
    return outRecords;
}

// TODO: make template
// from Johannes: Couldn't you declare this method as a general template method to downcast the variant?
std::deque<double> GMDBManager::getTableFromTableName_DequeDouble(std::string tableName)
{
    std::vector<double> inputRecords = getTableFromTableName_VectorDouble(std::move(tableName));
    // Initialize a deque from vector using range constructor
    std::deque<double> outRecords(inputRecords.begin(), inputRecords.end());
    return outRecords;
}

// TODO: simplify error reporting for SQLite
// void GMDBManager::showError(const QSqlError &err) const
//{
//  qWarning() << "Unable to initialize Database" << "Error initializing
//  database: " + err.text();
//}

bool GMDBManager::addListOfChildrenPositions(
    const DBRowsList& records) {
    if (records.size() > 0) {
        // NOTE: Choose the right function for your version of SQLite!!
        return addListOfRecordsToTable("ChildrenPositions",
                                       records);  // needs SQLite >= 3.7.11
    }
    return false;
}

bool GMDBManager::addListOfPublishedAlignableTransforms(
    const std::vector<std::vector<std::string>>& records,
    const std::string& suffix /* optional parameter */) {
    std::string tableName =
        "PublishedAlignableTransforms";  // default table name
    std::string nodeType = "GeoAlignableTransform";
    const std::type_info& keyType(
        typeid(std::string));  // TODO: type should be custom too!!
    if ("" != suffix)
    {
        tableName += "_";
        tableName += suffix;
        if (m_loglevel > 0)
        {
            std::cout << "\nSaving the published '" << nodeType
                      << "' nodes to the custom table : '" << tableName
                      << "'." << std::endl;
        }
    }
    // create custom table first then add to it
    createTableCustomPublishedNodes(tableName, nodeType, &keyType);
    return addListOfRecordsToTable(tableName,
                                   records);  // needs SQLite >= 3.7.11
}

bool GMDBManager::addListOfPublishedFullPhysVols(
    const std::vector<std::vector<std::string>>& records,
    const std::string& suffix /* optional parameter */) {
    std::string tableName = "PublishedFullPhysVols";  // default table name
    std::string nodeType = "GeoFullPhysVol";
    const std::type_info& keyType(
        typeid(std::string));  // TODO: type should be custom too!!
    if ("" != suffix)
    {
        tableName += "_";
        tableName += suffix;
        if (m_loglevel > 0)
        {
            std::cout << "\nSaving the published '" << nodeType
                      << "' nodes to the custom table : '"
                      << tableName << "'." << std::endl;
        }
    }
    // create custom table first then add to it
    createTableCustomPublishedNodes(tableName, nodeType, &keyType);
    return addListOfRecordsToTable(tableName,
                                   records);  // needs SQLite >= 3.7.11
}


bool GMDBManager::addListOfRecords(
    const std::string& geoType,
    const std::vector<std::vector<std::string>>& records) {

    std::string tableName = m_childType_tableName[geoType];

    if (tableName.size() == 0) {
        THROW_EXCEPTION("ERROR!! could not retrieve tableName for node type '" + geoType + "'!! Aborting...\n");
    }

    if (records.size() > 0) {
        return addListOfRecordsToTable(
            tableName, records);  // needs SQLite >= 3.7.11
    } else {
        if (m_loglevel > 1)
            std::cout << "Info: no records to save for geoType '" << geoType
                      << "'. Skipping..." << std::endl;
    }
    return true;
}

bool GMDBManager::addListOfRecords(
    const std::string& geoType,
    const DBRowsList& records) {
    std::string tableName = m_childType_tableName[geoType];

    if (tableName.size() == 0) {
        THROW_EXCEPTION("ERROR!! could not retrieve tableName for node type '" + geoType + "'!! Aborting...\n");
    }

    if (records.size() > 0) {
        return addListOfRecordsToTable(
            tableName, records);  // needs SQLite >= 3.7.11
    } else {  
        if (m_loglevel > 1)
            std::cout << "Info: no records to save for geoType '" << geoType
                      << "'. Skipping..." << std::endl;
    }
    return true;
}

// ***Note***
// The syntax built here below is more convenient
// but it is supported only in SQLite >= 3.7.11
//
// Here we build a query like this:
// queryStr = QString("INSERT INTO Materials (id, name) VALUES  (1,'Air'),
// (2,'Silicon'), (368,'ShieldSteel');");
//
bool GMDBManager::addListOfRecordsToTable(
    const std::string& tableName,
    const std::vector<std::vector<std::string>>& records) {
    if ( !(hasTableBeenCreatedInDB(tableName)) ) {
        THROW_EXCEPTION("ERROR!!! The DB has no '" << tableName << "' table; probably, the table has not been created in the DB.");
    }
    // get table columns and format them for query
    std::string tableColString =
        "(" + GeoStrUtils::chainUp(m_tableNames.at(tableName), ", ") + ")";
    if (m_loglevel > 2) std::cout << "tableColString:" << tableColString << std::endl;

    unsigned int nRecords = records.size();
    std::cout << "Info: number of " << tableName << " records to dump into the DB: " << nRecords << std::endl;

    // preparing the SQL query
    std::string sql = std::format("INSERT INTO {0} {1} VALUES ", tableName, tableColString);
    unsigned int id = 0;
    for (const std::vector<std::string>& rec : records) {
        ++id;
        std::vector<std::string> items;
        items.reserve(rec.size());
        for (const std::string& item : rec) {
            items.push_back(
                "'" + item +
                "'");  // TODO: we should differentiate strings from other
                       // values when inserting them in the table, as we now
                       // do for the std::variant version!
        }
        std::string values = GeoStrUtils::chainUp(items, ",");
        sql += " (" + std::to_string(id) + "," + values + ")"; // INT
        if (id != nRecords) {
            sql += ",";
        } else {
            sql += ";";
        }
    }
    if (m_loglevel > 2) std::cout << "Query string:" << sql << std::endl;  // debug

    // executing the SQL query
    if (!(execQuery(sql))) {
        return false;
    }
    return true;
}

bool GMDBManager::addListOfRecordsToTable(
    const std::string& tableName,
    const DBRowsList& records) {

    if ( !(hasTableBeenCreatedInDB(tableName)) ) {
        THROW_EXCEPTION("ERROR!!! The DB has no '" << tableName << "' table; probably, the table has not been created in the DB.");
    }

    if (records.size() > 0) {
    // get table columns and format them for query
    std::string tableColString =
        "(" + GeoStrUtils::chainUp(m_tableNames.at(tableName), ", ") + ")";
    if (m_loglevel > 2) std::cout << "tableColString:" << tableColString << std::endl;

    unsigned int nRecords = records.size();
    std::cout << "Info: number of " << tableName
              << " records to dump into the DB: " << nRecords << std::endl;


    // preparing the SQL query
    std::string sql =
        std::format("INSERT INTO {0} {1} VALUES ", tableName, tableColString);
    unsigned int id = 0;
    for (const DBRowEntry&
             rec : records) {
        ++id;
        // a vector to store string-conversions of values, to build the SQL
        // query
        std::vector<std::string> items;
        // loop over all entries in a row/record
        for (const std::variant<int, long, float, double, std::string>& item :
             rec) {
            if (std::holds_alternative<int>(item))
                items.push_back(std::to_string(std::get<int>(item))); // INT
            else if (std::holds_alternative<long>(item))
                items.push_back(std::to_string(std::get<long>(item))); // INT
            else if (std::holds_alternative<float>(item))
                items.push_back(GeoStrUtils::to_string_with_precision(std::get<float>(item)));
            else if (std::holds_alternative<double>(item))
                items.push_back(GeoStrUtils::to_string_with_precision(std::get<double>(item)));
            else if (std::holds_alternative<std::string>(item)) {
                std::string str = std::get<std::string>(item);
                // NOTE: if item is a "NULL" string, we don't encapsulate it
                // into quotes, so it is taken as the SQL's NULL value in
                // the SQL string, and inserted as a NULL value in the
                // table, instead of as a "NULL" text string
                if (str == "NULL")
                    items.push_back(str);
                else
                    items.push_back("'" + str + "'");
            } else
                THROW_EXCEPTION("No std::variant alternative found!");
        }
        // we build the long string containing all values
        std::string values = GeoStrUtils::chainUp(items, ",");
        sql += " (" + std::to_string(id) + "," + values + ")"; // INT
        if (id != nRecords) {
            sql += ",";
        } else {
            sql += ";";
        }
    }
    if (m_loglevel > 2) std::cout << "Query string:" << sql << std::endl;  // debug

    // executing the SQL query
    if (!(execQuery(sql))) {
        return false;
    }
    return true;
  } 
  return false;
}

bool GMDBManager::addRecordsToTable(
    const std::string& tableName,
    const DBRowEntry& records)
{
    if (records.size() > 0) {
    // get table columns and format them for query
    std::string tableColString =
        "(" + GeoStrUtils::chainUp(m_tableNames.at(tableName), ", ") + ")";
    if (m_loglevel > 2) std::cout << "tableColString:" << tableColString << std::endl;

    unsigned int nRecords = records.size();

    std::cout << "Info: number of " << tableName
              << " records to dump into the DB: " << nRecords << std::endl;

    // preparing the SQL query
    std::string sql =
        std::format("INSERT INTO {0} {1} VALUES ", tableName, tableColString);
    unsigned int id = 0;
    // a vector to store string-conversions of values, to build the SQL
    // query
    // TODO: we should remove this TEXT-conversion 
    // and we should use prepared statements instead!
    std::vector<std::string> items;
    // loop over all entries in a row/record
    for (const std::variant<int, long, float, double, std::string> &item :
         records)
    {
        ++id;

        std::string startRow = "(" + std::to_string(id) + ", "; // INT
        items.push_back(startRow);

        if (std::holds_alternative<int>(item))
            items.push_back(std::to_string(std::get<int>(item))); // INT
        else if (std::holds_alternative<long>(item))
            items.push_back(std::to_string(std::get<long>(item))); // INT
        else if (std::holds_alternative<float>(item))
            items.push_back(GeoStrUtils::to_string_with_precision(std::get<float>(item)));
        else if (std::holds_alternative<double>(item))
            items.push_back(GeoStrUtils::to_string_with_precision(std::get<double>(item)));
        else if (std::holds_alternative<std::string>(item))
        {
            std::string str = std::get<std::string>(item);
            // NOTE: if item is a "NULL" string, we don't encapsulate it
            // into quotes, so it is taken as the SQL's NULL value in
            // the SQL string, and inserted as a NULL value in the
            // table, instead of as a "NULL" text string
            if (str == "NULL")
            {
                items.push_back(str);
            }
            else
            {
                items.push_back("'" + str + "'");
            }
        }
        else
        {
            THROW_EXCEPTION("No std::variant alternative found!");
        }

        std::string endRow = ")";
        if (id != nRecords)
        {
            endRow += ", ";
        }
        else
        {
            endRow += ";";
        }
        items.push_back(endRow);
    }
    // we build the long SQL string containing all values
    std::string values = GeoStrUtils::chainUp<std::string>(items, "");

    sql += " " + values + ";";
    if (m_loglevel > 2)
        std::cout << "Query string:" << sql << std::endl; // debug

    // executing the SQL query
    if (!(execQuery(sql)))
    {
        return false;
    }
    return true;
    } 
    return false;
}


// bool GMDBManager::addRootVolume(const std::vector<std::string>& values) {
bool GMDBManager::addRootVolume(const std::string_view type, const unsigned id)
{
    const std::string nodeType{type};
    const unsigned int volId{id};
    return storeRootVolume(volId, nodeType);
}
bool GMDBManager::addRootVolume(const std::pair<std::string, unsigned>& rootValues)
{
    const std::string nodeType{rootValues.first};
    const unsigned int volId{rootValues.second};
    return storeRootVolume(volId, nodeType);
}

void GMDBManager::addDBversion(const std::string& version) {
    checkIsDBOpen();
    sqlite3_stmt* st = nullptr;
    int rc = -1;
    std::string sql = "INSERT INTO dbversion(version) VALUES(?)";
    rc = sqlite3_prepare_v2(m_d->dbSqlite.ptr(), sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        THROW_EXCEPTION("[SQLite ERR] (" << __func__ << ") : Error msg: " << m_d->dbSqlite.errmsg());
    }
    rc = sqlite3_bind_text(st, 1, version.c_str(), version.length(),
                           SQLITE_TRANSIENT);
    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) {
        THROW_EXCEPTION("[SQLite ERR] (" << __func__ << ") : Error msg: " << m_d->dbSqlite.errmsg());
    }
    // finalize
    sqlite3_finalize(st);
    return;
}

bool GMDBManager::checkIsDBOpen() const {
    if (m_d->dbSqlite.ptr() != nullptr) {
        return true;
    } else {
        THROW_EXCEPTION("ERROR! The SQLite DB is not accessible! Exiting...");
    }
}

std::vector<std::string> GMDBManager::getItemAndType(unsigned int tableId,
                                                     unsigned int id) {
    std::vector<std::string> results;
    std::string tableName = getTableNameFromTableId(tableId);
    std::string nodeType = getNodeTypeFromTableId(tableId);
    std::vector<std::string> item = getItemFromTableName(tableName, id);
    // add the type
    results.push_back(nodeType);
    // add the item
    results.insert(results.end(), item.begin(), item.end());
    return results;
}

std::vector<std::string> GMDBManager::getItemFromTableName(
    const std::string& tableName, unsigned int id) {
    // FIXME: when you create caches for all tables, replace this method
    // with a lookup action in the cache.
    /*
     * Get the object from DB.
     */
    std::vector<std::string> item;
    // set a SQL command string with the right table name
    std::string sql =
        std::format("SELECT * FROM {0} WHERE id = (?)", tableName);
    // prepare the query
    sqlite3_stmt* stmt = nullptr;
    int rc = -1;
    rc = sqlite3_prepare_v2(m_d->dbSqlite.ptr(), sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        THROW_EXCEPTION("[SQLite ERR] 'prepare' (" << __func__ << ") : Error msg: " << m_d->dbSqlite.errmsg());
    }
    // bind the parameters
    rc = sqlite3_bind_int(stmt, 1, id);
    // execute the query and loop over all rows and all columuns
    if (stmt) {
        int ctotal = sqlite3_column_count(
            stmt);  // Count the Number of Columns in the Table
        int res = 0;
        while (1) {
            res = sqlite3_step(stmt);  // Execute SQL Statement.
            if (res == SQLITE_ROW) {
                for (int i = 0; i < ctotal;
                     i++)  // Loop times the number of columns in the table
                {
                    std::string s = (char*)sqlite3_column_text(
                        stmt,
                        i);  // Read each Column in the row as text FIXME:
                             // is there a method to get the right type,
                             // e.g. double, instead of text?
                    item.push_back(s);
                }
            }
            if (res == SQLITE_DONE || res == SQLITE_ERROR) {
                if (res == SQLITE_ERROR) {
                    std::string errmsg(m_d->dbSqlite.errmsg());
                    sqlite3_finalize(stmt);
                    THROW_EXCEPTION(errmsg);
                }
                break;
            }
        }
    }
    
    // finalize
    sqlite3_finalize(stmt);

    if (item.size() == 0) {
        THROW_EXCEPTION("ERROR!! Item with ID:'" << id << "' does not exist in table"
                  << tableName << "! Exiting...");
    }

    return item;
}

// Get all parent-children data from the database in one go
DBRowsList GMDBManager::getChildrenTable() {
    return getTableRecords_VecVecData("ChildrenPositions");
}

unsigned int GMDBManager::getTableIdFromNodeType(const std::string_view nodeType) {
    if (m_cache_nodeType_tableID.empty()) THROW_EXCEPTION("ERROR! Cache is empty!");
    return m_cache_nodeType_tableID[std::string(nodeType)];
}

std::string GMDBManager::getTableNameFromNodeType(const std::string& nodeType) {
    std::string st{""};
    if (m_cache_nodeType_tableName.count(nodeType)) {
        st = m_cache_nodeType_tableName.at(nodeType);
    } else {
        if (m_loglevel > 1)
        {

            std::mutex coutMutex;
            coutMutex.lock();
            std::cout << "\t ===> WARNING! A table for nodeType '" << nodeType
                      << "' has not been found in the input geometry file."
                      << std::endl;
            coutMutex.unlock();
        }
    }
    return st;
}

// build the GeoNodeTypes cache
int GMDBManager::loadGeoNodeTypesAndBuildCache() {
    checkIsDBOpen();
    std::string queryStr;
    sqlite3_stmt* st = nullptr;
    int rc = -1;
    // set the SQL query string
    std::string sql = "SELECT * FROM GeoNodesTypes";
    // declare the data we want to fetch
    unsigned int id = 0;
    std::string nodeType = "";
    std::string tableName = "";
    // prepare the query
    rc = sqlite3_prepare_v2(m_d->dbSqlite.ptr(), sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        THROW_EXCEPTION("[SQLite ERR] (" << __func__ << ") : Error msg: " << m_d->dbSqlite.errmsg());
    }
    // execute the statement until all selected records are processed
    while ((rc = sqlite3_step(st)) == SQLITE_ROW) {
        // get the data
        id = sqlite3_column_int(st, 0);
        nodeType = std::string(
            reinterpret_cast<const char*>(sqlite3_column_text(st, 1)));
        tableName = std::string(
            reinterpret_cast<const char*>(sqlite3_column_text(st, 2)));
        if (m_loglevel > 2)
            std::cout << "row: " << id << "," << nodeType << "," << tableName
                      << std::endl;
        // fill the caches
        m_cache_tableId_tableName.insert(
            std::pair<unsigned int, std::string>(id, tableName));
        m_cache_tableId_nodeType.insert(
            std::pair<unsigned int, std::string>(id, nodeType));
        m_cache_nodeType_tableName.insert(
            std::pair<std::string, std::string>(nodeType, tableName));
        m_cache_nodeType_tableID.insert(
            std::pair<std::string, unsigned int>(nodeType, id));
    }
    if (rc != SQLITE_DONE) {
        std::string errmsg(m_d->dbSqlite.errmsg());
        sqlite3_finalize(st);
        THROW_EXCEPTION(errmsg);
    }
    // finalize
    sqlite3_finalize(st);
    return rc;
}

std::string GMDBManager::getTableNameFromTableId(unsigned int tabId) {
    if (m_cache_tableId_tableName.empty()) THROW_EXCEPTION("ERROR! Cache is empty!");
    return m_cache_tableId_tableName[tabId];
}

std::string GMDBManager::getNodeTypeFromTableId(unsigned int tabId) {
    if (m_cache_tableId_nodeType.empty()) THROW_EXCEPTION("ERROR! Cache is empty!");
    return m_cache_tableId_nodeType[tabId];
}

std::unordered_map<unsigned int, std::string>
GMDBManager::getAll_TableIDsNodeTypes() {
    return m_cache_tableId_nodeType;
}

std::unordered_map<std::string, unsigned int>
GMDBManager::getAll_NodeTypesTableIDs() {
    return m_cache_nodeType_tableID;
}

sqlite3_stmt* GMDBManager::Imp::selectAllFromTable(
    const std::string_view tableName) const {
    return selectAllFromTableSortBy(tableName, "id");
}

sqlite3_stmt* GMDBManager::Imp::selectAllFromTableSortBy(
    const std::string_view tableName, const std::string_view sortCol) const {
    std::string sortColumn{sortCol};
    theManager->checkIsDBOpen();
    if ("" == sortColumn || 0 == sortColumn.size()) {
        sortColumn = "id";
    }
    sqlite3_stmt* st = nullptr;  // SQLite statement to be returned
    int rc = -1;                 // SQLite return code
    // set the SQL query string
    std::string sql =
        std::format("SELECT * FROM {0} ORDER BY {1}", tableName, sortColumn);
    // prepare the query
    rc = sqlite3_prepare_v2(dbSqlite.ptr(), sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        THROW_EXCEPTION("[SQLite ERR] (" << __func__ << ") : Error msg: " << dbSqlite.errmsg());
    }
    return st;
}

bool GMDBManager::Imp::checkTableFromDB_imp(const std::string& tableName) const {
    theManager->checkIsDBOpen();
    sqlite3_stmt* st = nullptr;  // SQLite statement to be returned
    int rc = -1;                 // SQLite return code
    // set the SQL query string
    std::string sql = std::format("SELECT * FROM {0}", tableName);
    // prepare the query
    rc = sqlite3_prepare_v2(dbSqlite.ptr(), sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) return false;
    return true;
}

sqlite3_stmt* GMDBManager::Imp::selectAllFromTableChildrenPositions() const {
    theManager->checkIsDBOpen();
    sqlite3_stmt* st = nullptr;
    int rc = -1;
    // set the SQL query string
    std::string sql =
        "SELECT * FROM ChildrenPositions ORDER BY parentTable, parentId, "
        "parentCopyNumber, position";
    // prepare the query
    rc = sqlite3_prepare_v2(dbSqlite.ptr(), sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        THROW_EXCEPTION("[SQLite ERR] (" << __func__ << ") : Error msg: " << dbSqlite.errmsg());
    }
    return st;
}

bool GMDBManager::initDB() {
    // create tables
    bool tablesOK = createTables();
    // store DB version
    addDBversion(dbversion);
    return tablesOK;
}


bool GMDBManager::checkTableFromCache(const std::string_view tableName) const
{
    std::string tableNameStr{tableName};
    if (m_cache_tables.size() == 0)
    {
        return false;
    }
    else if (m_cache_tables.find(tableNameStr) != m_cache_tables.end())
    {
        return true;
    }
    return false;
}

void GMDBManager::printAllDBTables() {
    if (m_cache_tables.size() == 0) {
        getAllDBTables();  // load tables and build the cache
    }
    for (const auto& str : m_cache_tables) std::cout << str << std::endl;
}

void GMDBManager::getAllDBTables() {
    std::string tableName;
    std::set<std::string> tables;
    // define a query string containing the necessary SQL instructions
    std::string queryStr =
        "SELECT name FROM sqlite_master WHERE type ='table' AND name NOT "
        "LIKE "
        "'sqlite_%';";
    // prepare the query with the query string
    sqlite3_stmt* stmt;
    int rc =
        sqlite3_prepare_v2(m_d->dbSqlite.ptr(), queryStr.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        THROW_EXCEPTION(m_d->dbSqlite.errmsg());
    }
    // execute the statement until all selected records are processed
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        tableName = std::string(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        // std::cout << "tableName: " << tableName << std::endl; // debug
        tables.insert(tableName);
    }
    if (rc != SQLITE_DONE) {
        std::string errmsg(m_d->dbSqlite.errmsg());
        sqlite3_finalize(stmt);
        THROW_EXCEPTION(errmsg);
    }
    // finalize
    sqlite3_finalize(stmt);

    m_cache_tables = tables;
}

void GMDBManager::getAllDBTableColumns() {
    std::vector<std::string> cols;
    std::string colName;

    // populate the cache with tables' names, if needed
    if (!m_cache_tables.size()) {
        if (m_loglevel > 2) {
            std::cout << "*** WARNING! Tables' cache is empty! That's normal if you are saving a GeoModel tree into a new .db file. ***\n";
        }
        return;
    }

    for (const auto& tableName : m_cache_tables) {
        sqlite3_stmt* stmt;
        // get the 'name' column from the PRAGMA's table's definition
        // see: https://stackoverflow.com/a/54962853/320369
        std::string queryStr =
            "select name from pragma_table_info('" + tableName + "')";
        int rc = sqlite3_prepare_v2(m_d->dbSqlite.ptr(), queryStr.c_str(), -1,
                                    &stmt, NULL);
        if (rc == SQLITE_OK) {
            // OK, now looping over table's columns...
            // will continue to go down the rows (columns in your table)
            // till there are no more
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                // sprintf(colName, "%s", sqlite3_column_text(stmt, 1));
                colName = std::string(reinterpret_cast<const char*>(
                    sqlite3_column_text(stmt, 0)));

                // do something with colName because it contains the
                // column's name
                m_tableNames[tableName].push_back(colName);
            }
        }
    }  // end of for loop
}

// TODO: currently, we retrieve published data as strings, but we want to
// retrieve that according to the original data type
// std::vector<std::vector<std::string>> GMDBManager::getPublishedFPVTable(
DBRowsList GMDBManager::getPublishedFPVTable(
    const std::string_view suffix) {
    std::string tableName = "PublishedFullPhysVols";  // default table name
    if ( !suffix.empty() ) {
        tableName += "_";
        tableName += suffix;
    }
    if (!checkTableFromCache(tableName))
    {
        THROW_EXCEPTION("ERROR!!! Table name '" + tableName + "' does not exist in the DB!");
    }
    return getTableRecords_VecVecData(tableName);
}
// TODO: currently, we retrieve published data as strings, but we want to
// retrieve that according to the original data type
// std::vector<std::vector<std::string>> GMDBManager::getPublishedAXFTable(
DBRowsList GMDBManager::getPublishedAXFTable(
    const std::string_view suffix) {
    std::string tableName =
        "PublishedAlignableTransforms";  // default table name
    if ( !suffix.empty() ) {
        tableName += "_";
        tableName += suffix;
    }
    if (!checkTableFromCache(tableName))
    {
        THROW_EXCEPTION("ERROR!!! Table name '" + tableName + "' does not exist in cache! (It has not been loaded from the DB)");
    }
    return getTableRecords_VecVecData(tableName);
}

bool GMDBManager::checkTableFromDB(const std::string& tableName) const {
    return m_d->checkTableFromDB_imp(tableName);
}

// create a user-defined custom table to store the published nodes
// (usually GeoFullPhysVol and AlignableTransform nodes) and their keys.
bool GMDBManager::createTableCustomPublishedNodes(
    const std::string& tableName, const std::string& nodeType,
    const std::type_info* keyType) {
    // get the right node type and referenced table
    if (nodeType != "GeoFullPhysVol" && nodeType != "GeoVFullPhysVol" &&
        nodeType != "GeoAlignableTransform") {
        THROW_EXCEPTION("ERROR!! GeoModel node type '" << nodeType
                  << "' is not currently supported in "
                     "GMDBManager::createTableCustomPublishedNodes()"
                  << " Please, ask to geomodel-developers@cern.ch. Exiting...");
    }
    std::string referencedTable = "";
    if ("GeoFullPhysVol" == nodeType || "GeoVFullPhysVol" == nodeType)
        referencedTable = "FullPhysVols";
    if ("GeoAlignableTransform" == nodeType)
        referencedTable = "AlignableTransforms";

    // set the right type to use in the DB, based on the type used for the
    // published key
    std::string keyTypeDB = "";
    if (typeid(std::string) == *keyType)
        keyTypeDB = "varchar";
    else if (typeid(int) == *keyType || typeid(unsigned) == *keyType)
        keyTypeDB = "integer";
    else {
        THROW_EXCEPTION("ERROR!!! The key type '" << typeid(keyType).name()
                  << "' is not currently supported in "
                     "GMDBManager::createTableCustomPublishedNodes()."
                  << " Please, ask to 'geomodel-developers@cern.ch'. Exiting...");                  
    }

    int rc = -1;  // sqlite's return code
    std::string queryStr;

    std::vector<std::string> tab;

    tab.insert(tab.begin(), {tableName, "id", "key", "nodeID", "keyType"});
    storeTableColumnNames(tab);

    queryStr = std::format(
        "create table {0} ({1} integer primary key, {2} {3} not null, {4} "
        "integer not null REFERENCES {5}(id), {6} varchar not null)",
        tab[0], tab[1], tab[2], keyTypeDB, tab[3], referencedTable, tab[4]);
    // std::cout << "Creating table with query: '" << queryStr << "'..." <<
    // std::endl; // debug msg
    rc = execQuery(queryStr);
    tab.clear();
    // std::cout << "Created the custom table: '" << tableName << "'." <<
    // std::endl; // debug msg
    return rc;
}


// create a user-defined custom table to store auxiliary data, from
// vector<vector<variant>>
bool GMDBManager::createCustomTable(
    const std::string& tableName, const std::vector<std::string>& tableColNames,
    const std::vector<std::string>& tableColTypes,
    const std::vector<
        DBRowEntry>&
        records) {
    if (tableColNames.size() == 0) {
        THROW_EXCEPTION("GMDBManager::createCustomTable -- The list of columns' names "
            "is empty!!");
    }
    if (tableColTypes.size() == 0) {
        THROW_EXCEPTION("GMDBManager::createCustomTable -- The list of columns' types "
            "is empty!!");
    }

    std::string queryStr;

    std::vector<std::string> tab;

    tab.push_back(tableName);
    tab.push_back("id");  // this is the column to store the records' IDs
    for (const auto& colName : tableColNames) tab.push_back(colName);

    storeTableColumnNames(tab);

    // prepare the dynamic query to create the custom table
    queryStr =
        std::format("create table {0} ( id integer primary key ", tab[0]);
    for (size_t ii = 0; ii < tableColNames.size(); ++ii) {
        std::string colType = "";

        // *** NOTE: ***
        // -- Here we check the datum's type, which is more universal than
        // using string-encoded types
        // -- but this does not work if the first entry of a given column is
        // empty.
        // -- If so, the 'null' entry is taken as string, and the column is
        // set as containing TEXT... convert std::variant types to SQLite
        // data types (see SQLite documentation)
        /*
        if(      std::holds_alternative<int>(records[0][ii]) ) colType =
        "INTEGER"; else if( std::holds_alternative<long>(records[0][ii]) )
        colType = "INTEGER"; else if(
        std::holds_alternative<float>(records[0][ii]) )       colType =
        "REAL"; else if( std::holds_alternative<double>(records[0][ii]) )
        colType = "REAL"; else if(
        std::holds_alternative<std::string>(records[0][ii]) ) colType =
        "TEXT"; else throw std::runtime_error("No std::variant alternative
        has been found!\n");
        */

        if (tableColTypes[ii] == "INT")
            colType = "INTEGER";
        else if (tableColTypes[ii] == "LONG")
            colType = "INTEGER";
        else if (tableColTypes[ii] == "FLOAT")
            colType = "REAL";
        else if (tableColTypes[ii] == "DOUBLE")
            colType = "REAL";
        else if (tableColTypes[ii] == "STRING")
            colType = "TEXT";
        else
             THROW_EXCEPTION("No suitable column type has been found ==> " +
                tableColTypes[ii]);

        std::string colStr =
            std::format(", {0} {1} ", tableColNames[ii], colType);
        queryStr += colStr;
    }
    queryStr += ")";
    if (m_loglevel > 0)
        std::cout << "- table definition: " << queryStr << std::endl;

    (void)execQuery(queryStr);
    tab.clear();
    return addListOfRecordsToTable(tableName,
                                   records);  // needs SQLite >= 3.7.11
}

bool GMDBManager::createTables() {
    checkIsDBOpen();

    int rc = -1;  // sqlite return code
    std::string queryStr;

    std::string geoNode;
    std::string tableName;
    std::vector<std::string> tab;

    // create a table to store the relation between the types of GeoNodes
    // and the name of the table
    tableName = "dbversion";
    tab.insert(tab.begin(), {tableName, "id", "version"});
    storeTableColumnNames(tab);
    queryStr =
        std::format("create table {0} ({1} integer primary key, {2} integer)",
                    tab[0], tab[1], tab[2]);
    rc = execQuery(queryStr);
    tab.clear();

    // create a table to store the relation between the types of GeoNodes
    // and the name of the table
    tableName = "GeoNodesTypes";
    tab.insert(tab.begin(), {tableName, "id", "nodeType", "tableName"});
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} varchar, {3} "
        "varchar)",
        tab[0], tab[1], tab[2], tab[3]);
    rc = execQuery(queryStr);
    tab.clear();

    // create a table to store the mother-daughter relationships between
    // nodes (notably, between the [Full]PhysVols as the parents and their
    // children)
    tableName = "ChildrenPositions";
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("parentId");
    tab.push_back("parentTable");
    tab.push_back("parentCopyNumber");
    tab.push_back("position");
    tab.push_back("childTable");
    tab.push_back("childId");
    tab.push_back("childCopyNumber");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} integer, "
        "{3} integer not null REFERENCES GeoNodesTypes(id), "
        "{4} integer, {5} integer, "
        "{6} integer not null REFERENCES GeoNodesTypes(id), "
        "{7} integer not null, "
        "{8} integer not null)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7], tab[8]);
    rc = execQuery(queryStr);
    tab.clear();


    // create a table to store the numeric data used in Functions
    tableName = "FuncExprData";
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("data");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real not null)",
        tab[0], tab[1], tab[2]);
    rc = execQuery(queryStr);
    tab.clear();
    
    // create a table to store information about the 'root' volume (also
    // known as the 'world' volume)
    tableName = "RootVolume";
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("volId");
    tab.push_back("volTable");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} integer not null, "
        "{3} "
        "integer not null REFERENCES GeoNodesTypes(id))",
        tab[0], tab[1], tab[2], tab[3]);
    rc = execQuery(queryStr);
    tab.clear();

    // PhysVols table
    geoNode = "GeoPhysVol";
    tableName = "PhysVols";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("logvol");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} integer not null)",
        tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // FullPhysVols table
    geoNode = "GeoFullPhysVol";
    tableName = "FullPhysVols";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("logvol");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} integer not null)",
        tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // LogVols table
    geoNode = "GeoLogVol";
    tableName = "LogVols";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("name");
    tab.push_back("shape");
    tab.push_back("shapeType");
    tab.push_back("material");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} varchar, {3} "
        "integer "
        "not null, {4} varchar not null, {5} integer not null)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Materials table
    geoNode = "GeoMaterial";
    tableName = "Materials";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("name");
    tab.push_back("density");
    tab.push_back("dataStart");
    tab.push_back("dataEnd");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} varchar, "
        "{3} real, {4} integer, {5} integer)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // create a table to store the numeric data used in GeoPcon shapes
    tableName = "Materials_Data";
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("element");
    tab.push_back("fraction");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, "
        "{2} integer not null, "
        "{3} real )",
        tab[0], tab[1], tab[2], tab[3]);
    rc = execQuery(queryStr);
    tab.clear();

    // Elements table
    geoNode = "GeoElement";
    tableName = "Elements";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("name");
    tab.push_back("symbol");
    tab.push_back("Z");
    tab.push_back("A");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} varchar, "
        "{3} varchar, "
        "{4} real, {5} real)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // TODO: to be removed as soson as all shapes are migrated to the new DB schema
    // Shapes table
    geoNode = "GeoShape";
    tableName = "Shapes";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("type");
    tab.push_back("parameters");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} varchar, {3} "
        "varchar)",
        tab[0], tab[1], tab[2], tab[3]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Shapes-Box table
    // ID, XHalfLength, YHalfLength, ZHalfLength
    geoNode = "GeoBox";
    tableName = "Shapes_Box";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("XHalfLength");
    tab.push_back("YHalfLength");
    tab.push_back("ZHalfLength");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-EllipticalTube table
    // ID, XHalfLength, YHalfLength, ZHalfLength
    geoNode = "GeoEllipticalTube";
    tableName = "Shapes_EllipticalTube";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("XHalfLength");
    tab.push_back("YHalfLength");
    tab.push_back("ZHalfLength");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-Tube table
    geoNode = "GeoTube";
    tableName = "Shapes_Tube";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("RMin");
    tab.push_back("RMax");
    tab.push_back("ZHalfLength");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-Cons table
    geoNode = "GeoCons";
    tableName = "Shapes_Cons";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("RMin1");
    tab.push_back("RMin2");
    tab.push_back("RMax1");
    tab.push_back("RMax2");
    tab.push_back("DZ");
    tab.push_back("SPhi");
    tab.push_back("DPhi");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real, {6} real, {7} real, {8} real, {9} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7], tab[8], tab[9]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-Para table
    geoNode = "GeoPara";
    tableName = "Shapes_Para";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("XHalfLength");
    tab.push_back("YHalfLength");
    tab.push_back("ZHalfLength");
    tab.push_back("Alpha");
    tab.push_back("Theta");
    tab.push_back("Phi");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real, {6} real, {7} real, {8} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7], tab[8] );
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-Trap table
    geoNode = "GeoTrap";
    tableName = "Shapes_Trap";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("ZHalfLength");
    tab.push_back("Theta");
    tab.push_back("Phi");
    tab.push_back("Dydzn");
    tab.push_back("Dxdyndzn");
    tab.push_back("Dxdypdzn");
    tab.push_back("Angleydzn");
    tab.push_back("Dydzp");
    tab.push_back("Dxdyndzp");
    tab.push_back("Dxdypdzp");
    tab.push_back("Angleydzp");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real, {6} real, {7} real, {8} real, {9} real, {10} real, {11} real, {12} real, {13} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7], tab[8], tab[9], tab[10], tab[11], tab[12], tab[13] );
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Shapes-Trd table
    geoNode = "GeoTrd";
    tableName = "Shapes_Trd";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("XHalfLength1");
    tab.push_back("XHalfLength2");
    tab.push_back("YHalfLength1");
    tab.push_back("YHalfLength2");
    tab.push_back("ZHalfLength");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real, {6} real, {7} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7] );
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-Tubs table
    geoNode = "GeoTubs";
    tableName = "Shapes_Tubs";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("RMin");
    tab.push_back("RMax");
    tab.push_back("ZHalfLength");
    tab.push_back("SPhi");
    tab.push_back("DPhi");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real, {6} real, {7} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7] );
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-Tubs table
    geoNode = "GeoTorus";
    tableName = "Shapes_Torus";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("RMin");
    tab.push_back("RMax");
    tab.push_back("RTor");
    tab.push_back("SPhi");
    tab.push_back("DPhi");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real, {6} real, {7} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7] );
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-TwistedTrap table
    geoNode = "GeoTwistedTrap";
    tableName = "Shapes_TwistedTrap";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("PhiTwist");
    tab.push_back("ZHalfLength");
    tab.push_back("Theta");
    tab.push_back("Phi");
    tab.push_back("Y1HalfLength");
    tab.push_back("X1HalfLength");
    tab.push_back("X2HalfLength");
    tab.push_back("Y2HalfLength");
    tab.push_back("X3HalfLength");
    tab.push_back("X4HalfLength");
    tab.push_back("TiltAngleAlpha");
    storeTableColumnNames(tab);
    queryStr = std::format(
         "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real, {6} real, {7} real, {8} real, {9} real, {10} real, {11} real, {12} real, {13} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7], tab[8], tab[9], tab[10], tab[11], tab[12], tab[13] );
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-Pcon table
    geoNode = "GeoPcon";
    tableName = "Shapes_Pcon";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("SPhi");
    tab.push_back("DPhi");
    tab.push_back("NZPlanes");
    tab.push_back("dataStart");
    tab.push_back("dataEnd");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} integer, {6} integer, {7} integer )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-Pgon table
    geoNode = "GeoPgon";
    tableName = "Shapes_Pgon";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("SPhi");
    tab.push_back("DPhi");
    tab.push_back("NSides");
    tab.push_back("NZPlanes");
    tab.push_back("dataStart");
    tab.push_back("dataEnd");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} integer, {6} integer, {7} integer, {8} integer )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7], tab[8]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Shapes-SimplePolygonBrep table
    geoNode = "GeoSimplePolygonBrep";
    tableName = "Shapes_SimplePolygonBrep";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("DZ");
    tab.push_back("NVertices");
    tab.push_back("dataStart");
    tab.push_back("dataEnd");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} integer, {5} integer, {6} integer )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-SimplePolygonBrep table
    geoNode = "GeoGenericTrap";
    tableName = "Shapes_GenericTrap";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("ZHalfLength");
    tab.push_back("NVertices");
    tab.push_back("dataStart");
    tab.push_back("dataEnd");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} integer, {5} integer, {6} integer )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Shapes-Intersection table
    geoNode = "GeoShapeIntersection";
    tableName = "Shapes_Intersection";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("shapeAType");
    tab.push_back("shapeAId");
    tab.push_back("shapeBType");
    tab.push_back("shapeBId");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} varchar, {4} integer, {5} varchar, {6} integer )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6] );
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Shapes-Shift table
    geoNode = "GeoShapeShift";
    tableName = "Shapes_Shift";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("shapeType");
    tab.push_back("shapeId");
    tab.push_back("transformId");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} varchar, {4} integer, {5} integer )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5] );
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Shapes-Subtraction table
    geoNode = "GeoShapeSubtraction";
    tableName = "Shapes_Subtraction";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("shapeAType");
    tab.push_back("shapeAId");
    tab.push_back("shapeBType");
    tab.push_back("shapeBId");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} varchar, {4} integer, {5} varchar, {6} integer )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6] );
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Shapes-Union table
    geoNode = "GeoShapeUnion";
    tableName = "Shapes_Union";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("shapeAType");
    tab.push_back("shapeAId");
    tab.push_back("shapeBType");
    tab.push_back("shapeBId");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} varchar, {4} integer, {5} varchar, {6} integer )",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6] );
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Shapes-GeoUnidentifiedShape table
    // ID, XHalfLength, YHalfLength, ZHalfLength
    geoNode = "GeoUnidentifiedShape";
    tableName = "Shapes_UnidentifiedShape";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedVolume");
    tab.push_back("name");
    tab.push_back("asciiData");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} varchar, {4} varchar )",
        tab[0], tab[1], tab[2], tab[3], tab[4]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // create a table to store the numeric data used in GeoPcon shapes
    tableName = "Shapes_Pcon_Data";
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("ZPlane");
    tab.push_back("RMinPlane");
    tab.push_back("RMaxPlane");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4]);
    rc = execQuery(queryStr);
    tab.clear();
    
    // create a table to store the numeric data used in GeoPcon shapes
    tableName = "Shapes_Pgon_Data";
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("ZPlane");
    tab.push_back("RMinPlane");
    tab.push_back("RMaxPlane");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real )",
        tab[0], tab[1], tab[2], tab[3], tab[4]);
    rc = execQuery(queryStr);
    tab.clear();
    
    // create a table to store the numeric data used in GeoPcon shapes
    tableName = "Shapes_SimplePolygonBrep_Data";
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("XVertex");
    tab.push_back("YVertex");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real )",
        tab[0], tab[1], tab[2], tab[3]);
    rc = execQuery(queryStr);
    tab.clear();
    
    // create a table to store the numeric data used in GeoPcon shapes
    tableName = "Shapes_GenericTrap_Data";
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("XVertex");
    tab.push_back("YVertex");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real )",
        tab[0], tab[1], tab[2], tab[3]);
    rc = execQuery(queryStr);
    tab.clear();

    // SerialDenominators table
    geoNode = "GeoSerialDenominator";
    tableName = "SerialDenominators";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("baseName");
    storeTableColumnNames(tab);
    queryStr =
        std::format("create table {0}({1} integer primary key, {2} varchar)",
                    tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // SerialIdentifiers table
    geoNode = "GeoSerialIdentifier";
    tableName = "SerialIdentifiers";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("baseId");
    storeTableColumnNames(tab);
    queryStr =
        std::format("create table {0}({1} integer primary key, {2} integer)",
                    tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // IdentifierTags table
    geoNode = "GeoIdentifierTag";
    tableName = "IdentifierTags";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("identifier");
    storeTableColumnNames(tab);
    queryStr =
        std::format("create table {0}({1} integer primary key, {2} integer)",
                    tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Functions table
    geoNode = "Function";
    tableName = "Functions";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("expression");
    tab.push_back("dataStart");
    tab.push_back("dataEnd");
    storeTableColumnNames(tab);
    queryStr =
        std::format("create table {0}({1} integer primary key, {2} varchar, {3} integer, {4} integer)",
                    tab[0], tab[1], tab[2], tab[3], tab[4]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // SerialTransformers table
    geoNode = "GeoSerialTransformer";
    tableName = "SerialTransformers";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("funcId");
    tab.push_back("volId");
    tab.push_back("volTable");
    tab.push_back("copies");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} integer not null "
        "REFERENCES Functions(id), {3} integer not null, {4} integer not "
        "null "
        "REFERENCES GeoNodesTypes(id), {5} integer)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Transforms table
    geoNode = "GeoTransform";
    tableName = "Transforms";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("xx");
    tab.push_back("xy");
    tab.push_back("xz");
    tab.push_back("yx");
    tab.push_back("yy");
    tab.push_back("yz");
    tab.push_back("zx");
    tab.push_back("zy");
    tab.push_back("zz");
    tab.push_back("dx");
    tab.push_back("dy");
    tab.push_back("dz");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} "
        "real, {5} real, {6} real, {7} real, {8} real, {9} real, {10} "
        "real, "
        "{11} real, {12} real, {13} real)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7], tab[8],
        tab[9], tab[10], tab[11], tab[12], tab[13]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Transforms table
    geoNode = "GeoAlignableTransform";
    tableName = "AlignableTransforms";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("xx");
    tab.push_back("xy");
    tab.push_back("xz");
    tab.push_back("yx");
    tab.push_back("yy");
    tab.push_back("yz");
    tab.push_back("zx");
    tab.push_back("zy");
    tab.push_back("zz");
    tab.push_back("dx");
    tab.push_back("dy");
    tab.push_back("dz");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} "
        "real, {5} real, {6} real, {7} real, {8} real, {9} real, {10} "
        "real, "
        "{11} real, {12} real, {13} real)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7], tab[8],
        tab[9], tab[10], tab[11], tab[12], tab[13]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // NameTags table
    geoNode = "GeoNameTag";
    tableName = "NameTags";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("name");
    storeTableColumnNames(tab);
    queryStr =
        std::format("create table {0}({1} integer primary key, {2} varchar)",
                    tab[0], tab[1], tab[2]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
//----------------------------------------------------------------------------
    // VSurface table
    // ID = 33 in GeoNodesTypes, so that .db files introduced before Virtual Surface won't crush
    geoNode = "GeoVSurface";
    tableName = "VirtualSurface";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("shapeType");
    tab.push_back("shapeId");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} varchar not null, {3} integer not null)",
        tab[0], tab[1], tab[2], tab[3]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Surface-Rectangle table
    // ID = 34 in GeoNodesTypes
    // ID, XHalfLength, YHalfLength
    geoNode = "RectangleSurface";
    tableName = "Surface_Rectangle";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedArea");
    tab.push_back("XHalfLength");
    tab.push_back("YHalfLength");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real)",
        tab[0], tab[1], tab[2], tab[3], tab[4]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();
    
    // Surface-Trapezoid table
    // ID = 35 in GeoNodesTypes
    // ID, XHalfLengthMin, XHalfLengthMax, YHalfLength
    geoNode = "TrapezoidSurface";
    tableName = "Surface_Trapezoid";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedArea");
    tab.push_back("XHalfLengthMin");
    tab.push_back("XHalfLengthMax");
    tab.push_back("YHalfLength");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Surface-Annulus table
    // ID = 36 in GeoNodesTypes
    // ID, Ox, Oy, radius_in, radius_out, phi
    geoNode = "AnnulusSurface";
    tableName = "Surface_Annulus";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedArea");
    tab.push_back("Ox");
    tab.push_back("Oy");
    tab.push_back("radius_in");
    tab.push_back("radius_out");
    tab.push_back("phi");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real, {6} real, {7} real)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();

    // Surface-Diamond table
    // ID = 37 in GeoNodesTypes
    // ID, X_bottom_half, X_mid_half, X_top_half, Y_bottom_half, Y_top_half
    geoNode = "DiamondSurface";
    tableName = "Surface_Diamond";
    m_childType_tableName[geoNode] = tableName;
    tab.push_back(tableName);
    tab.push_back("id");
    tab.push_back("computedArea");
    tab.push_back("X_bottom_half");
    tab.push_back("X_mid_half");
    tab.push_back("X_top_half");
    tab.push_back("Y_bottom_half");
    tab.push_back("Y_top_half");
    storeTableColumnNames(tab);
    queryStr = std::format(
        "create table {0}({1} integer primary key, {2} real, {3} real, {4} real, {5} real, {6} real, {7} real)",
        tab[0], tab[1], tab[2], tab[3], tab[4], tab[5], tab[6], tab[7]);
    if (0 == (rc = execQuery(queryStr))) {
        storeNodeType(geoNode, tableName);
    }
    tab.clear();     

//----------------------------------------------------------------------------
    if (m_loglevel > 1) {
        std::cout << "All these tables have been successfully created:"
                  << std::endl;  // debug
        printAllDBTables();
    }

    if (rc == 0) {
        return true;
    }
    return false;
}

int GMDBManager::execQuery(const std::string& queryStr) {
    if (m_loglevel > 2)
        std::cout << "queryStr to execute: " << queryStr << std::endl;  // debug
    checkIsDBOpen();
    int result = -1;
    if ((result = sqlite3_exec(m_d->dbSqlite.ptr(), queryStr.c_str(), NULL, 0,
                               &m_d->m_SQLiteErrMsg))) {
        printf("[ERR] : \t> CMD: %s , Error: %d\n", queryStr.c_str(), result);
        if (m_d->m_SQLiteErrMsg) {
            printf("[ERR] : Error msg: %s\n", m_d->m_SQLiteErrMsg);
            sqlite3_free(m_d->m_SQLiteErrMsg);
        }
    }
    return result;
}

void GMDBManager::storeTableColumnNames(std::vector<std::string> input) {
    if (!(input.size() == 0)) {
        std::string tabName =
            input.at(0);  // get the first element storing the name of the table
        input.erase(input.begin());  // remove the first element storing the
                                     // name of the table
        m_tableNames[tabName] = input;
    }
}

std::vector<std::string> GMDBManager::getTableColumnNames(
    const std::string& tableName) {
    return m_tableNames.at(tableName);
}

bool GMDBManager::hasTableBeenCreatedInDB(const std::string_view tableName) {
    if (m_tableNames.count(std::string(tableName)) == 0) return false;
    return true;
}


void GMDBManager::storeNodeType(const std::string& nodeType, const std::string& tableName) {
    checkIsDBOpen();
    std::string queryStr;
    sqlite3_stmt* st = nullptr;
    int rc = -1;
    // preparing the SQL query
    std::string sql =
        "INSERT INTO GeoNodesTypes(nodeType, tableName) VALUES(?, ?)";
    rc = sqlite3_prepare_v2(m_d->dbSqlite.ptr(), sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        THROW_EXCEPTION("[SQLite ERR] (" << __func__ << ") : Error msg: " << m_d->dbSqlite.errmsg());
    }
    if (m_loglevel > 2)
        std::cout << "storeNodeType - Query string:" << sql
                  << std::endl;  // debug
    // bind the parameters
    rc = sqlite3_bind_text(st, 1, nodeType.c_str(), nodeType.length(),
                           SQLITE_TRANSIENT);
    rc = sqlite3_bind_text(st, 2, tableName.c_str(), tableName.length(),
                           SQLITE_TRANSIENT);
    // execute the query
    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) {
        THROW_EXCEPTION("[SQLite ERR] (" << __func__ << ") : Error msg: " << m_d->dbSqlite.errmsg());
    }
    // finalize
    sqlite3_finalize(st);
    return;
}







bool GMDBManager::storeRootVolume(const unsigned &id,
                                  const std::string_view nodeType) {
    checkIsDBOpen();

    std::string tableName = "RootVolume";
    const unsigned int typeId = getTableIdFromNodeType(nodeType);
    std::vector<std::string> cols = getTableColumnNames(tableName);

    // set the SQL query
    std::string queryStr;
    std::string sql = std::format("INSERT INTO {0} ({1}, {2}) VALUES (?, ?)",
                                  tableName, cols[1], cols[2]);
    // preparing the SQL query
    sqlite3_stmt* st = nullptr;
    int rc = -1;
    rc = sqlite3_prepare_v2(m_d->dbSqlite.ptr(), sql.c_str(), -1, &st, NULL);
    if (rc != SQLITE_OK) {
        THROW_EXCEPTION("[SQLite ERR] (" << __func__ << ") : Error msg: " << m_d->dbSqlite.errmsg());
    }
    if (m_loglevel > 2) std::cout << "Query string:" << sql << std::endl;  // debug
    // bind the parameters
    rc = sqlite3_bind_int(st, 1, id);
    rc = sqlite3_bind_int(st, 2, typeId);
    // execute the query
    rc = sqlite3_step(st);
    if (rc != SQLITE_DONE) {
        THROW_EXCEPTION("[SQLite ERR] (" << __func__ << ") : Error msg: " << m_d->dbSqlite.errmsg());
    }
    // finalize
    sqlite3_finalize(st);

    if (rc != 0) {
        return false;
    }
    return true;
}

// std::vector<std::string> GMDBManager::getRootPhysVol() {
std::pair<unsigned, unsigned> GMDBManager::getRootPhysVol() {
    // get the type and ID of the ROOT vol from the table "RootVolume"
    sqlite3_stmt* stmt = m_d->selectAllFromTable("RootVolume");
    // declare the data we want to fetch
    unsigned int id = 0;
    unsigned int typeId = 0;
    // execute the statement on all rows
    int rc = -1;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // get the data;
        // we skip the SQLite record ID stored in column 0, 
        // because we only have one Root volume
        id = sqlite3_column_int(stmt, 1);
        typeId = sqlite3_column_int(stmt, 2);
        // TODO: fill a cache
    }
    if (rc != SQLITE_DONE) {
        std::string errmsg(m_d->dbSqlite.errmsg());
        sqlite3_finalize(stmt);
        THROW_EXCEPTION(errmsg);
    }
    // finalize
    sqlite3_finalize(stmt);
    
    // std::vector<std::string> results = getItemAndType(typeId, id);
    return std::pair<unsigned, unsigned>{typeId, id};
    
}

std::string GMDBManager::getDBFilePath() { return m_dbpath; }

// FIXME: TODO: move to an utility class
int lastIndexOf(std::vector<std::string> v, const std::string& str, int pos = 0) {
    auto it = std::find(std::next(v.rbegin(), v.size() - pos), v.rend(), str);
    if (it != v.rend()) {
        auto idx = std::distance(v.begin(), it.base() - 1);
        //    std::cout << idx << std::endl;
        return idx;
    }
    return -1;  // no item matched
}

int GMDBManager::getTableColIndex(const std::string& tableName,
                                  const std::string& colName) {
    std::vector<std::string> colFields = m_tableNames.at(tableName);
    return lastIndexOf(colFields, colName);
}

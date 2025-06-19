// GeoModel includes
#include "GeoModelDBManager/GMDBManager.h"

#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <array>
#include <functional>
#include <filesystem>

#define GTEST_COUT std::cerr << "[ MESSAGE  ] "

class DatabaseTest : public ::testing::Test {
protected:
  std::string dbFile = "test_database.db";
  std::unique_ptr<GMDBManager> dbManager{};

  void SetUp() override {
    // Remove any existing test database
    if (std::filesystem::exists(dbFile)) {
        std::filesystem::remove(dbFile);
    }
    // open the DB connection
    dbManager = std::make_unique<GMDBManager>(dbFile);
    // check the DB connection
    if (dbManager->checkIsDBOpen()){
        std::cout << "OK! Database is open!" << std::endl;
    } else {
        std::cout << "Database ERROR!! Exiting..." << std::endl;
        exit(EXIT_FAILURE);
    }
  }

  void TearDown() override {
    if (std::filesystem::exists(dbFile)) {
        std::filesystem::remove(dbFile);
    }
  }
};

using MemberFunc = decltype(&GMDBManager::printAllPhysVols);
  constexpr std::array<MemberFunc,19> printFunctions{
    &GMDBManager::printAllPhysVols,
    &GMDBManager::printAllFullPhysVols,
    &GMDBManager::printAllLogVols,
    &GMDBManager::printAllMaterials,
    &GMDBManager::printAllElements,
    &GMDBManager::printAllShapes,
    &GMDBManager::printAllShapesData,
    &GMDBManager::printAllSerialDenominators,
    &GMDBManager::printAllSerialIdentifiers,
    &GMDBManager::printAllSerialTransformers,
    &GMDBManager::printAllFunctions,
    &GMDBManager::printAllTransforms,
    &GMDBManager::printAllAlignableTransforms,
    &GMDBManager::printAllNameTags,
    &GMDBManager::printAllChildrenPositions,
    &GMDBManager::printAllNodeTypes,
    &GMDBManager::printRootVolumeId,
    &GMDBManager::printDBVersion,
    &GMDBManager::printRootVolumeId
  };

void expectTextInOutput(GMDBManager* d, auto fn, const std::string & txt) {
  testing::internal::CaptureStdout();
  (d->*fn)();
  std::string output = testing::internal::GetCapturedStdout();
  EXPECT_TRUE((output.find(txt)!=std::string::npos));
}
void expectEmpty(GMDBManager* d, auto fn) {
  testing::internal::CaptureStdout();
  (d->*fn)();
  std::string output = testing::internal::GetCapturedStdout();
  GTEST_COUT<<output;
  EXPECT_TRUE((output.empty()));
}



TEST_F(DatabaseTest, CanCreateDatabase) {
    EXPECT_TRUE(std::filesystem::exists(dbFile));
}

TEST_F(DatabaseTest, PropertiesOfAnUninitialisedDatabase) {
  EXPECT_TRUE(dbManager->checkIsDBOpen());
  //
  testing::internal::CaptureStdout();
  EXPECT_NO_THROW(dbManager->printAllDBTables());
  std::string output = testing::internal::GetCapturedStdout();
  EXPECT_TRUE(output.empty()); //in contrast to subsequent tests, this is empty
  output.clear();
  // signature of an error in the output
  const std::string errTxt{"ERROR"};
  //
  //Parameterless 'print' functions called on an uninitialised database
  //do not throw but 
  //result simply in a cout message which contains the "ERROR" text
  auto expectErrorInOutput = [d = dbManager.get(), &errTxt] (auto fn){
    expectTextInOutput(d,fn, errTxt);
  };
  
  for (const auto & f:printFunctions) expectErrorInOutput(f);
  //'print' functions taking one parameter, defaulted to empty string
  testing::internal::CaptureStdout();
  EXPECT_NO_THROW(dbManager->printAllPublishedFullPhysVols());
  output = testing::internal::GetCapturedStdout();
  EXPECT_TRUE((output.find(errTxt)!=std::string::npos));
  output.clear();
  //
  testing::internal::CaptureStdout();
  EXPECT_NO_THROW(dbManager->printAllPublishedAlignableTransforms());
  output = testing::internal::GetCapturedStdout();
  EXPECT_TRUE((output.find(errTxt)!=std::string::npos));
  output.clear();
  //
  testing::internal::CaptureStdout();
  EXPECT_NO_THROW(dbManager->printAllRecords("Dummy"));//no tables exist anyway
  output = testing::internal::GetCapturedStdout();
  EXPECT_TRUE((output.find(errTxt)!=std::string::npos));
  output.clear();
  //
  EXPECT_NO_THROW(dbManager->createTableDataCaches());
  EXPECT_NO_THROW(dbManager->getAllDBTableColumns());
  EXPECT_EQ(dbManager->getDBFilePath(),"test_database.db");
  //the following throws, in contrast to most other methods
  EXPECT_THROW(dbManager->getRootPhysVol(), std::runtime_error);
}

TEST_F(DatabaseTest, CanInitialiseDatabaseWithDefaultTables) {
    EXPECT_TRUE(dbManager->initDB());
    testing::internal::CaptureStdout();
    dbManager->printAllDBTables();
    std::string output = testing::internal::GetCapturedStdout();
    //the following is a raw string literal; any change in formatting will change the actual comparison string!
    const std::string expectedResponse{R"(AlignableTransforms
ChildrenPositions
Elements
FullPhysVols
FuncExprData
Functions
GeoNodesTypes
IdentifierTags
LogVols
Materials
Materials_Data
NameTags
PhysVols
RootVolume
SerialDenominators
SerialIdentifiers
SerialTransformers
Shapes
Shapes_Box
Shapes_Cons
Shapes_EllipticalTube
Shapes_GenericTrap
Shapes_GenericTrap_Data
Shapes_Intersection
Shapes_Para
Shapes_Pcon
Shapes_Pcon_Data
Shapes_Pgon
Shapes_Pgon_Data
Shapes_Shift
Shapes_SimplePolygonBrep
Shapes_SimplePolygonBrep_Data
Shapes_Subtraction
Shapes_Torus
Shapes_Trap
Shapes_Trd
Shapes_Tube
Shapes_Tubs
Shapes_TwistedTrap
Shapes_UnidentifiedShape
Shapes_Union
Surface_Annulus
Surface_Diamond
Surface_Rectangle
Surface_Trapezoid
Transforms
VirtualSurface
dbversion
)"};
  EXPECT_EQ(output, expectedResponse);
  //example: table 'Shapes' exists
  EXPECT_TRUE(dbManager->checkTableFromDB("Shapes"));
}

TEST_F(DatabaseTest, PropertiesOfAnInitialisedButEmptyDatabase) {
  dbManager->initDB();
  //
  std::string output;
  // signature of an error in the output
  const std::string idTxt{"id"};
  const std::string errTxt{"ERROR"};
  //
  //Parameterless 'print' functions called on an initialised database
  //result in a cout message which contains the "id" text
  //and other parameters of the table
  auto expectIdInOutput = [d = dbManager.get(), &idTxt] (auto fn){
    expectTextInOutput(d,fn, idTxt);
  };
  
  for (const auto & f:printFunctions) expectIdInOutput(f);
  EXPECT_EQ(dbManager->getDBFilePath(),"test_database.db");
  EXPECT_NO_THROW(dbManager->getRootPhysVol());
  //
  //'print' functions taking one parameter, defaulted to empty string
  testing::internal::CaptureStdout();
  EXPECT_NO_THROW(dbManager->printAllPublishedFullPhysVols());
  output = testing::internal::GetCapturedStdout();
  //*** ERROR!! No table 'PublishedFullPhysVols' found! ***
  EXPECT_TRUE((output.find(errTxt)!=std::string::npos));
  output.clear();
  //
  testing::internal::CaptureStdout();
  EXPECT_NO_THROW(dbManager->printAllPublishedAlignableTransforms());
  output = testing::internal::GetCapturedStdout();
  //*** ERROR!! No table 'PublishedAlignableTransforms' found! ***
  EXPECT_TRUE((output.find(errTxt)!=std::string::npos));
  output.clear();
  //
  testing::internal::CaptureStdout();
  EXPECT_NO_THROW(dbManager->printAllRecords("Shapes_Para"));
  output = testing::internal::GetCapturedStdout();
  //id, computedVolume, XHalfLength, YHalfLength, ZHalfLength, Alpha, Theta, Phi
  GTEST_COUT<<output<<std::endl;
  EXPECT_TRUE((output.find(idTxt)!=std::string::npos));
  output.clear();
  //
  testing::internal::CaptureStdout();
  EXPECT_NO_THROW(dbManager->printAllRecords("Dummy"));//does not exist
  output = testing::internal::GetCapturedStdout();
  EXPECT_TRUE((output.find(errTxt)!=std::string::npos));
  output.clear();
  EXPECT_FALSE(dbManager->checkTableFromDB("Dummy"));
  EXPECT_TRUE(dbManager->checkTableFromDB("Shapes_Para"));
  EXPECT_FALSE(dbManager->checkTableFromCache("Shapes_Para"));
  //
  EXPECT_NO_THROW(dbManager->createTableDataCaches());
  EXPECT_TRUE(dbManager->checkTableFromCache("Shapes_Para"));
  EXPECT_FALSE(dbManager->checkTableFromCache("Dummy"));
  EXPECT_NO_THROW(dbManager->getAllDBTableColumns());
}



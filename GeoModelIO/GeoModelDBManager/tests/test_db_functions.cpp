// GeoModel includes
#include "GeoModelDBManager/GMDBManager.h"

#include "gtest/gtest.h"
#include <gtest/gtest.h>
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
        if (dbManager->checkIsDBOpen())
        {
            std::cout << "OK! Database is open!" << std::endl;
        }
        else
        {
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

TEST_F(DatabaseTest, CanCreateDatabase) {
    EXPECT_TRUE(std::filesystem::exists(dbFile));
}

TEST_F(DatabaseTest, PropertiesOfAnUnintialisedDatabase) {
  EXPECT_NO_THROW(dbManager->printAllDBTables());
  EXPECT_TRUE(dbManager->checkIsDBOpen());
  EXPECT_NO_THROW(dbManager->printAllPhysVols());
  EXPECT_NO_THROW(dbManager->printAllFullPhysVols());
  EXPECT_NO_THROW(dbManager->printAllLogVols());
  EXPECT_NO_THROW(dbManager->printAllMaterials());
  
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


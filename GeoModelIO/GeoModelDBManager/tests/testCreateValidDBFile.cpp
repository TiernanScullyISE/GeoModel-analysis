// GeoModel includes
#include "GeoModelDBManager/GMDBManager.h"

// GeoModel includes
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelIOHelpers/GMIO.h"


#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <array>
#include <functional>
#include <iostream>
#include <filesystem>

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits // so we will get, e.g., 'GeoModelKernelUnits::cm'
// Define the units
#define gr   SYSTEM_OF_UNITS::gram
#define mole SYSTEM_OF_UNITS::mole
#define cm3  SYSTEM_OF_UNITS::cm3
#define GTEST_COUT std::cerr << "[ MESSAGE  ] "
//

// Define the chemical elements
auto  Nitrogen = make_intrusive<GeoElement> ("Nitrogen" ,"N"  ,  7.0 ,  14.0067 *gr/mole);
auto  Oxygen   = make_intrusive<GeoElement> ("Oxygen"   ,"O"  ,  8.0 ,  15.9995 *gr/mole);
auto  Argon    = make_intrusive<GeoElement> ("Argon"    ,"Ar" , 18.0 ,  39.948  *gr/mole);
auto  Hydrogen =  make_intrusive<GeoElement> ("Hydrogen" ,"H"  ,  1.0 ,  1.00797 *gr/mole);
auto  Iron     =  make_intrusive<GeoElement> ("Iron"     ,"Fe" , 26.0 ,  55.847  *gr/mole);
auto  Carbon   =  make_intrusive<GeoElement> ("Carbon"   ,"C"  ,  6.0 ,  12.0107 *gr/mole);
auto  Silicon =  make_intrusive<GeoElement> ("Silicon"  ,"Si" , 14.0 ,  28.085  *gr/mole);

using namespace std::string_literals;

//base class purely to separate out the valid db creation functionality
class CreateDatabase : public ::testing::Test {
protected:
  std::string dbFile = "newValid_database.db";
  
  std::shared_ptr<GMDBManager> dbManager{};

  void SetUp() override {
   if (std::filesystem::exists(dbFile)) {
      std::filesystem::remove(dbFile);
    }
    // Define the materials

    double densityOfAir=0.001214 *gr/cm3;
    auto air = make_intrusive<GeoMaterial>("Air", densityOfAir);
    air->add(Nitrogen  , 0.7494);
    air->add(Oxygen, 0.2369);
    air->add(Argon, 0.0129);
    air->add(Hydrogen, 0.0008);
    air->lock();
  
    auto steel  = make_intrusive<GeoMaterial>("Steel", 7.9 *gr/cm3);
    steel->add(Iron  , 0.98);
    steel->add(Carbon, 0.02);
    steel->lock();
  
    // Silicon 100% (Detector)
    auto silicon = make_intrusive<GeoMaterial>("Silicon", 2.329 *gr/cm3);
    silicon->add(Silicon, 1.0);
    silicon->lock();
    //-----------------------------------------------------------------------------------//
    // create the world volume container and
    // get the 'world' volume, i.e. the root volume of the GeoModel tree
    GTEST_COUT<<  "Creating the 'world' volume, i.e. the root volume of the GeoModel tree...\n" ;
    auto  worldBox = make_intrusive<GeoBox>(1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm);
    auto  worldLog = make_intrusive<GeoLogVol>("WorldLog", worldBox, air);
    auto  world = make_intrusive<GeoPhysVol>(worldLog);
    //-----------------------------------------------------------------------------------//
    // Next make the box that describes the shape of the toy volume:                     //
    auto toyBox    = make_intrusive<GeoBox>(800*SYSTEM_OF_UNITS::cm, 800*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm);                   //
    // Bundle this with a material into a logical volume:                                //
    auto toyLog    = make_intrusive<GeoLogVol>("ToyLog", toyBox, silicon);                 //
    // ..And create a physical volume:                                                   //
    auto       toyPhys   = make_intrusive<GeoPhysVol>(toyLog);                               //
  
    auto sPass = make_intrusive<GeoBox>(5.0*SYSTEM_OF_UNITS::cm, 30*SYSTEM_OF_UNITS::cm, 30*SYSTEM_OF_UNITS::cm);
    auto lPass = make_intrusive<GeoLogVol>("Passive", sPass, steel);
    auto pPass = make_intrusive<GeoPhysVol>(lPass);
  
    auto sIPass = make_intrusive<GeoBox>(4*SYSTEM_OF_UNITS::cm, 25*SYSTEM_OF_UNITS::cm, 25*SYSTEM_OF_UNITS::cm);
    auto lIPass = make_intrusive<GeoLogVol>("InnerPassive", sIPass, air);
    auto pIPass = make_intrusive<GeoPhysVol>(lIPass);
  
    pPass->add(pIPass);
    toyPhys->add(pPass);
    // Now insert all of this into the world...                                           //
    auto tag = make_intrusive<GeoNameTag>("Toy");
    world->add(tag);
    world->add(toyPhys);
    //
    GTEST_COUT<<  "Dumping the GeoModel geometry to the DB file...\n" ;
    auto db = std::move(GeoModelIO::IO::saveToDB(world, dbFile, 0, false));
    GTEST_COUT<<  "DONE. Geometry saved.\n" ;
  }

  void TearDown() override {
    if (std::filesystem::exists(dbFile)) {
      std::filesystem::remove(dbFile);
    }
  }
};

//derived class connects to the pre-created database
class ValidDatabaseTest : public CreateDatabase {
protected:
  std::unique_ptr<GMDBManager> dbManager{};
  void SetUp() override final{
    // open the DB connection
    dbManager = std::make_unique<GMDBManager>(dbFile);
    // check the DB connection
    if (not dbManager->checkIsDBOpen()){
        std::cout << "Database ERROR!! Exiting..." << std::endl;
        exit(EXIT_FAILURE);
    }
  }

  void TearDown() override final{
    
  }
};


TEST_F(ValidDatabaseTest, CanCreateAndConnectValidDatabase) {
  GTEST_COUT<<std::filesystem::current_path()<<"\n";
  EXPECT_TRUE(std::filesystem::exists(dbFile));
  EXPECT_TRUE(dbManager->checkIsDBOpen());
}

TEST_F(ValidDatabaseTest, DISABLED_CanGetMaterials){
  testing::internal::CaptureStdout();
  EXPECT_NO_THROW(dbManager->printAllRecords("Materials"));
  std::string output  = testing::internal::GetCapturedStdout();
  //id, computedVolume, XHalfLength, YHalfLength, ZHalfLength, Alpha, Theta, Phi
  GTEST_COUT<<output<<std::endl;
  EXPECT_TRUE((output.find("Air")!=std::string::npos));
  output.clear();
}



// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/*
 * This example writes a sample geometry, reads it back in, then writes it again.
 * It is meant to test the GeoModel I/O packages and methods.
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: May, 2022
 *
 */

// GeoModel includes
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelWrite/WriteGeoModel.h"
#include "GeoModelRead/ReadGeoModel.h"

// C++ includes
#include <iostream>
#include <fstream>
#include <cstdlib> // EXIT_FAILURE


// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits // so we will get, e.g., 'GeoModelKernelUnits::cm'


int main(int argc, char *argv[])
{
  //-----------------------------------------------------------------------------------//
	// Define the materials that we shall use.                                              //
	// ----------------------------------------------------------------------------------//

	// Define the units
	#define gr   SYSTEM_OF_UNITS::gram
	#define mole SYSTEM_OF_UNITS::mole
	#define cm3  SYSTEM_OF_UNITS::cm3

	// Define the chemical elements
	GeoElement*  Nitrogen = new GeoElement ("Nitrogen" ,"N"  ,  7.0 ,  14.0067 *gr/mole);
	GeoElement*  Oxygen   = new GeoElement ("Oxygen"   ,"O"  ,  8.0 ,  15.9995 *gr/mole);
	GeoElement*  Argon    = new GeoElement ("Argon"    ,"Ar" , 18.0 ,  39.948  *gr/mole);
	GeoElement*  Hydrogen = new GeoElement ("Hydrogen" ,"H"  ,  1.0 ,  1.00797 *gr/mole);
	GeoElement*  Iron     = new GeoElement ("Iron"     ,"Fe" , 26.0 ,  55.847  *gr/mole);
	GeoElement*  Carbon   = new GeoElement ("Carbon"   ,"C"  ,  6.0 ,  12.0107 *gr/mole);
	GeoElement*  Sillicon = new GeoElement ("Silicon"  ,"Si" , 14.0 ,  28.085  *gr/mole);

	// Define the materials

	double densityOfAir=0.001214 *gr/cm3;
	GeoMaterial *air = new GeoMaterial("Air", densityOfAir);
	air->add(Nitrogen  , 0.7494);
	air->add(Oxygen, 0.2369);
	air->add(Argon, 0.0129);
	air->add(Hydrogen, 0.0008);
	air->lock();


  //-----------------------------------------------------------------------------------//
  // create the world volume container and
  // get the 'world' volume, i.e. the root volume of the GeoModel tree
  std::cout << "Creating the 'world' volume, i.e. the root volume of the GeoModel tree..." << std::endl;
  const GeoBox* worldBox = new GeoBox(1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm);
  const GeoLogVol* worldLog = new GeoLogVol("WorldLog", worldBox, air);
  GeoPhysVol* world = new GeoPhysVol(worldLog);


	  const double degree=M_PI/180.0;

  // Define elements used in this example:
  GeoElement  *aluminium     = new GeoElement("Aluminium", "Al", 13,  26 * gr/mole );

  // Define materials:
  // Define Aluminium
  double densityOfAluminium=2.7;                  // g/cm^3
  GeoMaterial *Aluminium     = new GeoMaterial("Aluminium", densityOfAluminium);
  Aluminium->add(aluminium,1.0);
  Aluminium->lock();

  // Some dimensions used below:
  double platformHeight=34.5;                             // Height to the top of the flanges
  //double flangeDiameter=3.375;                            // Diameter of the flanges
  double flangeThickness=3.0/16.0;                        // Thickness of the flanges
  double t1TubeLength = platformHeight-flangeThickness;   // Overall length of tube t1;
  double innerRadius=0.75/2.0;                            // 3/4 inch (inner diameter) pipe
  double outerRadius=(17.0/16.0)/2.0;                     // 1-1/16   (outer diameter) 
  double leftRightLegSeparation=61.0+11.0/16.0;           // Distance between legs, in x. 
  double frontBackLegSeparation=19.0+3.0/4.0;             // Distance between legs, in y.
  //const double barWidth1=4.0;                             // Width of front-back primary support
  //const double barWidth2=3.0;                             // Width of secondary aluminium support
  //double barThickness   =1.25;                            // Thickness of Aluminium bars
  //double cutoutDepth    = 21.5;                           // Depth of the cutout hole
  //double cutoutWidth    = 32.375;                         // Width of the cutout hole

  // Add the four legs of the kitchen sink:
  {
    const GeoTube      *t1Tube    = new GeoTube(innerRadius,outerRadius, t1TubeLength/2.0);
    const GeoLogVol    *t1Log     = new  GeoLogVol("T1Log", t1Tube, Aluminium);
    const GeoLogVol    *t2Log     = new  GeoLogVol("T2Log", t1Tube, Aluminium);
    GeoPhysVol         *t1Phys    = new GeoPhysVol(t1Log);
    GeoPhysVol         *t2Phys    = new GeoPhysVol(t2Log);
    //GeoPhysVol         *t3Phys    = new GeoPhysVol(t1Log);
    GeoTransform  *xform1         = new GeoTransform(GeoTrf::Translate3D(leftRightLegSeparation/2.0, frontBackLegSeparation/2.0, 0));
    GeoTransform  *xform2         = new GeoTransform(GeoTrf::Translate3D(leftRightLegSeparation/2.0, -frontBackLegSeparation/2.0, 0));
    GeoTransform  *xform3         = new GeoTransform(GeoTrf::Translate3D(-leftRightLegSeparation/2.0, -frontBackLegSeparation/2.0, 0));
    //GeoTransform  *xform4         = new GeoTransform(GeoTrf::Translate3D(-leftRightLegSeparation/2.0, frontBackLegSeparation/2.0, 0));
    world->add(xform1);
    
    world->add(t1Phys);
    
    world->add(xform2);
    world->add(t2Phys);
    
    world->add(xform3); 
    world->add(t2Phys);
    world->add(t2Phys);
    world->add(t2Phys);
  }






	//------------------------------------------------------------------------------------//
	// 1 -- Writing the geometry to file
	//------------------------------------------------------------------------------------//
  std::string path = "geometry_1.db";

	// check if DB file exists. If not, return.
  // FIXME: TODO: this check should go in the 'GMDBManager' constructor.
  std::ifstream infile(path.c_str());
    if ( infile.good() ) {
      std::cout << "\n\tERROR!! A '" << path << "' file exists already!! Please, remove, move, or rename it before running this program. Exiting...";
        exit(EXIT_FAILURE);
  }
  infile.close();

	// open the DB connection
  GMDBManager db(path);

  // check the DB connection
  if (db.checkIsDBOpen()) {
    std::cout << "OK! Database is open!" << std::endl;
  } else {
    std::cout << "Database ERROR!! Exiting..." << std::endl;
    exit(EXIT_FAILURE);
  }

  // Dump the tree volumes to a local file
	std::cout << "Dumping the GeoModel geometry to the DB file..." << std::endl;
  GeoModelIO::WriteGeoModel dumpGeoModelGraph(db); // init the GeoModel node action
  world->exec(&dumpGeoModelGraph); // visit all GeoModel nodes
  dumpGeoModelGraph.saveToDB(); // save to the SQlite DB file
  std::cout << "DONE. Geometry saved." <<std::endl;



  //std::cout << "\nTest - list of all the GeoMaterial nodes in the persistified geometry:" << std::endl;
  //db.printAllMaterials();
  //std::cout << "\nTest - list of all the GeoElement nodes in the persistified geometry:" << std::endl;
  //db.printAllElements();

	//------------------------------------------------------------------------------------//
	// 2 -- Reading back the geometry from file
	//------------------------------------------------------------------------------------//

  std::ifstream infile2(path.c_str());
    if ( ! infile2.good() ) {
      std::cout << "\n\tERROR!! A '" << path << "' file does not exist!! Please, check the path of the input file before running this program. Exiting...";
      exit(EXIT_FAILURE);
  }
  infile2.close();
  // open the DB
  auto db2 = std::make_unique<GMDBManager>(path);
  /* Open database */
  if (db2->checkIsDBOpen()) {
    std::cout << "OK! Database is open!\n";
  }
  else {
    std::cout << "Database is not open!\n";
    // return;
    throw;
  }
  /* setup the GeoModel reader */
  GeoModelIO::ReadGeoModel geoReader{db2.get()};
  std::cout << "OK! ReadGeoModel is set." << std::endl;
  /* build the GeoModel geometry */
  PVConstLink dbPhys = geoReader.buildGeoModel(); // builds the whole GeoModel tree in memory
  std::cout << "ReadGeoModel::buildGeoModel() done." << std::endl;


	//------------------------------------------------------------------------------------//
	// 3 -- Writing the geometry to file
	//------------------------------------------------------------------------------------//
  path = "geometry_2.db";

	// check if DB file exists. If not, return.
  // FIXME: TODO: this check should go in the 'GMDBManager' constructor.
  std::ifstream infile3(path.c_str());
    if ( infile3.good() ) {
      std::cout << "\n\tERROR!! A '" << path << "' file exists already!! Please, remove, move, or rename it before running this program. Exiting...";
        exit(EXIT_FAILURE);
  }
  infile3.close();

	// open the DB connection
  GMDBManager db3(path);

  // check the DB connection
  if (db.checkIsDBOpen()) {
    std::cout << "OK! Database is open!" << std::endl;
  } else {
    std::cout << "Database ERROR!! Exiting..." << std::endl;
    exit(EXIT_FAILURE);
  }

  // Dump the tree volumes to a local file
	std::cout << "Dumping the GeoModel geometry to the DB file..." << std::endl;
  GeoModelIO::WriteGeoModel dumpGeoModelGraph3(db3); // init the GeoModel node action
  //world->exec(&dumpGeoModelGraph3); // visit all GeoModel nodes
  dbPhys->exec(&dumpGeoModelGraph3); // visit all GeoModel nodes
  dumpGeoModelGraph3.saveToDB(); // save to the SQlite DB file
  std::cout << "DONE. Geometry saved." <<std::endl;


  return 0;
}

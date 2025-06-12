/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/*
* author: Riccardo Maria Bianchi @ CERN - 2020
*
* The example show how to write a more complex Toy detector geometry, 
* through the use of GeoSerialTransformers. 
* It also shows how to publish the list of FullPhysVol and AlignableTransform nodes,
* to be retrieved later (for example, when setting the readout geometry up).
* 
*/

#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoSerialDenominator.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoPublisher.h"
#include "GeoModelHelpers/TransformToStringConverter.h"

#include "GeoGenericFunctions/AbsFunction.h"
#include "GeoGenericFunctions/Variable.h"
#include "GeoGenericFunctions/Sin.h"
#include "GeoGenericFunctions/Cos.h"

#include "GeoModelDBManager/GMDBManager.h"

#include "GeoModelWrite/WriteGeoModel.h"
#include "GeoModelRead/ReadGeoModel.h"

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits // so we will get, e.g., 'SYSTEM_OF_UNITS::cm'

// C++ includes
#include <iostream>
#include <fstream>
#include <cstdlib> // EXIT_FAILURE

using namespace GeoGenfun;
using namespace GeoXF;

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

        // Define the materials

	// Air: Nitrogen + Oxygen + Argon + Hydrogen
        double densityOfAir=0.001214 *gr/cm3;
        GeoMaterial *air = new GeoMaterial("Air", densityOfAir);
        air->add(Nitrogen  , 0.7494);
        air->add(Oxygen, 0.2369);
        air->add(Argon, 0.0129);
        air->add(Hydrogen, 0.0008);
        air->lock();

	// Steel: Iron + Carbon
        GeoMaterial* steel  = new GeoMaterial("Steel", 7.9 *gr/cm3);
        steel->add(Iron  , 0.98);
        steel->add(Carbon, 0.02);
        steel->lock();

        /*
   //-----------------------------------------------------------------------------------//
  // create the world volume container and
  // get the 'world' volume, i.e. the root volume of the GeoModel tree
  std::cout << "Creating the 'world' volume, i.e. the root volume of the GeoModel tree..." << std::endl;
  const GeoBox* worldBox = new GeoBox(1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm);
  const GeoLogVol* worldLog = new GeoLogVol("WorldLog", worldBox, air);
  GeoPhysVol* world = new GeoPhysVol(worldLog);
*/
 
  //--------------------------------------//
  // Next make the box that describes
  // the shape of the toy volume:
  //--------------------------------------//
  const GeoBox *toyBox = new GeoBox(1200*SYSTEM_OF_UNITS::cm,1200*SYSTEM_OF_UNITS::cm, 1200*SYSTEM_OF_UNITS::cm);
  
  //--------------------------------------//
  // Bundle this with a material
  // into a logical volume:
  //--------------------------------------//
  const GeoLogVol *toyLog = new GeoLogVol("ToyLog", toyBox, air);                 //
  
  //--------------------------------------//
  // ..And create a physical volume:
  //--------------------------------------//
  GeoPhysVol *toyPhys = new GeoPhysVol(toyLog);
  
  //--------------------------------------//
  // Daughters
  //--------------------------------------//
  const GeoTube *ringTube  = new GeoTube(500*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm, 5.0*SYSTEM_OF_UNITS::cm);
  
  // Bundle this with a material //
  // into a logical volume:      //
  const GeoLogVol   *ringLog  = new  GeoLogVol("RingLog", ringTube, steel);
  
  // Make 100 of these              //
  // within the volume of the toy:  //
  GeoSerialDenominator *ringName = new GeoSerialDenominator("RING");
  toyPhys->add(ringName);
  
  // Before doing that, instanciate a GeoPublisher, to publish the list of FullPhysVol and AlignableTransforms nodes
  GeoPublisher* publisher = new GeoPublisher;
  // Optional - We set a name for the publisher: it will be appended to the name of the DB tables that host our published AXF and FPV nodes. 
  // Note : This is not compulsory: if not set, the default table name will be used; 
  //        however, it helps to keep the output data well organized.
  publisher->setName("HelloToyExample");

  for (int i=0;i<10;i++) {
    GeoFullPhysVol         *ringPhys = new GeoFullPhysVol(ringLog);
    GeoAlignableTransform  *xform    = new GeoAlignableTransform(GeoTrf::TranslateZ3D((i-50)*20*SYSTEM_OF_UNITS::cm));
    toyPhys->add(xform);
    toyPhys->add(ringPhys);

    // *** publish the list of FPV and AXF nodes ***
    // in this example, we use integer-based keys for FullPhysVols...
    unsigned int keyInt = i+1;
    publisher->publishNode<GeoVFullPhysVol*,unsigned>(ringPhys, keyInt);
    // ...and string-based keys for AlignableTransforms
    std::string keyStr = "HelloToy-AXF-" + std::to_string(i+1);
    publisher->publishNode<GeoAlignableTransform*,std::string>(xform, keyStr);
    
    std::cout << "step1 - FPV, key: " << keyInt 
              << " - xf: "<< GeoTrf::toString(ringPhys->getAbsoluteTransform());

  }


  //--------------------------------------//
  // Now insert all of this into the world...
  //--------------------------------------//
  //GeoNameTag *tag = new GeoNameTag("Toy");
  //world->add(tag);
  //world->add(toyPhys);
  //--------------------------------------//




   //------------------------------------------------------------------------------------//
   // Writing the geometry to file
   //------------------------------------------------------------------------------------//
  std::string path = "geometry.db";

        // check if DB file exists. If yes, delete it.
  // FIXME: TODO: this check should go in the 'GMDBManager' constructor.
  std::ifstream infile(path.c_str());
  if ( infile.good() ) {
      if( remove( path.c_str() ) != 0 )
              perror( "Error deleting file" );
        else
                puts( "File successfully deleted" );
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
  toyPhys->exec(&dumpGeoModelGraph); // visit all GeoModel nodes

  // Save the GeoModel tree to the SQlite DB file.
  // We pass a pointer to the GeoPublisher as well, so the list of published 
  // FullPhysVol and AlignableTransform nodes will be stored into the DB too.
  dumpGeoModelGraph.saveToDB( publisher );

  std::cout << "\n-----\nDONE. Geometry saved.\n-----\n" <<std::endl;


  //db.close(); // TODO: do we need a clse() method??

  // GET GEOMETRY FROM LOCAL DB


  // open the DB
  auto db2 = std::make_unique<GMDBManager>(path);
  /* Open database */
  if (db2->checkIsDBOpen()) {
    std::cout << "OK! Database is open!\n";
  }
  else {
    std::cout << "Database is not open!\n";
    // return;
    throw std::runtime_error("Database is not open.");
  }

  
  /* setup the GeoModel reader */
  GeoModelIO::ReadGeoModel readInGeo{db2.get()};
  std::cout << "OK! ReadGeoModel is set." << std::endl;


  /* build the GeoModel tree */
  PVConstLink world2 = readInGeo.buildGeoModel(); // builds the whole GeoModel tree in memory
  std::cout << "ReadGeoModel::buildGeoModel() done." << std::endl;


  //------------------------------------------------------------------------------------//
  // Testing the persitified geometry
  //------------------------------------------------------------------------------------//
/*
  std::cout << "\nTest - list of all the GeoFullPhysVol nodes in the persistified geometry:" << std::endl;
  db.printAllFullPhysVols();
  std::cout << "\nTest - list of all the GeoAlignableTransform nodes in the persistified geometry:" << std::endl;
  db.printAllAlignableTransforms();
  
  std::cout << "\nTest - list of all the 'published' GeoFullPhysVol nodes in the persistified geometry:" << std::endl;
  db.printAllPublishedFullPhysVols( publisher->getName() );
  std::cout << "\nTest - list of all the 'published' GeoAlignableTransform nodes in the persistified geometry:" << std::endl;
  db.printAllPublishedAlignableTransforms( publisher->getName() );
*/
 
  // --- testing the imported Geometry

  // get number of children volumes
  unsigned int nChil = world2->getNChildVols();
  std:: cout << "world's number of children: " << nChil << std::endl;

  // loop over all children nodes
  std::cout << "Looping over all 'volume' children (i.e., GeoPhysVol and GeoFullPhysVol)..." << std::endl;
  for (unsigned int idx=0; idx<nChil; ++idx) {
	  PVConstLink nodeLink = world2->getChildVol(idx);

	  if ( dynamic_cast<const GeoVPhysVol*>( &(*( nodeLink ))) ) {

		  std::cout << "\n\t" << "the child n. " << idx << " ";
		  const GeoVPhysVol *childVolV = &(*( nodeLink ));

		  if ( dynamic_cast<const GeoPhysVol*>(childVolV) ) {
			  const GeoPhysVol* childVol = dynamic_cast<const GeoPhysVol*>(childVolV);
			  std::cout << "is a GeoPhysVol, whose GeoLogVol's name is: " << childVol->getLogVol()->getName();
			  std::cout<< " and it has  "<<childVol->getNChildVols()<<" child volumes" << std::endl;
		  }
		  else if ( dynamic_cast<const GeoFullPhysVol*>(childVolV) ) {
			  const GeoFullPhysVol* childVol = dynamic_cast<const GeoFullPhysVol*>(childVolV);
			  std::cout << childVol << " is a GeoFullPhysVol, whose GeoLogVol's name is: " << childVol->getLogVol()->getName();
			  std::cout<< " and it has  "<<childVol->getNChildVols()<<" child volumes "
                  <<GeoTrf::toString(childVol->getAbsoluteTransform());
		  }
	  }
  }



  // cleaning
  delete publisher;
  publisher = nullptr;


  return 0;

}


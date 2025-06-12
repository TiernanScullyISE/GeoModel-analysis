/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
        GeoElement*  Sillicon = new GeoElement ("Silicon"  ,"Si" , 14.0 ,  28.085  *gr/mole);

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

        // Silicon 100% (Detector)
        GeoMaterial* silicon = new GeoMaterial("Silicon", 2.329 *gr/cm3);
        silicon->add(const_cast<GeoElement*> (Sillicon), 1.0);
        silicon->lock();

        // Carbon
        GeoMaterial* carbon = new GeoMaterial("Carbon", 2.329 *gr/cm3);
        carbon->add(const_cast<GeoElement*> (Carbon), 1.0);
        carbon->lock();

 
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
  const GeoLogVol   *ringLog  = new  GeoLogVol("RingLog", ringTube, carbon);
  
  // Make 100 of these              //
  // within the volume of the toy:  //
  GeoSerialDenominator *ringName = new GeoSerialDenominator("RING");
  toyPhys->add(ringName);
 
  // Instanciate a GeoPublisher, to publish the list of FullPhysVol and AlignableTransforms nodes
  GeoPublisher* publisher = new GeoPublisher;
  // Optional - We set a name for the publisher: it will be appended to the name of the DB tables that host our published AXF and FPV nodes. 
  // Note : This is not compulsory: if not set, the default table name will be used; 
  //        however, it helps to keep the output data well organized.
  publisher->setName("HelloToyExample");

  for (int i=0;i<100;i++) {
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
    

  }


  //--------------------------------------//
  //  Now, in addition to active daughters,
  // add some passive material.
  // This is done here using
  // the "SerialTransformer",
  // our way of parameterizing volumes.
  // It does not need to be done this way,
  // but we want to provide an example of
  // parametrizations in the Toy
  //--------------------------------------//

  GeoBox       *sPass = new GeoBox(5.0*SYSTEM_OF_UNITS::cm, 30*SYSTEM_OF_UNITS::cm, 30*SYSTEM_OF_UNITS::cm);
  GeoLogVol    *lPass = new GeoLogVol("Passive", sPass, steel);
  GeoPhysVol   *pPass = new GeoPhysVol(lPass);

  GeoBox       *sIPass = new GeoBox(4*SYSTEM_OF_UNITS::cm, 25*SYSTEM_OF_UNITS::cm, 25*SYSTEM_OF_UNITS::cm);
  GeoLogVol    *lIPass = new GeoLogVol("InnerPassive", sIPass, silicon);
  GeoPhysVol   *pIPass = new GeoPhysVol(lIPass);

  pPass->add(pIPass);

  const unsigned int NPLATES=100;
  Variable       i;
  Sin            sin;
  GENFUNCTION    f = 360*SYSTEM_OF_UNITS::deg/NPLATES*i;
  GENFUNCTION    g = sin(4*f);
  GENFUNCTION    h = -g;
  TRANSFUNCTION t1 = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateX3D(1100*SYSTEM_OF_UNITS::cm)*Pow(GeoTrf::TranslateZ3D(800*SYSTEM_OF_UNITS::cm),g);
  TRANSFUNCTION t2 = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateX3D(1100*SYSTEM_OF_UNITS::cm)*Pow(GeoTrf::TranslateZ3D(800*SYSTEM_OF_UNITS::cm),h);

  //--------------------------------------//
  // Inside, by the way, the serial transformer
  // will evaluate the functions:
  // HepTransform3D xf = t1(i), for i=1,NPLATES....
  //--------------------------------------//

  GeoSerialDenominator  *pass1Name = new GeoSerialDenominator("PASSIVE-1-");
  GeoSerialTransformer *s1 = new GeoSerialTransformer(pPass,&t1, NPLATES);
  toyPhys->add(pass1Name);
  toyPhys->add(s1);

  GeoSerialDenominator *pass2Name = new GeoSerialDenominator("PASSIVE-2-");
  GeoSerialTransformer *s2 = new GeoSerialTransformer(pPass,&t2, NPLATES);
  toyPhys->add(pass2Name);
  toyPhys->add(s2);



   //------------------------------------------------------------------------------------//
   // Writing the geometry to file
   //------------------------------------------------------------------------------------//
  std::string path = "geometry.db";

  // check if DB file exists. If yes, delete it.
  std::ifstream infile(path.c_str());
  if ( infile.good() ) {
      if( remove( path.c_str() ) != 0 )
          perror( "Error deleting file" );
      else {
          std::string msg = "Previously existing " + path + " successfully deleted"; 
          puts( msg.c_str() );
      }
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
  // cleaning
  delete publisher;
  publisher = nullptr;


  return 0;

}


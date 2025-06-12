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
#include "GeoModelKernel/GeoEllipticalTube.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoPara.h"
#include "GeoModelKernel/GeoTrap.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoTorus.h"
#include "GeoModelKernel/GeoTwistedTrap.h"
#include "GeoModelKernel/GeoSimplePolygonBrep.h"
#include "GeoModelKernel/GeoGenericTrap.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelKernel/GeoUnidentifiedShape.h"

#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeIntersection.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoShapeUnion.h"

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

#include "GeoModelKernel/throwExcept.h"


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
    
    std::cout << "step1 - FPV, key: " << keyInt << std::endl;

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
  // GeoTube       *sPass = new GeoTube(5.0*SYSTEM_OF_UNITS::cm, 10*SYSTEM_OF_UNITS::cm, 30*SYSTEM_OF_UNITS::cm);
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



  // === Add single test instances for all shapes ===

  // Add a test GeoBox shape
  GeoBox *sBox = new GeoBox(5.0 * SYSTEM_OF_UNITS::cm, 30 * SYSTEM_OF_UNITS::cm, 30 * SYSTEM_OF_UNITS::cm);
  GeoLogVol *lBox = new GeoLogVol("box", sBox, steel);
  GeoPhysVol *pBox = new GeoPhysVol(lBox);
  GeoNameTag *nBox = new GeoNameTag("Shape-Box");
  toyPhys->add(nBox);
  toyPhys->add(pBox);

  // Add a test GeoEllipticalTube shape
  GeoEllipticalTube *sEllipticalTube = new GeoEllipticalTube(5.0 * SYSTEM_OF_UNITS::cm, 30 * SYSTEM_OF_UNITS::cm, 30 * SYSTEM_OF_UNITS::cm);
  GeoLogVol *lEllipticalTube = new GeoLogVol("ellipticaltube", sEllipticalTube, steel);
  GeoPhysVol *pEllipticalTube = new GeoPhysVol(lEllipticalTube);
  GeoNameTag *nEllipticalTube = new GeoNameTag("Shape-EllipticalTube");
  toyPhys->add(nEllipticalTube);
  toyPhys->add(pEllipticalTube);
  
  // Add a test GeoTube shape
  GeoTube *sTube = new GeoTube(5.0 * SYSTEM_OF_UNITS::cm, 10 * SYSTEM_OF_UNITS::cm, 30 * SYSTEM_OF_UNITS::cm);
  GeoLogVol *lTube = new GeoLogVol("tube", sTube, steel);
  GeoPhysVol *pTube = new GeoPhysVol(lTube);
  GeoNameTag *nTube = new GeoNameTag("Shape-Tube");
  toyPhys->add(nTube);
  toyPhys->add(pTube);

  // Add a test GeoPcon shape
  GeoPcon *sPcon = new GeoPcon(0, 360 * SYSTEM_OF_UNITS::deg);
  sPcon->addPlane(1 * SYSTEM_OF_UNITS::m, 1, 5);
  sPcon->addPlane(2 * SYSTEM_OF_UNITS::m, 2, 3);
  GeoLogVol *lPcon = new GeoLogVol("pcon", sPcon, steel);
  GeoPhysVol *pPcon = new GeoPhysVol(lPcon);
  GeoNameTag *nPcon = new GeoNameTag("Shape-Pcon");
  toyPhys->add(nPcon);
  toyPhys->add(pPcon);

  // Add a test GeoCons shape
  double rMin1 = 1 * SYSTEM_OF_UNITS::m;
  double rMin2 = 1.5 * SYSTEM_OF_UNITS::m;
  double rMax1 = 3 * SYSTEM_OF_UNITS::m;
  double rMax2 = 3.5 * SYSTEM_OF_UNITS::m;
  double zmin = 1 * SYSTEM_OF_UNITS::m;
  double zmax = 10 * SYSTEM_OF_UNITS::m;
  double dZnew = 0.5 * (zmax - zmin);
  GeoCons *sCons = new GeoCons(rMin1, rMin2,
                               rMax1, rMax2,
                               dZnew,
                               0 * SYSTEM_OF_UNITS::deg, 360 * SYSTEM_OF_UNITS::deg);
  GeoLogVol *lCons = new GeoLogVol("cons", sCons, steel);
  GeoPhysVol *pCons = new GeoPhysVol(lCons);
  GeoNameTag *nCons = new GeoNameTag("Shape-Cons");
  toyPhys->add(nCons);
  toyPhys->add(pCons);

  // Add a test GeoPara shape
  double Rmax = 335. * SYSTEM_OF_UNITS::mm;
  double bepo_Beta = 4.668 * SYSTEM_OF_UNITS::degree; 
  double Zall = (912. / 2.) * SYSTEM_OF_UNITS::mm;
  double Xall = (171. / 2.) * SYSTEM_OF_UNITS::mm;
  double Yall = (300. / 2.) * SYSTEM_OF_UNITS::mm;
  double Rmax_1 = Rmax - 2. * Zall * tan(bepo_Beta);
  GeoPara *sPara = new GeoPara(Zall, Yall, Xall, 0 * SYSTEM_OF_UNITS::degree, bepo_Beta, 0. * SYSTEM_OF_UNITS::degree);
  GeoLogVol *lPara = new GeoLogVol("para", sPara, steel);
  GeoPhysVol *pPara = new GeoPhysVol(lPara);
  GeoNameTag *nPara = new GeoNameTag("Shape-Para");
  toyPhys->add(nPara);
  toyPhys->add(pPara);

  // Add a test GeoPara shape
  GeoPgon *sPgon = new GeoPgon(0., 360 * SYSTEM_OF_UNITS::degree, 6);
  sPgon->addPlane(2 * SYSTEM_OF_UNITS::m, 0.2*SYSTEM_OF_UNITS::m, 0.5 * SYSTEM_OF_UNITS::m);
  sPgon->addPlane(3 * SYSTEM_OF_UNITS::m, 0, 0.5 * SYSTEM_OF_UNITS::m);
  GeoLogVol *lPgon = new GeoLogVol("pgon", sPgon, steel);
  GeoPhysVol *pPgon = new GeoPhysVol(lPgon);
  GeoNameTag *nPgon = new GeoNameTag("Shape-Pgon");
  toyPhys->add(nPgon);
  toyPhys->add(pPgon);

  // Add a test GeoTrap shape
  double length = 5 * SYSTEM_OF_UNITS::m;
  double halflength = 0.5 * length;
  double rmax = 2 * SYSTEM_OF_UNITS::m;
  double rmin = 1 * SYSTEM_OF_UNITS::m;
  double phiWidth = 30 * SYSTEM_OF_UNITS::degree;
  double thickness = 0.5 * (rmax - rmin);
  double averad = 0.5 * (rmin + rmax);
  double w1 = 0.5 * phiWidth * rmin / averad;
  double w2 = 0.5 * phiWidth * rmax / averad;
  GeoTrap *sTrap = new GeoTrap(halflength, 0, 0, thickness, w1, w2, 0, thickness, w1, w2, 0);
  GeoLogVol *lTrap = new GeoLogVol("trap", sTrap, steel);
  GeoPhysVol *pTrap = new GeoPhysVol(lTrap);
  GeoNameTag *nTrap = new GeoNameTag("Shape-Trap");
  toyPhys->add(nTrap);
  toyPhys->add(pTrap);
  
  // Add a test GeoTrd shape
  GeoTrd *sTrd = new GeoTrd(2*SYSTEM_OF_UNITS::m, 2*SYSTEM_OF_UNITS::m, 1*SYSTEM_OF_UNITS::m, 1*SYSTEM_OF_UNITS::m, 1*SYSTEM_OF_UNITS::m);
  GeoLogVol *lTrd = new GeoLogVol("trd", sTrd, steel);
  GeoPhysVol *pTrd = new GeoPhysVol(lTrd);
  GeoNameTag *nTrd = new GeoNameTag("Shape-Trd");
  toyPhys->add(nTrd);
  toyPhys->add(pTrd);

  // Add a test GeoTubs shape
  GeoTubs *sTubs = new GeoTubs(1 * SYSTEM_OF_UNITS::m, 2 * SYSTEM_OF_UNITS::m, 4 * SYSTEM_OF_UNITS::m, 0 * SYSTEM_OF_UNITS::degree, 270 * SYSTEM_OF_UNITS::degree);
  GeoLogVol *lTubs = new GeoLogVol("tubs", sTubs, steel);
  GeoPhysVol *pTubs = new GeoPhysVol(lTubs);
  GeoNameTag *nTubs = new GeoNameTag("Shape-Tubs");
  toyPhys->add(nTubs);
  toyPhys->add(pTubs);

  // Add a test GeoTwistedTrap shape
  double tw_PhiTwist = 30 * SYSTEM_OF_UNITS::deg; // twist angle
  double tw_Dz = 5 * SYSTEM_OF_UNITS::m;          // half z length
  double tw_Theta = 30 * SYSTEM_OF_UNITS::deg;    // direction between end planes
  double tw_Phi = 30 * SYSTEM_OF_UNITS::deg;      // defined by polar and azim. angles
  double tw_Dy1 = 1 * SYSTEM_OF_UNITS::m;         // half y length at -pDz
  double tw_Dx1 = 1 * SYSTEM_OF_UNITS::m;         // half x length at -pDz,-pDy
  double tw_Dx2 = 1 * SYSTEM_OF_UNITS::m;         // half x length at -pDz,+pDy
  double tw_Dy2 = 1 * SYSTEM_OF_UNITS::m;         // half y length at +pDz
  double tw_Dx3 = 1 * SYSTEM_OF_UNITS::m;         // half x length at +pDz,-pDy
  double tw_Dx4 = 1 * SYSTEM_OF_UNITS::m;         // half x length at +pDz,+pDy
  double tw_Alph = 30 * SYSTEM_OF_UNITS::deg;     // tilt angle
  GeoTwistedTrap *sTwist = new GeoTwistedTrap(tw_PhiTwist, tw_Dz, tw_Theta, tw_Phi,
                                              tw_Dy1, tw_Dx1, tw_Dx2, tw_Dy2, tw_Dx3, tw_Dx4, tw_Alph);
  GeoLogVol *lTwist = new GeoLogVol("twistedtrap", sTwist, steel);
  GeoPhysVol *pTwist = new GeoPhysVol(lTwist);
  GeoNameTag *nTwist = new GeoNameTag("Shape-TwistedTrap");
  toyPhys->add(nTwist);
  toyPhys->add(pTwist);

  // Add a test GeoSimplePolygonBrep shape
   const double DZ = 1 * SYSTEM_OF_UNITS::m;
   const double xV1 = 1.5 * SYSTEM_OF_UNITS::m;
   const double yV1 = 1.5 * SYSTEM_OF_UNITS::m;
   const double xV2 = 3 * SYSTEM_OF_UNITS::m;
   const double yV2 = 3 * SYSTEM_OF_UNITS::m;
   const double xV3 = 4 * SYSTEM_OF_UNITS::m;
   const double yV3 = 4 * SYSTEM_OF_UNITS::m;
   GeoSimplePolygonBrep* sSimplePolygonBrep = new GeoSimplePolygonBrep(DZ);
   sSimplePolygonBrep->addVertex(xV1, yV1);
   sSimplePolygonBrep->addVertex(xV2, yV2);
   sSimplePolygonBrep->addVertex(xV3, yV3);
   if (!sSimplePolygonBrep->isValid())
    {
        THROW_EXCEPTION("ERROR! GeoSimplePolygonBrep shape is not valid!!");
    }
  GeoLogVol *lSimplePolygonBrep = new GeoLogVol("SimplePolygonBrep", sSimplePolygonBrep, steel);
  GeoPhysVol *pSimplePolygonBrep = new GeoPhysVol(lSimplePolygonBrep);
  GeoNameTag *nSimplePolygonBrep = new GeoNameTag("Shape-SimplePolygonBrep");
  toyPhys->add(nSimplePolygonBrep);
  toyPhys->add(pSimplePolygonBrep);

  // Add a test *shared* GeoSimplePolygonBrep shape
  GeoLogVol *lSimplePolygonBrep2 = new GeoLogVol("SimplePolygonBrepShared", sSimplePolygonBrep, steel);
  GeoPhysVol *pSimplePolygonBrep2 = new GeoPhysVol(lSimplePolygonBrep2);
  GeoNameTag *nSimplePolygonBrep2 = new GeoNameTag("Shape-SimplePolygonBrepShared");
  toyPhys->add(nSimplePolygonBrep2);
  toyPhys->add(pSimplePolygonBrep2);

  // Add a test GeoShift boolean shape:
  // a shift of a box
  GeoShapeShift* sShift = new GeoShapeShift(sPass, GeoTrf::TranslateZ3D(50*SYSTEM_OF_UNITS::cm));
  GeoLogVol *lShift = new GeoLogVol("Shift", sShift, steel);
  GeoPhysVol *pShift = new GeoPhysVol(lShift);
  GeoNameTag *nShift = new GeoNameTag("Shape-Shift");
  toyPhys->add(nShift);
  toyPhys->add(pShift);

  // Add a test GeoIntersection boolean shape
  GeoShapeIntersection* sIntersection = new GeoShapeIntersection(sPass, sIPass);
  GeoLogVol *lIntersection = new GeoLogVol("Intersection", sIntersection, steel);
  GeoPhysVol *pIntersection = new GeoPhysVol(lIntersection);
  GeoNameTag *nIntersection = new GeoNameTag("Shape-Intersection");
  toyPhys->add(nIntersection);
  toyPhys->add(pIntersection);

// Add a test GeoSubtraction boolean shape
  GeoShapeSubtraction* sSubtraction = new GeoShapeSubtraction(sPass, sIPass);
  GeoLogVol *lSubtraction = new GeoLogVol("Subtraction", sSubtraction, steel);
  GeoPhysVol *pSubtraction = new GeoPhysVol(lSubtraction);
  GeoNameTag *nSubtraction = new GeoNameTag("Shape-Subtraction");
  toyPhys->add(nSubtraction);
  toyPhys->add(pSubtraction);

// Add a test GeoSubtraction boolean shape
  GeoShapeUnion* sUnion = new GeoShapeUnion(sPass, sIPass);
  GeoLogVol *lUnion = new GeoLogVol("Union", sUnion, steel);
  GeoPhysVol *pUnion = new GeoPhysVol(lUnion);
  GeoNameTag *nUnion = new GeoNameTag("Shape-Union");
  toyPhys->add(nUnion);
  toyPhys->add(pUnion);


// Add a test chain GeoShift operator shape:
// a shift of a shift of a box
  GeoShapeShift* sShift2 = new GeoShapeShift(sShift, GeoTrf::TranslateZ3D(50*SYSTEM_OF_UNITS::cm));
  GeoLogVol *lShift2 = new GeoLogVol("Shift2", sShift2, steel);
  GeoPhysVol *pShift2 = new GeoPhysVol(lShift2);
  GeoNameTag *nShift2 = new GeoNameTag("Shape-Shift-2");
  toyPhys->add(nShift2);
  toyPhys->add(pShift2);

// Add a test chain & mixed GeoShift boolean shape:
// a shift of a union of two boxes
  GeoShapeShift* sShiftUnion = new GeoShapeShift(sUnion, GeoTrf::TranslateZ3D(50*SYSTEM_OF_UNITS::cm));
  GeoLogVol *lShiftUnion = new GeoLogVol("Shift-Union", sShiftUnion, steel);
  GeoPhysVol *pShiftUnion = new GeoPhysVol(lShiftUnion);
  GeoNameTag *nShiftUnion = new GeoNameTag("Shape-Shift-Union");
  toyPhys->add(nShiftUnion);
  toyPhys->add(pShiftUnion);

// Add a test chain & mixed GeoShift boolean shape:
// a shift of a union of a subtraction of two boxes and an intersection of two boxes
  GeoShapeUnion* sUnionSubInt = new GeoShapeUnion(sSubtraction, sIntersection);
  GeoShapeShift* sShiftUnionSubInt = new GeoShapeShift(sUnionSubInt, GeoTrf::TranslateZ3D(50*SYSTEM_OF_UNITS::cm));
  GeoLogVol *lShiftUnionSubInt = new GeoLogVol("Shift-Union-Subtraction-Intersection", sShiftUnionSubInt, steel);
  GeoPhysVol *pShiftUnionSubInt = new GeoPhysVol(lShiftUnionSubInt);
  GeoNameTag *nShiftUnionSubInt = new GeoNameTag("Shape-Shift-Union-Subtraction_Intersection");
  toyPhys->add(nShiftUnionSubInt);
  toyPhys->add(pShiftUnionSubInt);



  // Add a test "UnidentifiedShape" shape node
  const std::string shapeUnidName = "LArCustomShape";
  const std::string shapeUnidAscii = "LAr::Example";
  GeoUnidentifiedShape* sUnidentifiedShape = new GeoUnidentifiedShape(shapeUnidName, shapeUnidAscii);
  const GeoLogVol* lUnidentifiedShape = new GeoLogVol("UnidentifiedShape", sUnidentifiedShape, steel);
  GeoPhysVol *pUnidentifiedShape = new GeoPhysVol(lUnidentifiedShape);
  GeoNameTag *nUnidentifiedShape = new GeoNameTag("UnidentifiedShape");
  toyPhys->add(nUnidentifiedShape);
  toyPhys->add(pUnidentifiedShape);


  // Add a test "Torus" shape node
  const GeoTorus* sTorus = new GeoTorus(2*SYSTEM_OF_UNITS::m, 1*SYSTEM_OF_UNITS::m, 10*SYSTEM_OF_UNITS::m, 0*SYSTEM_OF_UNITS::rad, 4.5*SYSTEM_OF_UNITS::rad);
  const GeoLogVol* lTorus = new GeoLogVol("Torus", sTorus, steel);
  GeoPhysVol* pTorus = new GeoPhysVol(lTorus);
  GeoNameTag* nTorus = new GeoNameTag("Shape-Torus");
  toyPhys->add(nTorus);
  toyPhys->add(pTorus);

  // Add a test GeoGenericTrap shape
  const double gt_Zlength = 1 * SYSTEM_OF_UNITS::m;
  const double gt_xV1 = 1.5 * SYSTEM_OF_UNITS::m;
  const double gt_yV1 = 1.5 * SYSTEM_OF_UNITS::m;
  const double gt_xV2 = 3 * SYSTEM_OF_UNITS::m;
  const double gt_yV2 = 3 * SYSTEM_OF_UNITS::m;
  const double gt_xV3 = 4 * SYSTEM_OF_UNITS::m;
  const double gt_yV3 = 4 * SYSTEM_OF_UNITS::m;
  GeoGenericTrapVertices gt_Vertices;
  gt_Vertices.push_back(GeoTwoVector(gt_xV1, gt_yV1));
  gt_Vertices.push_back(GeoTwoVector(gt_xV2, gt_yV2));
  gt_Vertices.push_back(GeoTwoVector(gt_xV3, gt_yV3));
  GeoGenericTrap *sGenericTrap = new GeoGenericTrap(gt_Zlength, gt_Vertices);
  // if (!sGenericTrap->isValid())
  // {
  //   THROW_EXCEPTION("ERROR! GeoGenericTrap shape is not valid!!");
  // }
  GeoLogVol *lGenericTrap = new GeoLogVol("GeoGenericTrap", sGenericTrap, steel);
  GeoPhysVol *pGenericTrap = new GeoPhysVol(lGenericTrap);
  GeoNameTag *nGenericTrap = new GeoNameTag("Shape-GenericTrap");
  toyPhys->add(nGenericTrap);
  toyPhys->add(pGenericTrap);

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


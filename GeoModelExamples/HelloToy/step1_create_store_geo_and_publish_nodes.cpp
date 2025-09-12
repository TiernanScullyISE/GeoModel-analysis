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
#include "GeoModelKernel/GeoIntrusivePtr.h"
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
#define SYSTEM_OF_UNITS GeoModelKernelUnits
#define gr   SYSTEM_OF_UNITS::gram
#define mole SYSTEM_OF_UNITS::mole
#define cm3  SYSTEM_OF_UNITS::cm3
#define mm   SYSTEM_OF_UNITS::mm
#define cm   SYSTEM_OF_UNITS::cm
#define m    SYSTEM_OF_UNITS::m
#define deg  SYSTEM_OF_UNITS::deg

template<class T>
using GeoNodePtr = GeoIntrusivePtr<T>;

// C++ includes
#include <array>
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

  // Define the chemical elements
  GeoNodePtr<GeoElement>  Nitrogen {new GeoElement ("Nitrogen" ,"N"  ,  7.0 ,  14.0067 *gr/mole)};
  GeoNodePtr<GeoElement>  Oxygen   {new GeoElement ("Oxygen"   ,"O"  ,  8.0 ,  15.9995 *gr/mole)};
  GeoNodePtr<GeoElement>  Argon    {new GeoElement ("Argon"    ,"Ar" , 18.0 ,  39.948  *gr/mole)};
  GeoNodePtr<GeoElement>  Hydrogen {new GeoElement ("Hydrogen" ,"H"  ,  1.0 ,  1.00797 *gr/mole)};
  GeoNodePtr<GeoElement>  Iron     {new GeoElement ("Iron"     ,"Fe" , 26.0 ,  55.847  *gr/mole)};        
  GeoNodePtr<GeoElement>  Carbon   {new GeoElement ("Carbon"   ,"C"  ,  6.0 ,  12.0107 *gr/mole)};        
  GeoNodePtr<GeoElement>  Sillicon {new GeoElement ("Silicon"  ,"Si" , 14.0 ,  28.085  *gr/mole)};

  // Define the materials

  // Air: Nitrogen + Oxygen + Argon + Hydrogen
  double densityOfAir=0.001214 *gr/cm3;
  GeoNodePtr<GeoMaterial> air{new GeoMaterial("Air", densityOfAir)};
  air->add(Nitrogen , 0.7494);
  air->add(Oxygen   , 0.2369);
  air->add(Argon    , 0.0129);
  air->add(Hydrogen , 0.0008);
  air->lock();

  // Steel: Iron + Carbon
  GeoNodePtr<GeoMaterial> steel{new GeoMaterial("Steel", 7.9 *gr/cm3)};
  steel->add(Iron   , 0.98);
  steel->add(Carbon , 0.02);
  steel->lock();

  // Silicon 100% (Detector)
  GeoNodePtr<GeoMaterial> silicon{new GeoMaterial("Silicon", 2.329 *gr/cm3)};
  silicon->add(Sillicon, 1.0);
  silicon->lock();

  // Carbon
  GeoNodePtr<GeoMaterial> carbon{new GeoMaterial("Carbon", 2.329 *gr/cm3)};
  carbon->add(Carbon, 1.0);
  carbon->lock();
 
  //--------------------------------------//
  // Next make the box that describes
  // the shape of the toy volume:
  //--------------------------------------//
  GeoNodePtr<GeoBox> toyBox {new GeoBox(1200*cm,1200*cm, 1200*cm)};
  
  //--------------------------------------//
  // Bundle this with a material
  // into a logical volume:
  //--------------------------------------//
  GeoNodePtr<GeoLogVol> toyLog {new GeoLogVol("ToyLog", toyBox, air)};
  
  //--------------------------------------//
  // ..And create a physical volume:
  //--------------------------------------//
  GeoNodePtr<GeoPhysVol> toyPhys {new GeoPhysVol(toyLog)};
  
  // Introduce an intermediate daughter volume that will serve as a container for
  // Full Physical Volumes and Passive Materials (serial transformers)
  // Such a hierarchy should make it easier to visualise the persistent geometry
  // from the database file using gmex
  GeoNodePtr<GeoTube>    containerTube {new GeoTube(400*cm, 1500*cm, 1100.0*cm)};
  GeoNodePtr<GeoLogVol>  containerLog  {new GeoLogVol("ContainerLog", containerTube, air)};
  GeoNodePtr<GeoPhysVol> containerPhys {new GeoPhysVol(containerLog)};
  GeoNodePtr<GeoNameTag> containerName {new GeoNameTag("ToyGeometry")};
  toyPhys->add(containerName);
  toyPhys->add(containerPhys);

  //--------------------------------------//
  // Daughters
  //--------------------------------------//
  GeoNodePtr<GeoTube>   ringTube {new GeoTube(500*cm, 1000*cm, 5.0*cm)};
  
  // Bundle this with a material //
  // into a logical volume:      //
  GeoNodePtr<GeoLogVol> ringLog  {new GeoLogVol("RingLog", ringTube, carbon)};
  
  // Make 100 of these              //
  // within the volume of the toy:  //
  GeoNodePtr<GeoSerialDenominator> ringName {new GeoSerialDenominator("RING")};
  containerPhys->add(ringName);
 
  // Instanciate a GeoPublisher, to publish the list of FullPhysVol and AlignableTransforms nodes
  GeoPublisher* publisher = new GeoPublisher;
  // Optional - We set a name for the publisher: it will be appended to the name of the DB tables that host our published AXF and FPV nodes. 
  // Note : This is not compulsory: if not set, the default table name will be used; 
  //        however, it helps to keep the output data well organized.
  publisher->setName("HelloToyExample");

  for (int i=0;i<100;i++) {
    GeoNodePtr<GeoFullPhysVol>        ringPhys {new GeoFullPhysVol(ringLog)};
    GeoNodePtr<GeoAlignableTransform> xform    {new GeoAlignableTransform(GeoTrf::TranslateZ3D((i-50)*20*cm))};
    containerPhys->add(xform);
    containerPhys->add(ringPhys);
    

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

  GeoNodePtr<GeoBox>     sPass {new GeoBox(5.0*cm, 30*cm, 30*cm)};
  GeoNodePtr<GeoLogVol>  lPass {new GeoLogVol("Passive", sPass, steel)};
  GeoNodePtr<GeoPhysVol> pPass {new GeoPhysVol(lPass)};

  GeoNodePtr<GeoBox>     sIPass {new GeoBox(4*cm, 25*cm, 25*cm)};
  GeoNodePtr<GeoLogVol>  lIPass {new GeoLogVol("InnerPassive", sIPass, silicon)};
  GeoNodePtr<GeoPhysVol> pIPass {new GeoPhysVol(lIPass)};

  pPass->add(pIPass);

  const unsigned int NPLATES=100;
  Variable       i;
  Sin            sin;
  GENFUNCTION    f = 360*deg/NPLATES*i;
  GENFUNCTION    g = sin(4*f);
  GENFUNCTION    h = -g;
  TRANSFUNCTION t1 = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateX3D(1100*cm)*Pow(GeoTrf::TranslateZ3D(800*cm),g);
  TRANSFUNCTION t2 = Pow(GeoTrf::RotateZ3D(1.0),f)*GeoTrf::TranslateX3D(1100*cm)*Pow(GeoTrf::TranslateZ3D(800*cm),h);

  //--------------------------------------//
  // Inside, by the way, the serial transformer
  // will evaluate the functions:
  // HepTransform3D xf = t1(i), for i=1,NPLATES....
  //--------------------------------------//

  GeoNodePtr<GeoSerialDenominator> pass1Name {new GeoSerialDenominator("PASSIVE-1-")};
  GeoNodePtr<GeoSerialTransformer> s1        {new GeoSerialTransformer(pPass,&t1, NPLATES)};
  containerPhys->add(pass1Name);
  containerPhys->add(s1);

  GeoNodePtr<GeoSerialDenominator> pass2Name {new GeoSerialDenominator("PASSIVE-2-")};
  GeoNodePtr<GeoSerialTransformer> s2        {new GeoSerialTransformer(pPass,&t2, NPLATES)};
  containerPhys->add(pass2Name);
  containerPhys->add(s2);

  // === Add single test instances for all shapes ===

  // Add a test GeoBox shape
  GeoNodePtr<GeoBox>     sBox {new GeoBox(500.0 * cm, 600.0 * cm, 800.0 * cm)};
  GeoNodePtr<GeoLogVol>  lBox {new GeoLogVol("box", sBox, steel)};
  GeoNodePtr<GeoPhysVol> pBox {new GeoPhysVol(lBox)};
  GeoNodePtr<GeoNameTag> nBox {new GeoNameTag("Shape-Box")};
  toyPhys->add(nBox);
  toyPhys->add(pBox);

  // Add a test GeoTube shape
  GeoNodePtr<GeoTube>    sTube {new GeoTube(500.0 * cm, 1000.0 * cm, 1000.0 * cm)};
  GeoNodePtr<GeoLogVol>  lTube {new GeoLogVol("tube", sTube, steel)};
  GeoNodePtr<GeoPhysVol> pTube {new GeoPhysVol(lTube)};
  GeoNodePtr<GeoNameTag> nTube {new GeoNameTag("Shape-Tube")};
  toyPhys->add(nTube);
  toyPhys->add(pTube);

  // Add a test GeoPcon shape
  GeoNodePtr<GeoPcon> sPcon {new GeoPcon(0, 360 * deg)};
  sPcon->addPlane(-1000.0 * cm, 200.0 * cm,  500.0 * cm);
  sPcon->addPlane(      0 * cm, 500.0 * cm, 1000.0 * cm);
  sPcon->addPlane( 1000.0 * cm, 500.0 * cm,  600.0 * cm);
  GeoNodePtr<GeoLogVol>  lPcon {new GeoLogVol("pcon", sPcon, steel)};
  GeoNodePtr<GeoPhysVol> pPcon {new GeoPhysVol(lPcon)};
  GeoNodePtr<GeoNameTag> nPcon {new GeoNameTag("Shape-Pcon")};
  toyPhys->add(nPcon);
  toyPhys->add(pPcon);

  // Add a test GeoCons shape
  double rMin1 = 100.0 * cm;
  double rMin2 = 200.0 * cm;
  double rMax1 = 500.0 * cm;
  double rMax2 = 900.0 * cm;
  double zmin = 200.0 * cm;
  double zmax = 2000.0 * cm;
  double dZnew = 0.5 * (zmax - zmin);
  GeoNodePtr<GeoCons> sCons {new GeoCons(rMin1, rMin2,
    rMax1, rMax2, dZnew, 0 * deg, 270 * deg)};
  GeoNodePtr<GeoLogVol>  lCons {new GeoLogVol("cons", sCons, steel)};
  GeoNodePtr<GeoPhysVol> pCons {new GeoPhysVol(lCons)};
  GeoNodePtr<GeoNameTag> nCons {new GeoNameTag("Shape-Cons")};
  toyPhys->add(nCons);
  toyPhys->add(pCons);

  // Add a test GeoPara shape
  double Rmax = 335. * cm;
  double bepo_Beta = 4.668 * deg; 
  double Zall = (912. / 2.) * cm;
  double Xall = (171. / 2.) * cm;
  double Yall = (300. / 2.) * cm;
  double Rmax_1 = Rmax - 2. * Zall * tan(bepo_Beta);
  GeoNodePtr<GeoPara>    sPara {new GeoPara(Zall, Yall, Xall, 30 * deg, bepo_Beta, 0. * deg)};
  GeoNodePtr<GeoLogVol>  lPara {new GeoLogVol("para", sPara, steel)};
  GeoNodePtr<GeoPhysVol> pPara {new GeoPhysVol(lPara)};
  GeoNodePtr<GeoNameTag> nPara {new GeoNameTag("Shape-Para")};
  toyPhys->add(nPara);
  toyPhys->add(pPara);

  // Add a test GeoPgon shape
  GeoNodePtr<GeoPgon> sPgon {new GeoPgon(0., 360 * deg, 6)};
  sPgon->addPlane(-1000.0 * cm,   0.0 * cm, 700.0 * cm);
  sPgon->addPlane(    0.0 * cm, 200.0 * cm, 500.0 * cm);
  sPgon->addPlane( 1000.0 * cm, 200.0 * cm, 500.0 * cm);
  GeoNodePtr<GeoLogVol>  lPgon {new GeoLogVol("pgon", sPgon, steel)};
  GeoNodePtr<GeoPhysVol> pPgon {new GeoPhysVol(lPgon)};
  GeoNodePtr<GeoNameTag> nPgon {new GeoNameTag("Shape-Pgon")};
  toyPhys->add(nPgon);
  toyPhys->add(pPgon);

  // Add a test GeoTrap shape
  double length = 1000 * cm;
  double halflength = 0.5 * length;
  double rmax = 1200 * cm;
  double rmin = 600 * cm;
  double phiWidth = 30 * deg;
  double thickness = 0.5 * (rmax - rmin);
  double averad = 0.5 * (rmin + rmax);
  double w1 = 1500. * phiWidth * rmin / averad;
  double w2 = 1500. * phiWidth * rmax / averad;
  GeoNodePtr<GeoTrap>    sTrap {new GeoTrap(halflength, 0, 0, thickness, w1, w2, 0, thickness, w1, w2, 0)};
  GeoNodePtr<GeoLogVol>  lTrap {new GeoLogVol("trap", sTrap, steel)};
  GeoNodePtr<GeoPhysVol> pTrap {new GeoPhysVol(lTrap)};
  GeoNodePtr<GeoNameTag> nTrap {new GeoNameTag("Shape-Trap")};
  toyPhys->add(nTrap);
  toyPhys->add(pTrap);
  
  // Add a test GeoTrd shape
  GeoNodePtr<GeoTrd>     sTrd {new GeoTrd(800.0*cm, 200.0*cm, 800.0*cm, 200.0*cm, 400.0*cm)};
  GeoNodePtr<GeoLogVol>  lTrd {new GeoLogVol("trd", sTrd, steel)};
  GeoNodePtr<GeoPhysVol> pTrd {new GeoPhysVol(lTrd)};
  GeoNodePtr<GeoNameTag> nTrd {new GeoNameTag("Shape-Trd")};
  toyPhys->add(nTrd);
  toyPhys->add(pTrd);

  // Add a test GeoTubs shape
  GeoNodePtr<GeoTubs>    sTubs {new GeoTubs(500.0*cm, 600.0*cm, 800.0*cm, 0*deg, 270*deg)};
  GeoNodePtr<GeoLogVol>  lTubs {new GeoLogVol("tubs", sTubs, steel)};
  GeoNodePtr<GeoPhysVol> pTubs {new GeoPhysVol(lTubs)};
  GeoNodePtr<GeoNameTag> nTubs {new GeoNameTag("Shape-Tubs")};
  toyPhys->add(nTubs);
  toyPhys->add(pTubs);

  // Add a test GeoTwistedTrap shape
  double tw_PhiTwist = 30 * deg; // twist angle
  double tw_Dz = 5 * m;          // half z length
  double tw_Theta = 30 * deg;    // direction between end planes
  double tw_Phi = 30 * deg;      // defined by polar and azim. angles
  double tw_Dy1 = 1 * m;         // half y length at -pDz
  double tw_Dx1 = 1 * m;         // half x length at -pDz,-pDy
  double tw_Dx2 = 1 * m;         // half x length at -pDz,+pDy
  double tw_Dy2 = 1 * m;         // half y length at +pDz
  double tw_Dx3 = 1 * m;         // half x length at +pDz,-pDy
  double tw_Dx4 = 1 * m;         // half x length at +pDz,+pDy
  double tw_Alph = 30 * deg;     // tilt angle
  GeoNodePtr<GeoTwistedTrap> sTwist {new GeoTwistedTrap(tw_PhiTwist, tw_Dz
    , tw_Theta, tw_Phi
    , tw_Dy1, tw_Dx1
    , tw_Dx2, tw_Dy2
    , tw_Dx3, tw_Dx4
    , tw_Alph)};
  GeoNodePtr<GeoLogVol>  lTwist {new GeoLogVol("twistedtrap", sTwist, steel)};
  GeoNodePtr<GeoPhysVol> pTwist {new GeoPhysVol(lTwist)};
  GeoNodePtr<GeoNameTag> nTwist {new GeoNameTag("Shape-TwistedTrap")};
  toyPhys->add(nTwist);
  toyPhys->add(pTwist);

  // Add a test GeoSimplePolygonBrep shape
  const double DZ = 300 * cm;
  constexpr unsigned brepSize = 5;
  std::array<double, brepSize> xV{500*cm, 900*cm, 800*cm, 200*cm, 100*cm};
  std::array<double, brepSize> yV{200*cm, 500*cm, 800*cm, 700*cm, 100*cm};
  GeoNodePtr<GeoSimplePolygonBrep> sSimplePolygonBrep {new GeoSimplePolygonBrep(DZ)};
  for (unsigned ind{0}; ind<brepSize; ++ind) {
    sSimplePolygonBrep->addVertex(xV[ind],yV[ind]); 
  }
  if (!sSimplePolygonBrep->isValid()) {
    THROW_EXCEPTION("ERROR! GeoSimplePolygonBrep shape is not valid!!");
  }
  GeoNodePtr<GeoLogVol>  lSimplePolygonBrep {new GeoLogVol("SimplePolygonBrep", sSimplePolygonBrep, steel)};
  GeoNodePtr<GeoPhysVol> pSimplePolygonBrep {new GeoPhysVol(lSimplePolygonBrep)};
  GeoNodePtr<GeoNameTag> nSimplePolygonBrep {new GeoNameTag("Shape-SimplePolygonBrep")};
  toyPhys->add(nSimplePolygonBrep);
  toyPhys->add(pSimplePolygonBrep);

  // Add a test *shared* GeoSimplePolygonBrep shape
  GeoNodePtr<GeoLogVol>  lSimplePolygonBrep2 {new GeoLogVol("SimplePolygonBrepShared", sSimplePolygonBrep, steel)};
  GeoNodePtr<GeoPhysVol> pSimplePolygonBrep2 {new GeoPhysVol(lSimplePolygonBrep2)};
  GeoNodePtr<GeoNameTag> nSimplePolygonBrep2 {new GeoNameTag("Shape-SimplePolygonBrepShared")};
  toyPhys->add(nSimplePolygonBrep2);
  toyPhys->add(pSimplePolygonBrep2);

  // Add a test "Torus" shape node
  GeoNodePtr<GeoTorus>   sTorus {new GeoTorus(100*cm , 200*cm , 500*cm , 0*deg,  270*deg)};
  GeoNodePtr<GeoLogVol>  lTorus {new GeoLogVol("Torus", sTorus, steel)};
  GeoNodePtr<GeoPhysVol> pTorus {new GeoPhysVol(lTorus)};
  GeoNodePtr<GeoNameTag> nTorus {new GeoNameTag("Shape-Torus")};
  toyPhys->add(nTorus);
  toyPhys->add(pTorus);

  // ------------------------------- BOOLEAN SHAPES -------------------------------------------------

  // Add a test GeoShapeShift boolean shape:
  GeoNodePtr<GeoShapeShift> sShift {new GeoShapeShift(sBox, GeoTrf::TranslateZ3D(1000*cm))};
  GeoNodePtr<GeoLogVol>     lShift {new GeoLogVol("Shift", sShift, steel)};
  GeoNodePtr<GeoPhysVol>    pShift {new GeoPhysVol(lShift)};
  GeoNodePtr<GeoNameTag>    nShift {new GeoNameTag("Shape-Shift")};
  toyPhys->add(nShift);
  toyPhys->add(pShift);

  // Add a test GeoShapeIntersection boolean shape
  GeoNodePtr<GeoShapeIntersection> sIntersection {new GeoShapeIntersection(sBox, sTube)};
  GeoNodePtr<GeoLogVol>            lIntersection {new GeoLogVol("Intersection", sIntersection, steel)};
  GeoNodePtr<GeoPhysVol>           pIntersection {new GeoPhysVol(lIntersection)};
  GeoNodePtr<GeoNameTag>           nIntersection {new GeoNameTag("Shape-Intersection")};
  toyPhys->add(nIntersection);
  toyPhys->add(pIntersection);

  // Add a test GeoShapeSubtraction boolean shape
  GeoNodePtr<GeoShapeSubtraction> sSubtraction {new GeoShapeSubtraction(sBox, sTube)};
  GeoNodePtr<GeoLogVol>           lSubtraction {new GeoLogVol("Subtraction", sSubtraction, steel)};
  GeoNodePtr<GeoPhysVol>          pSubtraction {new GeoPhysVol(lSubtraction)};
  GeoNodePtr<GeoNameTag>          nSubtraction {new GeoNameTag("Shape-Subtraction")};
  toyPhys->add(nSubtraction);
  toyPhys->add(pSubtraction);

  // Add a test GeoShapeUnion boolean shape
  GeoNodePtr<GeoShapeUnion> sUnion {new GeoShapeUnion(sPara, sTrap)};
  GeoNodePtr<GeoLogVol>     lUnion {new GeoLogVol("Union", sUnion, steel)};
  GeoNodePtr<GeoPhysVol>    pUnion {new GeoPhysVol(lUnion)};
  GeoNodePtr<GeoNameTag>    nUnion {new GeoNameTag("Shape-Union")};
  toyPhys->add(nUnion);
  toyPhys->add(pUnion);

  // Add a test chain GeoShift operator shape:
  GeoNodePtr<GeoShapeShift> sShift2 {new GeoShapeShift(sShift, GeoTrf::TranslateY3D(500*cm))};
  GeoNodePtr<GeoLogVol>     lShift2 {new GeoLogVol("Shift2", sShift2, steel)};
  GeoNodePtr<GeoPhysVol>    pShift2 {new GeoPhysVol(lShift2)};
  GeoNodePtr<GeoNameTag>    nShift2 {new GeoNameTag("Shape-Shift-2")};
  toyPhys->add(nShift2);
  toyPhys->add(pShift2);

  // Add a test chain & mixed GeoShift boolean shape:
  GeoNodePtr<GeoShapeShift> sShiftUnion {new GeoShapeShift(sUnion, GeoTrf::TranslateX3D(500*cm))};
  GeoNodePtr<GeoLogVol>     lShiftUnion {new GeoLogVol("Shift-Union", sShiftUnion, steel)};
  GeoNodePtr<GeoPhysVol>    pShiftUnion {new GeoPhysVol(lShiftUnion)};
  GeoNodePtr<GeoNameTag>    nShiftUnion {new GeoNameTag("Shape-Shift-Union")};
  toyPhys->add(nShiftUnion);
  toyPhys->add(pShiftUnion);

  // Add a test chain & mixed GeoShift boolean shape:
  // a shift of a union of a subtraction of two boxes and an intersection of two boxes
  GeoNodePtr<GeoShapeUnion> sUnionSubInt      {new GeoShapeUnion(sSubtraction, sIntersection)};
  GeoNodePtr<GeoShapeShift> sShiftUnionSubInt {new GeoShapeShift(sUnionSubInt, GeoTrf::TranslateX3D(1000*cm))};
  GeoNodePtr<GeoLogVol>     lShiftUnionSubInt {new GeoLogVol("Shift-Union-Subtraction-Intersection", sShiftUnionSubInt, steel)};
  GeoNodePtr<GeoPhysVol>    pShiftUnionSubInt {new GeoPhysVol(lShiftUnionSubInt)};
  GeoNodePtr<GeoNameTag>    nShiftUnionSubInt {new GeoNameTag("Shape-Shift-Union-Subtraction_Intersection")};
  toyPhys->add(nShiftUnionSubInt);
  toyPhys->add(pShiftUnionSubInt);

  /*
    ................ Problematic Shapes ...................

  // Add a test GeoEllipticalTube shape
  GeoEllipticalTube *sEllipticalTube = new GeoEllipticalTube(5.0 * cm, 30 * cm, 30 * cm);
  GeoLogVol *lEllipticalTube = new GeoLogVol("ellipticaltube", sEllipticalTube, steel);
  GeoPhysVol *pEllipticalTube = new GeoPhysVol(lEllipticalTube);
  GeoNameTag *nEllipticalTube = new GeoNameTag("Shape-EllipticalTube");
  toyPhys->add(nEllipticalTube);
  toyPhys->add(pEllipticalTube);

  // Add a test "UnidentifiedShape" shape node
  const std::string shapeUnidName = "LArCustomShape";
  const std::string shapeUnidAscii = "LAr::Example";
  GeoUnidentifiedShape* sUnidentifiedShape = new GeoUnidentifiedShape(shapeUnidName, shapeUnidAscii);
  const GeoLogVol* lUnidentifiedShape = new GeoLogVol("UnidentifiedShape", sUnidentifiedShape, steel);
  GeoPhysVol *pUnidentifiedShape = new GeoPhysVol(lUnidentifiedShape);
  GeoNameTag *nUnidentifiedShape = new GeoNameTag("UnidentifiedShape");
  toyPhys->add(nUnidentifiedShape);
  toyPhys->add(pUnidentifiedShape);

  // Add a test GeoGenericTrap shape
  const double gt_Zlength = 300 * cm;
  GeoGenericTrapVertices gt_Vertices;
  gt_Vertices.reserve(3);
  for (unsigned ind{0}; ind<3; ++ind) {
    gt_Vertices.push_back(GeoTwoVector(xV[ind],yV[ind])); 
  }
  GeoGenericTrap *sGenericTrap = new GeoGenericTrap(gt_Zlength, gt_Vertices);
  GeoLogVol *lGenericTrap = new GeoLogVol("GeoGenericTrap", sGenericTrap, steel);
  GeoPhysVol *pGenericTrap = new GeoPhysVol(lGenericTrap);
  GeoNameTag *nGenericTrap = new GeoNameTag("Shape-GenericTrap");
  toyPhys->add(nGenericTrap);
  toyPhys->add(pGenericTrap);
  */

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


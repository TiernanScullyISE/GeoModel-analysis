// Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

/*
 * main.cpp
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: Apr, 2019
 *
 */

// GeoModel includes
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"

// C++ includes
#include <iostream>

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits // so we will get, e.g., 'GeoModelKernelUnits::cm'

int main(int argc, char *argv[])
{
  	//-----------------------------------------------------------------------------------//
	// Get the materials that we shall use.                                              //
	// ----------------------------------------------------------------------------------//

	// Bogus densities.  Later: read from database.
	double densityOfAir=0.1, densityOfPolystyrene=0.2;
	GeoIntrusivePtr<const GeoMaterial> air{new GeoMaterial("Air Two",densityOfAir)};
	GeoIntrusivePtr<const GeoMaterial> poly{new GeoMaterial("std::Polystyrene",densityOfPolystyrene)};

	//-----------------------------------------------------------------------------------//
	// create the world volume container and
	// get the 'world' volume, i.e. the root volume of the GeoModel tree
	std::cout << "Creating the 'world' volume, i.e. the root volume of the GeoModel tree..." << std::endl;
	GeoIntrusivePtr<const GeoMaterial> worldMat{new GeoMaterial("std::Air", densityOfAir)};
	GeoIntrusivePtr<const GeoBox> worldBox{new GeoBox(1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm)};
	GeoIntrusivePtr<const GeoLogVol> worldLog{new GeoLogVol("WorldLog", worldBox, worldMat)};
	GeoIntrusivePtr<GeoPhysVol> world{new GeoPhysVol(worldLog)};

	//-----------------------------------------------------------------------------------//
	// Next make the box that describes the shape of the toy volume:                     //
	GeoIntrusivePtr<const GeoBox>      toyBox{new GeoBox(800*SYSTEM_OF_UNITS::cm         //
		, 800*SYSTEM_OF_UNITS::cm                                                        //
		, 1000*SYSTEM_OF_UNITS::cm)};                                                    //
	// Bundle this with a material into a logical volume:                                //
	GeoIntrusivePtr<const GeoLogVol>   toyLog{new GeoLogVol("ToyLog", toyBox, air)};     //
	// ..And create a physical volume:                                                   //
	GeoIntrusivePtr<GeoPhysVol>        toyPhys{new GeoPhysVol(toyLog)};                  //

	GeoIntrusivePtr<GeoBox>       sPass{new GeoBox(5.0*SYSTEM_OF_UNITS::cm
		, 30*SYSTEM_OF_UNITS::cm
		, 30*SYSTEM_OF_UNITS::cm)};
	GeoIntrusivePtr<GeoLogVol>    lPass{new GeoLogVol("Passive", sPass, poly)};
	GeoIntrusivePtr<GeoPhysVol>   pPass{new GeoPhysVol(lPass)};

	GeoIntrusivePtr<GeoBox>       sIPass{new GeoBox(4*SYSTEM_OF_UNITS::cm
		, 25*SYSTEM_OF_UNITS::cm
		, 25*SYSTEM_OF_UNITS::cm)};
	GeoIntrusivePtr<GeoLogVol>    lIPass{new GeoLogVol("InnerPassive", sIPass, air)};
	GeoIntrusivePtr<GeoPhysVol>   pIPass{new GeoPhysVol(lIPass)};

	pPass->add(pIPass);
	toyPhys->add(pPass);

	//------------------------------------------------------------------------------------//
	// Now insert all of this into the world...                                           //
	GeoIntrusivePtr<GeoNameTag> tag{new GeoNameTag("Toy")};                               //
	world->add(tag);                                                                      //
	world->add(toyPhys);                                                                  //
	//------------------------------------------------------------------------------------//

	// --- Now, we test the newly created Geometry

	// get the 'world' GeoLogVol
	std::cout << "\nGetting the GeoLogVol used by the 'world' volume..." << std::endl;
	GeoIntrusivePtr<const GeoLogVol> logVol = world->getLogVol();
	std::cout << "'world' GeoLogVol name: " << logVol->getName() << std::endl;
	std::cout << "'world' GeoMaterial name: " << logVol->getMaterial()->getName() << std::endl;
	
	// get number of children volumes
	unsigned int nChil = world->getNChildVols();
	std:: cout << "'world' number of children: " << nChil << std::endl;
	
	// loop over all child nodes
	std::cout << "Looping over all world's 'volume' children (i.e., GeoPhysVol and GeoFullPhysVol)..." << std::endl;
	for (unsigned int idx=0; idx<nChil; ++idx) {
		PVConstLink nodeLink = world->getChildVol(idx);
		const GeoVPhysVol *childVolV = nodeLink.get();
		bool isPV = dynamic_cast<const GeoPhysVol*>(childVolV)!=nullptr;

		std::cout << "\t" << "the child n. " << idx << " is a ";
		std::cout << ( isPV ? "GeoPhysVol" : "GeoFullPhysVol" );
		std::cout << ", whose GeoLogVol name is: " << nodeLink->getLogVol()->getName();
		std::cout << " and it has  " << nodeLink->getNChildVols() << " child volumes" << std::endl;
	}
	std::cout << "Done." << std::endl;
	return 0;
}

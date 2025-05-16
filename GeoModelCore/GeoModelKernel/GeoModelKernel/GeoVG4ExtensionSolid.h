/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOVG4EXTENSIONSOLID_H
#define GEOMODELKERNEL_GEOVG4EXTENSIONSOLID_H

/**
 * @class: GeoVG4ExtensionSolid
 *
 * @brief Abstact Base Class for a G4Solid plugin.
 *	  The G4Solid plugin builds the raw G4Solid
 */

class G4VSolid;
class GeoUnidentifiedShape;
class GeoVG4ExtensionSolid 
{
 public:

   //! Default constructor.
   GeoVG4ExtensionSolid() {}
     
   virtual ~GeoVG4ExtensionSolid() {}

   //! Create the specified solid. .
   virtual G4VSolid *newG4Solid(const GeoUnidentifiedShape *shape) const=0;

 private:

  // Illegal operations
  GeoVG4ExtensionSolid(const GeoVG4ExtensionSolid &right)=delete;
  GeoVG4ExtensionSolid & operator=(const GeoVG4ExtensionSolid &right)=delete;

};

#endif

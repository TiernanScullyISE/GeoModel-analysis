/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOSHAPEACTION_H
#define GEOMODELKERNEL_GEOSHAPEACTION_H

/**
 * @class GeoShapeAction
 *
 * @brief This class is a visitor for shapes.  It allows a
 *	navigation of the boolean expression tree for shapes, in
 *	order to accomplish actions such as volume computation.
 *	This is essentially an internal class.
 */

#include "GeoModelKernel/GeoShapePath.h"
#include <optional>

class GeoShape;
class GeoShapeIntersection;
class GeoShapeUnion;
class GeoShapeSubtraction;
class GeoShapeShift;
class GeoBox;
class GeoCons;
class GeoPcon;
class GeoPgon;
class GeoTrap;
class GeoTwistedTrap;
class GeoTrd;
class GeoPara;
class GeoTubs;
class GeoTube;
class GeoEllipticalTube;
class GeoTorus;
class GeoUnidentifiedShape;
class GeoSimplePolygonBrep;
class GeoTessellatedSolid;

class GeoGenericTrap;

class GeoVSurfaceShape;
class GeoRectSurface;
class GeoTrapezoidSurface;
class GeoAnnulusSurface;
class GeoDiamondSurface;

class GeoShapeAction 
{
 public:
  GeoShapeAction();
  virtual ~GeoShapeAction();

  std::optional<unsigned int>  getDepthLimit ();

  //	Sets a depth limit.
  void setDepthLimit (unsigned int limit);

  //	Clears a depth limit, if any.
  void clearDepthLimit ();

  //	Cause the action to terminate.
  void terminate ();

  //	Determine whether the action should terminate.
  bool shouldTerminate () const;

  //	Handles a shape.
  virtual void handleShape (const GeoShape *);

  //	Handles a shift shape.
  virtual void handleShift (const GeoShapeShift *shift);

  //	Handles a union shape.
  virtual void handleUnion (const GeoShapeUnion *unio);

  //	Handles an intersection shape.
  virtual void handleIntersection (const GeoShapeIntersection *isect);

  //	Handles a subtraction shape.
  virtual void handleSubtraction (const GeoShapeSubtraction *subtract);

  //	Handles a box shape.
  virtual void handleBox (const GeoBox *box);

  //	Handles a cons shape.
  virtual void handleCons (const GeoCons *cons);

  //	Handles a para shape.
  virtual void handlePara (const GeoPara *para);

  //	Handles a pcon shape.
  virtual void handlePcon (const GeoPcon *pcon);

  //	Handles a pgon shape.
  virtual void handlePgon (const GeoPgon *pgon);

  //	Handles a trap shape.
  virtual void handleTrap (const GeoTrap *trap);
    
  //    Handles a twistedtrap shape.
  virtual void handleTwistedTrap (const GeoTwistedTrap *trap);

  //	Handles a  trd shape.
  virtual void handleTrd (const GeoTrd *trd);

  //	Handles a tube shape.
  virtual void handleTube (const GeoTube *tube);

  //	Handles a tubs shape.
  virtual void handleTubs (const GeoTubs *tubs);
  
  //	Handles a virtual surface.
  virtual void handleShape (const GeoVSurfaceShape *);
  
  //    Handles a rectangular surface.
  virtual void handleRectSurface (const GeoRectSurface *surf);

  //    Handles a trapezoid surface.
  virtual void handleTrapezoidSurface (const GeoTrapezoidSurface *surf);

  //    Handles an annulus surface.
  virtual void handleAnnulusSurface (const GeoAnnulusSurface *surf);

  //    Handles a diamond surface.
  virtual void handleDiamondSurface (const GeoDiamondSurface *surf);

  //	Returns a pointer to the path object.
  GeoShapePath * getPath ();

  virtual void handleSimplePolygonBrep (const GeoSimplePolygonBrep *);
  virtual void handleTessellatedSolid (const GeoTessellatedSolid *);
  virtual void handleEllipticalTube (const GeoEllipticalTube *);
  virtual void handleTorus (const GeoTorus *);
  virtual void handleGenericTrap (const GeoGenericTrap *);
  virtual void handleUnidentifiedShape(const GeoUnidentifiedShape *shape);

  
 private:
  GeoShapeAction(const GeoShapeAction &right);
  GeoShapeAction & operator=(const GeoShapeAction &right);

  //	A limit may be placed upon the depth to which the action
  //	descends.  0 = self.  1 = self and children.
  std::optional<unsigned int> m_depth;
  
  //	Termination flag; causes an abortion of action execution.
  bool m_terminate;
  GeoShapePath m_path;

};

#endif

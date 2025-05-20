/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoShapeAction.h"

#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeIntersection.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoCons.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelKernel/GeoTrap.h"
#include "GeoModelKernel/GeoTwistedTrap.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoPara.h"
#include "GeoModelKernel/GeoRectSurface.h"
#include "GeoModelKernel/GeoTrapezoidSurface.h"
#include "GeoModelKernel/GeoAnnulusSurface.h"
#include "GeoModelKernel/GeoDiamondSurface.h"

#include "GeoModelKernel/GeoSimplePolygonBrep.h"
#include "GeoModelKernel/GeoTessellatedSolid.h"
#include "GeoModelKernel/GeoEllipticalTube.h"
#include "GeoModelKernel/GeoTorus.h"
#include "GeoModelKernel/GeoGenericTrap.h"
#include "GeoModelKernel/GeoUnidentifiedShape.h"

GeoShapeAction::GeoShapeAction()
{
}

GeoShapeAction::~GeoShapeAction()
{
}

std::optional<unsigned int>  GeoShapeAction::getDepthLimit ()
{
  return m_depth;
}

void GeoShapeAction::setDepthLimit (unsigned int limit)
{
  m_depth=limit;
}

void GeoShapeAction::clearDepthLimit ()
{
  m_depth=std::nullopt;
}

void GeoShapeAction::terminate ()
{
  m_terminate=true;
}

bool GeoShapeAction::shouldTerminate () const
{
  return m_terminate;
}

void GeoShapeAction::handleShape (const GeoShape *)
{
}

void GeoShapeAction::handleShape (const GeoVSurfaceShape *)
{
}

void GeoShapeAction::handleShift (const GeoShapeShift *shift)
{
  handleShape(shift);
}

void GeoShapeAction::handleUnion (const GeoShapeUnion *unio)
{
  handleShape(unio);
}

void GeoShapeAction::handleIntersection (const GeoShapeIntersection *isect)
{
  handleShape(isect);
}

void GeoShapeAction::handleSubtraction (const GeoShapeSubtraction *subtract)
{
  handleShape(subtract);
}

void GeoShapeAction::handleBox (const GeoBox *box)
{
  handleShape(box);
}

void GeoShapeAction::handleCons (const GeoCons *cons)
{
  handleShape(cons);
}

void GeoShapeAction::handlePara (const GeoPara *para)
{
  handleShape(para);
}

void GeoShapeAction::handlePcon (const GeoPcon *pcon)
{
  handleShape(pcon);
}

void GeoShapeAction::handlePgon (const GeoPgon *pgon)
{
  handleShape(pgon);
}

void GeoShapeAction::handleTrap (const GeoTrap *trap)
{
  handleShape(trap);
}

void GeoShapeAction::handleTwistedTrap (const GeoTwistedTrap *twistedtrap)
{
  handleShape(twistedtrap);
}

void GeoShapeAction::handleTrd (const GeoTrd *trd)
{
  handleShape(trd);
}

void GeoShapeAction::handleTube (const GeoTube *tube)
{
  handleShape(tube);
}

void GeoShapeAction::handleTubs (const GeoTubs *tubs)
{
  handleShape(tubs);
}

void GeoShapeAction::handleRectSurface (const GeoRectSurface *surf)
{
  handleShape(surf);
}

void GeoShapeAction::handleTrapezoidSurface (const GeoTrapezoidSurface *surf)
{
  handleShape(surf);
}

void GeoShapeAction::handleAnnulusSurface (const GeoAnnulusSurface *surf)
{
  handleShape(surf);
}

void GeoShapeAction::handleDiamondSurface (const GeoDiamondSurface *surf)
{
  handleShape(surf);
}

GeoShapePath * GeoShapeAction::getPath ()
{
  return &m_path;
}

void GeoShapeAction::handleSimplePolygonBrep (const GeoSimplePolygonBrep* brep)
{
  handleShape(brep);
}

void GeoShapeAction::handleTessellatedSolid (const GeoTessellatedSolid* tessellated)
{
  handleShape(tessellated);
}

void GeoShapeAction::handleEllipticalTube (const GeoEllipticalTube * elltube)
{
  handleShape(elltube);
}

void GeoShapeAction::handleTorus (const GeoTorus * torus)
{
  handleShape(torus);
}

void GeoShapeAction::handleGenericTrap (const GeoGenericTrap * gentrap)
{
  handleShape(gentrap);
}

void GeoShapeAction::handleUnidentifiedShape (const GeoUnidentifiedShape * uShape)
{
  handleShape(uShape);
}



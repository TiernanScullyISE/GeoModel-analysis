/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoShapeAction.h"
#include "GeoModelKernel/throwExcept.h"
#include <cmath>
#include <stdexcept>

const std::string GeoPcon::s_classType = "Pcon";
const ShapeType GeoPcon::s_classTypeID = 0x13;

GeoPcon::GeoPcon (double SPhi, double DPhi)
  : m_sPhi (SPhi)
  , m_dPhi (DPhi)
{}

double GeoPcon::volume (int) const
{
  if (!isValid ())
    THROW_EXCEPTION("Volume requested for incomplete polycone");
  double vol = 0;
  for (size_t k = 0; k < getNPlanes() - 1; ++k)
  {
    double fDz = std::abs(getZPlane(k + 1) - getZPlane(k)) / 2.0;
    double fRmin1 = getRMinPlane(k + 1);
    double fRmin2 = getRMinPlane(k);
    double fRmax1 = getRMaxPlane(k + 1);
    double fRmax2 = getRMaxPlane(k);
    vol += (m_dPhi * (1./3)) * fDz *
      (fRmax1 * fRmax1 + fRmax2 * fRmax2 + fRmax1 * fRmax2 -
       fRmin1 * fRmin1 - fRmin2 * fRmin2 - fRmin1 * fRmin2);
  }
  return vol;
}

void GeoPcon::extent (double& xmin, double& ymin, double& zmin,
                      double& xmax, double& ymax, double& zmax) const
{
  if (!isValid ())
    THROW_EXCEPTION("Extent requested for incomplete polycone");
  double rmin = getRMinPlane(0);
  double rmax = getRMaxPlane(0);
  zmin = zmax = getZPlane(0);
  for (size_t i = 1; i < getNPlanes(); ++i)
  {
    rmin = std::min(rmin, getRMinPlane(i));
    rmax = std::max(rmax, getRMaxPlane(i));
    zmin = std::min(zmin, getZPlane(i));
    zmax = std::max(zmax, getZPlane(i));
  }
  GeoShape::diskExtent(rmin, rmax, getSPhi(), getDPhi(), xmin, ymin, xmax, ymax);
}

bool GeoPcon::contains (double x, double y, double z) const
{
#ifndef M_PI
  constexpr double M_PI = 3.14159265358979323846;
#endif
  if (!isValid ()) return false;
  size_t nz = getNPlanes();
  if (z < getZPlane(0) || z > getZPlane(nz - 1)) return false;

  if (m_dPhi < 2.0 * M_PI)
  {
    GeoTrf::Vector2D r(x, y);
    GeoTrf::Vector2D ns(std::sin(m_sPhi), -std::cos(m_sPhi));
    GeoTrf::Vector2D ne(-std::sin(m_sPhi + m_dPhi), std::cos(m_sPhi + m_dPhi));
    double ds = ns.dot(r);
    double de = ne.dot(r);
    bool inwedge = (m_dPhi <= M_PI) ? (ds <= 0 && de <= 0) : (ds <= 0 || de <= 0);
    if (!inwedge) return false;
  }

  double rr = x * x + y * y;
  for (size_t k = 0; k < nz - 1; ++k)
  {
    double zmin = getZPlane(k);
    double zmax = getZPlane(k + 1);
    if (z < zmin || z > zmax || zmin == zmax) continue;
    double t = (z - zmin) / (zmax - zmin);
    double rmin1 = getRMinPlane(k);
    double rmin2 = getRMinPlane(k + 1);
    double rmin = rmin1 + (rmin2 - rmin1) * t;
    double rmax1 = getRMaxPlane(k);
    double rmax2 = getRMaxPlane(k + 1);
    double rmax = rmax1 + (rmax2 - rmax1) * t;
    if (rr <= rmax * rmax && rr >= rmin * rmin) return true;
  }
  return false;
}

void GeoPcon::addPlane (double ZPlane, double RMinPlane, double RMaxPlane)
{
  // Basic sanity checks
  if(RMinPlane < 0. || RMaxPlane < RMinPlane) {
    THROW_EXCEPTION("GeoPcon::addPlane() wrong arguments! ("
		    << ZPlane << ","
		    << RMinPlane << ","
		    << RMaxPlane << ")");
  }

  m_zPlane.push_back (ZPlane);
  m_rMinPlane.push_back (RMinPlane);
  m_rMaxPlane.push_back (RMaxPlane);
}

void GeoPcon::exec (GeoShapeAction *action) const
{
  if (!isValid ())
    THROW_EXCEPTION("Requested to execute action for incomplete polycone");

  action->handlePcon(this);
}

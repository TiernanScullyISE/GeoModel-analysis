/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoLogVol.h"

GeoLogVol::GeoLogVol (std::string_view Name, const GeoShape *Shape, const GeoMaterial *Material)
  : m_name (Name)
  , m_material (Material)
  , m_shape (Shape){}


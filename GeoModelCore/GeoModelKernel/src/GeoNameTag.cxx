/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoNodeAction.h"

GeoNameTag::GeoNameTag (std::string_view BaseName)
 : m_name (BaseName)
{
}


GeoNameTag::~GeoNameTag()
{
}

void GeoNameTag::exec (GeoNodeAction *action) const
{
  action->handleNameTag (this);
}

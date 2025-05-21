/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoVDetectorManager.h"
#include "GeoModelKernel/GeoVDetectorElement.h"
#include "GeoModelKernel/GeoVDetectorFactory.h"


GeoVDetectorManager::GeoVDetectorManager() = default;
GeoVDetectorManager::~GeoVDetectorManager() = default;


void GeoVDetectorManager::setName(const std::string & name) 
{
  m_name=name;
}

const std::string & GeoVDetectorManager::getName() const 
{
  return m_name;
}

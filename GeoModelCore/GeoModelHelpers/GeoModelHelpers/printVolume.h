/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GEOMODELUTILS_PRINTVOLUME_H
#define GEOMODELUTILS_PRINTVOLUME_H
#include "GeoModelKernel/GeoVPhysVol.h"

/** @brief creates a string to print the volume */
std::string printVolume(const PVConstLink& volume);
/** @brief Prints hte node names of the volume */
std::string printNodes(const PVConstLink& volume);

#endif
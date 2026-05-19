/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOMETRYMAP_H
#define GEOMODELKERNEL_GEOMETRYMAP_H

#include "GeoModelKernel/GeoVPhysVol.h"
#include <string_view>

class GeometryMap {
  
 public:

  // constructor
  GeometryMap();
  
  // Destructor;
  ~GeometryMap();

  GeometryMap (const GeometryMap&) = delete;
  GeometryMap& operator= (const GeometryMap&) = delete;

  // Add a name to the list of designated volumes:  Can use wildcards.
  void add(std::string_view name);
  
  // Create the map; this will maintain a list of all volumes matching
  // the expressions under the given physical volume.
  void finalize(const PVConstLink& pv);

  
  // For iteration:
  typedef std::vector<PVConstLink>::const_iterator Iterator;

  Iterator begin() const;
  Iterator end() const;

 private:

  class Clockwork;
  Clockwork *m_c;
  
};
#endif

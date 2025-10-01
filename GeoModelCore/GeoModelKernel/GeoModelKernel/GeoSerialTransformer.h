/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOSERIALTRANSFORMER_H
#define GEOMODELKERNEL_GEOSERIALTRANSFORMER_H

/**
 * @class GeoSerialTransformer
 * 
 * @brief This class takes a physical volume and places it
 * according to a transformation field, N times.
 */

#include "GeoModelKernel/GeoGraphNode.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoXF.h"

#include <memory>
class GeoSerialTransformer : public GeoGraphNode
{
 public:
  GeoSerialTransformer (const GeoVPhysVol *volume, const GeoXF::Function *func, unsigned int copies);

  //	Executes a GeoNodeAction.
  virtual void exec (GeoNodeAction *action) const;

  constexpr bool typeQuery(GeoGraphNodeType in) const final { return in == GeoGraphNodeType::NodeEndpoint; }

  //	Returns the transformation field itself.
  const GeoXF::Function * getFunction () const{
      return m_function.get();
  }

  // Returns the volume:
  PVConstLink getVolume () const {
    return m_physVol;
  }

  // Returns the number of copies:
  unsigned int getNCopies () const {
     return m_nCopies;
  }

  // Returns the transformation to the ith copy:
  GeoTrf::Transform3D getTransform (int i) const {
    return (*m_function) (i);
  }

 protected:
  virtual ~GeoSerialTransformer() = default;

 private:
  //	Number of copies of a physical volume to generate.
  unsigned int m_nCopies{0};

  //	Recipe for the transformation; specifically, a
  //	Transform-valued m_function of a single variable.
  std::unique_ptr<const GeoXF::Function> m_function{};

  //	The physical volume to be multiply placed.
  GeoIntrusivePtr<const GeoVPhysVol> m_physVol{};
};

#endif

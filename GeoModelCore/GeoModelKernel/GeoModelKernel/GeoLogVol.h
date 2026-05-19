/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOLOGVOL_H
#define GEOMODELKERNEL_GEOLOGVOL_H
/**
 * @class GeoLogVol
 * 
 * @brief A Logical Volume, or agglomeration of shape and material.
 * The logical volume increases the reference count of the shape and the material.
 */

#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelKernel/GeoMaterial.h"
#include <vector>
#include <thread>
#include <shared_mutex>
#include <mutex>
#include <string_view>

class GeoLogVol : public RCBase
{
 public:
  GeoLogVol (std::string_view Name, const GeoShape *Shape, const GeoMaterial *Material);

  //	Returns the name of the logical volume.
  const std::string & getName () const {
     return m_name;
  }
  
  //	Returns the shape of the logical volume.
  const GeoShape* getShape () const {
      return m_shape;
  }
  
  //	Returns the material of the logical volume.
  const GeoMaterial* getMaterial () const {
    std::shared_lock lock{m_mutex};
    return m_material;
  }
  
  //    In some cases (gases, liquids) materials be updated?
  void setMaterial (const GeoMaterial *newMaterial) const {
    std::unique_lock guards{m_mutex};
    m_material = newMaterial;
  }
  
 protected:
  virtual ~GeoLogVol() = default;
  
 private:
  
  //	The name of the logical volume.
  std::string m_name{};

  //	Material composition of this volume.
  mutable GeoIntrusivePtr<const GeoMaterial> m_material{};
  mutable std::shared_mutex m_mutex{};

  //	Shape of this volume.
  GeoIntrusivePtr<const GeoShape> m_shape{};
};


#endif

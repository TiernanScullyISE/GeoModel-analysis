/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOELEMENT_H
#define GEOMODELKERNEL_GEOELEMENT_H

/**
 * @class GeoElement
 *
 * @brief This class represents chemical elements.  Elements are
 * allocated on the heap and deleted automatically when their reference count 
 * falls to zero.
 */

#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/Units.h"
#include <string>
#include <string_view>

class GeoElement : public RCBase
{
 public:
  GeoElement (std::string_view Name, std::string_view Symbol, double Z, double A);
  
  int operator==(const GeoElement &right) const;
  int operator!=(const GeoElement &right) const;
  
  //	Returns the effective number of nucleons in the element.
  double getN () const{
    return m_a * (GeoModelKernelUnits::mole / GeoModelKernelUnits::gram);
  }

  //	The name of the element, e.g. "Carbon".
  const std::string& getName () const {
    return m_name;
  }

  //	The chemical symbol for the element, e.g. C, O, S, Na....
  const std::string& getSymbol () const {
    return m_symbol;
  }

  //	The atomic number Z for the material.
  double getZ() const{
    return m_z;
  }

  //	The average atomic mass for the element.
  double getA() const {
    return m_a;
  }
  
  //        Tsai formula for the radiation length
  double getRadTsai() const;
  
 protected:
  virtual ~GeoElement() = default;
  
 private:
  std::string m_name{};
  std::string m_symbol{};
  double m_z{0.};
  double m_a{0.};  
};

inline int GeoElement::operator==(const GeoElement &right) const{
  return m_name == right.m_name && m_symbol == right.m_symbol && m_z == right.m_z && m_a == right.m_a;
}

inline int GeoElement::operator!=(const GeoElement &right) const
{
  return m_name != right.m_name ||
    m_symbol != right.m_symbol || m_z != right.m_z || m_a != right.m_a;
}


#endif

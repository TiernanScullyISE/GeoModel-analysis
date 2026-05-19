/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOMATERIAL_H
#define GEOMODELKERNEL_GEOMATERIAL_H

/**
 * @class GeoMaterial
 * 
 * @brief This class represents materials.  A material is a
 *      collection of elements.  In addition it has a density,
 *      radiation length, and interaction length, and can
 *      provide a description of energy lost which is computed
 *      from its composition.
 *
 *      Also noteworthy:
 *
 *      * Materials are allocated on the heap and deleted
 *      automatically when their reference count falls to zero.
 *
 *      * After the constructor is called the elements are added
 *      one-by-one; when the last one is called the material
 *      must be "locked" after the last element is added.
 *
 *      * The materials are assigned an identifier.
 */

#include "GeoModelKernel/RCBase.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include <vector>
#include <string_view>

class GeoMaterial : public RCBase {
 public:
  GeoMaterial (std::string_view name, double density);
  
  // Add an element to the material.
  void add (const GeoElement* element, double fraction = 1.0);
  
  // Add another material to the material (this copies all of
  // the element from the other material into this one).
  // Fraction is by mass.
  void add (const GeoMaterial* material, double fraction);
  
  // Lock the material against the addition of other
  // materials or elements.
  void lock ();
  
  // Constant dEdx term.
  double getDeDxConstant () const;
  
  // I0 term (ionization potential).  From tables. Units:  eV.
  double getDeDxI0 () const;
  
  // Get dEdx_min.  dEdxConstant*11.528 Paul Avery, CBX-92-39.
  double getDeDxMin () const;
  
  // Returns the radiation length, computed from the density
  // and the list of constituents, and their properties.
  double getRadLength () const;
  
  // Return the nuclear interaction length, computed from the
  // density, the list of constituents, and their properties.
  double getIntLength () const;

  // Return the number of elements  
  unsigned int getNumElements () const;
  
  // Gets the ith element.
  const GeoElement* getElement (unsigned int i) const;
  
  // Gets the fraction by weight of the ith element
  double getFraction (int i) const;
  
  // The name of the material.
  const std::string& getName () const{
      return m_name;
  }
  // The density of the material.
   double getDensity () const{
      return m_density;
   }
  // Gives an integral identifier for the material.  For
  // convenience.
  unsigned int getID () const {
      return m_iD;
  }
  
 protected:
  virtual ~GeoMaterial() = default;
  
 private:
  
  std::string m_name{};
  double m_density{0.};
  unsigned int m_iD{0};


  // The radiation length of the material.
  double m_radLength{0.};
  
  // The interaction length of the material.
  double m_intLength{0.};

  // The constant term in the formula governing dE/dx.
  double m_dedDxConst{0.};

  // The ionization potential in the formula governing dE/dx.
  double m_deDxI0{0.};

  // A flag used to lock the material from further addition
  // of elements or other constituents.
  bool m_locked{false};

  // A list of the fractional composition of each material.
  // Fraction is by mass.

  /// The list of GeoElements composing a GeoMaterial with the corresponding fraction.
  using ElementWithFrac = std::pair<GeoIntrusivePtr<const GeoElement>, double>;
  std::vector<ElementWithFrac> m_elements{};

  // A static used to assign unique identifiers to new materials.
  static std::atomic<unsigned int> s_lastID;
  
};

#endif

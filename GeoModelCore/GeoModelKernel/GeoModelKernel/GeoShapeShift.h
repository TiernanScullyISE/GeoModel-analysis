/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOSHAPESHIFT_H
#define GEOMODELKERNEL_GEOSHAPESHIFT_H

#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelKernel/GeoDefinitions.h"


class GeoShapeShift : public GeoShape {
 public:
  //    Constructor
  GeoShapeShift (const GeoShape* A, const GeoTrf::Transform3D &X);

  //    Returns the volume of the shape, for mass inventory
  virtual double volume (int npoints = 1000000) const {
     return m_op->volume(npoints);
  }

  //    Returns the bonding box of the shape
  virtual void extent (double& xmin, double& ymin, double& zmin,
                       double& xmax, double& ymax, double& zmax) const;

  //    Returns true if the shape contains the point, false otherwise
  virtual bool contains (double x, double y, double z) const;

  //    Returns the OR shape type, as a string.
  virtual const std::string & type() const {
      return getClassType();
  }

  //    Returns the OR shape type, as a coded integer.
  virtual ShapeType typeID () const {
    return getClassTypeID();
  }

  //    Returns true if OR is a polyhedron, false otherwise
  virtual bool isPolyhedron () const {
    return m_op->isPolyhedron();
  }

  //    Returns number of constituents
  virtual unsigned int getNoConstituents () const {
    return m_op->getNoConstituents();
  }

  // Returns the first operand being ORed
  const GeoShape* getOp() const {
      return m_op;
  }

  //    Returns the shift of this shape.
  const GeoTrf::Transform3D & getX () const {
     return m_shift;
  }

  //    Executes a GeoShapeAction
  virtual void exec (GeoShapeAction *action) const;


  static ShapeType getClassTypeID () {
     return s_classTypeID;
  }

  //    For type identification.
  static const std::string& getClassType () {
     return s_classType;
  }

 protected:
    virtual ~GeoShapeShift() = default;

 private:
    //    The shape operand in the NOT operation.
    GeoIntrusivePtr<const GeoShape> m_op{};

    //    Gives the amount by which the volume is shifted.
    GeoTrf::Transform3D m_shift{GeoTrf::Transform3D::Identity()};

    static const std::string s_classType;
    static const ShapeType s_classTypeID;
};


#endif

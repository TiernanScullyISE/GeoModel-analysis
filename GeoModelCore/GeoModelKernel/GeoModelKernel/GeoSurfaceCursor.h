#ifndef GEOMODELKERNEL_GEOSURFCURSOR_H
#define GEOMODELKERNEL_GEOSURFCURSOR_H

#include "GeoModelKernel/GeoNodeAction.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoVSurface.h"
#include <vector>

class GeoVAlignmentStore;

class GeoSurfaceCursor final : public GeoNodeAction
{
 public:
 
  using VSConstLink = GeoIntrusivePtr<const GeoVSurface>;
  
  GeoSurfaceCursor (PVConstLink parent, GeoVAlignmentStore* store=nullptr);
  virtual ~GeoSurfaceCursor() override;
  
  GeoSurfaceCursor(const GeoSurfaceCursor &right) = delete;
  GeoSurfaceCursor & operator=(const GeoSurfaceCursor &right) = delete;
  
  void next();
  
  bool atEnd() const;
  
  /// Returns the transformation to the surface or volume.
  GeoTrf::Transform3D getTransform () const;
  
  /// Returns the default ransformation to the surface or volume.
  GeoTrf::Transform3D getDefTransform () const;
/*
  /// Returns the name of the surface. All are called VSurface for now.
  std::string getName () const;
*/
 private:
  /// Handles a Transform.
  virtual void handleTransform (const GeoTransform *xform) override; 
 
  /// Handles a physical volume.
  virtual void handlePhysVol (const GeoPhysVol *vol) override;

  /// Handles a physical volume.
  virtual void handleFullPhysVol (const GeoFullPhysVol *vol) override;
   
  /// Handles a rectangular virtual surface.
  virtual void handleVSurface (const GeoVSurface *surf) override;
  
  /// Ressucitate (undo terminate)
  void resuscitate();
  
  PVConstLink                           m_parent;
  PVConstLink                           m_volume;
  VSConstLink                           m_surface;
  GeoTrf::Transform3D                   m_transform;
  GeoTrf::Transform3D                   m_defTransform;
      
  unsigned int                          m_majorIndex;
  unsigned int                          m_volCount;
  unsigned int                          m_surfCount;
   
  std::vector<const GeoTransform *>     m_pendingTransformList;
  
  bool                                  m_hasAlignTrans{};  
  GeoVAlignmentStore                   *m_alignStore;
};

#endif

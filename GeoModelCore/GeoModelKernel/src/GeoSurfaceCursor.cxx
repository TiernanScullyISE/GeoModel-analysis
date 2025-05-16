#include <utility>

#include "GeoModelKernel/GeoSurfaceCursor.h"
#include "GeoModelKernel/GeoAlignableTransform.h"

GeoSurfaceCursor::GeoSurfaceCursor (PVConstLink parent, GeoVAlignmentStore* store)
  : m_parent(std::move(parent))
  , m_transform(GeoTrf::Transform3D::Identity())
  , m_defTransform(GeoTrf::Transform3D::Identity())  
  , m_majorIndex(0)
  , m_volCount(0)
  , m_surfCount(0)
  , m_alignStore(store)  
{
  // depth is set to be 0, with next(), go to the child node
  setDepthLimit(0);
  next();
}

GeoSurfaceCursor::~GeoSurfaceCursor()
{
}

void GeoSurfaceCursor::next() {
  resuscitate();
  
  // not considering serial transformer
  m_volume=nullptr;
  m_surface=nullptr;

  m_pendingTransformList.erase (m_pendingTransformList.begin (),
 	                        m_pendingTransformList.end ());  
 	                        
  int N = m_parent->getNChildNodes();
  if (N==0) return;  
  
  const GeoGraphNode * const *node  = m_parent->getChildNode(m_majorIndex);
  const GeoGraphNode * const *back  = m_parent->getChildNode(N-1);
  const GeoGraphNode * const *end   = back+1;  
  int i = 0;////////////
  while (node!=end) {
    (*node)->exec(this);
    const GeoGraphNode * const * flag = node;
    
    node++;
    m_majorIndex++;
    
    if (m_terminate) {
      
      if (dynamic_cast<const GeoVPhysVol*> (*flag)){
        m_volCount++;
      }
      else if (dynamic_cast<const GeoVSurface*> (*flag)){
        m_surfCount++;
      }
      
      break;
    }
  }    
}

void GeoSurfaceCursor::resuscitate() {
  m_terminate = false;
}

bool GeoSurfaceCursor::atEnd() const {
  return (!m_volume and !m_surface);
}

void GeoSurfaceCursor::handleTransform (const GeoTransform *xform)
{
  m_pendingTransformList.push_back (xform);
  if(dynamic_cast<const GeoAlignableTransform*>(xform))
    m_hasAlignTrans = true;
}

void GeoSurfaceCursor::handlePhysVol (const GeoPhysVol *vol)
{
  m_volume = vol;
  
  unsigned int listSize = m_pendingTransformList.size ();
  if (listSize == 0) {
    m_transform    = GeoTrf::Transform3D::Identity();
    m_defTransform = GeoTrf::Transform3D::Identity();
  }
  else {
    m_transform = m_pendingTransformList[0]->getTransform(m_alignStore);
    m_defTransform = m_pendingTransformList[0]->getDefTransform(m_alignStore);
    for (unsigned int t = 1; t < m_pendingTransformList.size (); t++) {
      m_transform    = m_transform    * m_pendingTransformList[t]->getTransform(m_alignStore);
      m_defTransform = m_defTransform * m_pendingTransformList[t]->getDefTransform(m_alignStore);
    }
  }
    
  terminate ();
}

void GeoSurfaceCursor::handleFullPhysVol (const GeoFullPhysVol *vol)
{
  m_volume = vol;
  
  unsigned int listSize = m_pendingTransformList.size ();
  if (listSize == 0) {
    m_transform    = GeoTrf::Transform3D::Identity();
    m_defTransform = GeoTrf::Transform3D::Identity();
  }
  else {
    m_transform = m_pendingTransformList[0]->getTransform(m_alignStore);
    m_defTransform = m_pendingTransformList[0]->getDefTransform(m_alignStore);
    for (unsigned int t = 1; t < m_pendingTransformList.size (); t++) {
      m_transform    = m_transform    * m_pendingTransformList[t]->getTransform(m_alignStore);
      m_defTransform = m_defTransform * m_pendingTransformList[t]->getDefTransform(m_alignStore);
    }
  }  
  
  terminate ();
}

void GeoSurfaceCursor::handleVSurface (const GeoVSurface *surf)
{
  m_surface = surf;
  
  unsigned int listSize = m_pendingTransformList.size ();
  if (listSize == 0) {
    m_transform    = GeoTrf::Transform3D::Identity();
    m_defTransform = GeoTrf::Transform3D::Identity();
  }
  else {
    m_transform = m_pendingTransformList[0]->getTransform(m_alignStore);
    m_defTransform = m_pendingTransformList[0]->getDefTransform(m_alignStore);
    for (unsigned int t = 1; t < m_pendingTransformList.size (); t++) {
      m_transform    = m_transform    * m_pendingTransformList[t]->getTransform(m_alignStore);
      m_defTransform = m_defTransform * m_pendingTransformList[t]->getDefTransform(m_alignStore);
    }
  }  
  
  terminate ();
}

GeoTrf::Transform3D GeoSurfaceCursor::getTransform () const
{
  //if (m_minorLimit) {
   // return m_transform*m_serialTransformer->getTransform(m_minorIndex);
  //}
  //else {
    return m_transform;
  //}
}

GeoTrf::Transform3D GeoSurfaceCursor::getDefTransform () const
{
  //if (m_minorLimit) {
  //  return m_defTransform*m_serialTransformer->getTransform(m_minorIndex);
  //}
  //else {
    return m_defTransform;
  //}
}
/*
std::string GeoSurfaceCursor::getName() const
{ 
    std::string name = "VSurface";
    return name;
}
*/

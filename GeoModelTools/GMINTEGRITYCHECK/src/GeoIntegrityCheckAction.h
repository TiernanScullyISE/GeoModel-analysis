/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOINTEGRITYCHECKACTION_H
#define GEOMODELKERNEL_GEOINTEGRITYCHECKACTION_H

/**
 * @class GeoIntegrityCheckAction
 * 
 * @brief This action prints the node tree.  It can be configured
 * to print a message on the following types of nodes:
 *      * Transforms
 *      * Physical Volumes and Full Physical Volumes
 *      * SerialDenominators
 *      * SerialTransforms 
 *      * IdentifierTag
 */

#include "GeoModelKernel/GeoNodeAction.h"
#include <iostream>
#include <set>

class GeoIntegrityCheckAction : public GeoNodeAction
{
  enum Type { TRANSFORM,
	      VOLUME,
	      NAMETAG,
	      IDENTIFIERTAG};
 public:
  GeoIntegrityCheckAction ();
  virtual ~GeoIntegrityCheckAction() = default;

  //	Handles a Transform.
  virtual void handleTransform (const GeoTransform *xform);

  //	Handles a physical volume.
  virtual void handlePhysVol (const GeoPhysVol *vol);
  
  //	Handles a physical volume.
  virtual void handleFullPhysVol (const GeoFullPhysVol *vol);
  
  //	Handles a Name Tag.
  virtual void handleNameTag (const GeoNameTag *nameTag);
  
  //	Handles a Serial Denominator.
  virtual void handleSerialDenominator (const GeoSerialDenominator *sD);
  
  //	Handles a Serial Transformer
  virtual void handleSerialTransformer (const GeoSerialTransformer  *sT);
  
  //	Handles an Identifier Tag.
  virtual void handleIdentifierTag (const GeoIdentifierTag *idTag);
  
  //	Sets the notification state.  Default: everything on.
  void setNotification (Type type, bool state);

  const std::set<const GeoAlignableTransform *>  & getAlignableTransforms() const { return m_alignableTransformList;}
  const std::set<const GeoFullPhysVol *>         & getFullPhysicalVolumes() const { return m_fullPhysVolList;}
  
 private:
  
  //	A pointer to a name tag.  If the volume is named.
  const GeoNameTag *m_nameTag{nullptr};

  //	A pointer to a serial denominator.  If one exists.
  const GeoSerialDenominator *m_serialDenominator{nullptr};

  //	A pointer to an identifier tag.  If the volume is
  //	identified.
  const GeoIdentifierTag *m_idTag{nullptr};

  //	On/off flag for transforms.
  bool m_transformState{true};

  //	On/off flag for physical volumes.
  bool m_volumeState{true};

  //	On/off flag for name tags.
  bool m_nametagState{true};

  //	On/off flag for serial denominators.
  bool m_serialDenominatorState{true};
  
  //	On/off flag for serial transformers.
  bool m_serialTransformerState{true};

  //	On/off flag for identifier tags.
  bool m_identifierState{true};

  std::set<const GeoFullPhysVol *> m_fullPhysVolList{};
  std::set<const GeoAlignableTransform *> m_alignableTransformList{};

  
};


#endif

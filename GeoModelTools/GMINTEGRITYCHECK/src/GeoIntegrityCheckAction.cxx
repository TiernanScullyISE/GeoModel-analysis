/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoIntegrityCheckAction.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
GeoIntegrityCheckAction::GeoIntegrityCheckAction ()
{
}


void GeoIntegrityCheckAction::handleTransform (const GeoTransform *xform)
{
  const GeoAlignableTransform *xf=dynamic_cast<const GeoAlignableTransform *> (xform);
  if (xf) m_alignableTransformList.insert(xf);
}


void GeoIntegrityCheckAction::handlePhysVol (const GeoPhysVol *vol)
{
}

void GeoIntegrityCheckAction::handleFullPhysVol (const GeoFullPhysVol *vol)
{
    m_fullPhysVolList.insert(vol);
}

void GeoIntegrityCheckAction::handleNameTag (const GeoNameTag *nameTag)
{
}

void GeoIntegrityCheckAction::handleSerialDenominator (const GeoSerialDenominator *sD)
{
}

void GeoIntegrityCheckAction::handleSerialTransformer (const GeoSerialTransformer  *sT)
{
}

void GeoIntegrityCheckAction::handleIdentifierTag (const GeoIdentifierTag *idTag)
{
}

void GeoIntegrityCheckAction::setNotification (Type type, bool state)
{
  if (type==TRANSFORM) {
    m_transformState=state;
  }
  else if (type==VOLUME) {
    m_volumeState=state; 
  }
  else if (type==NAMETAG) {
    m_nametagState=state;
  }
  else if (type==IDENTIFIERTAG) {
    m_identifierState=state;
  }
}





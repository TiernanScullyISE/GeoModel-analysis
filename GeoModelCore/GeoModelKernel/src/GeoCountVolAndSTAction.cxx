/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoCountVolAndSTAction.h"

GeoCountVolAndSTAction::GeoCountVolAndSTAction(){
  setDepthLimit (1);
}

void GeoCountVolAndSTAction::handlePhysVol(const GeoPhysVol*){
  //    
  // Do not count the top volume, this action counts only children!   
  //   
  if (getPath ()->getLength () > 1)
    ++m_count;
}

void GeoCountVolAndSTAction::handleFullPhysVol(const GeoFullPhysVol*){
  //   
  // Do not count the top volume, this action counts only children!   
  //   
  if (getPath ()->getLength () > 1)
    ++m_count;
}

void GeoCountVolAndSTAction::handleSerialTransformer(const GeoSerialTransformer *){
  ++m_count;
}






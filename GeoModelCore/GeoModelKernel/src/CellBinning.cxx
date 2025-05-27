/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/CellBinning.h"
#include <stdexcept>

CellBinning::CellBinning (double startVal, double endVal, int nBins, int firstDivisionNumber)
  :m_start(startVal),m_end(endVal),m_numDivisions(nBins),m_firstDivisionNumber(firstDivisionNumber)
{
  if (m_numDivisions < 1) throw std::range_error("CellBinning::CellBinning: nBins is zero or negative");
  m_delta = (m_end-m_start)/m_numDivisions;
}


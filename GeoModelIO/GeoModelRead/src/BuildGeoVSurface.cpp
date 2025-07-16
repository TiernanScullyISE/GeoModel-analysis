/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * BuildGeoVSurfaces.h
 *
 * Created on: June, 2024
 * Author: Rui XUE <r.xue@cern.ch><rux23@pitt.edu>
 *
 */
#include "GeoModelRead/BuildGeoVSurface.h"

namespace GeoModelIO {

BuildGeoVSurface::BuildGeoVSurface(DBRowsList&& surfaceRecords):
    m_surfaceRecords{std::move(surfaceRecords)} {}

GeoIntrusivePtr<GeoVSurfaceShape> 
    BuildGeoVSurface::getSurface(const unsigned int dbID){
    if (auto retObj = m_memCache.get(dbID); retObj!=nullptr) {
        return retObj;
    }
    if (dbID > m_surfaceRecords.size()) {
        THROW_EXCEPTION("Cannot build a new surface under ID "<<dbID);
    }
    const DBRowEntry& entry = m_surfaceRecords[dbID -1];
    buildSurface(entry);
    if (m_memCache.size() == m_surfaceRecords.size()) {
        m_surfaceRecords = DBRowsList{};
    }
    return m_memCache.get(dbID);
}
void BuildGeoVSurface::storeNewSurface(unsigned int dbID,                           
                                       GeoIntrusivePtr<GeoVSurfaceShape>&& surface){
    if (!m_memCache.insert(std::make_pair(dbID, std::move(surface)))){
        THROW_EXCEPTION("Failed to register surface under ID "<<dbID);
    }
}
std::size_t BuildGeoVSurface::size() const { return m_memCache.size(); }
}

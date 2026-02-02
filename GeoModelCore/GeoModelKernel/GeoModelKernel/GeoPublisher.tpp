/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelKernel/throwExcept.h"

#include <ranges>


template<typename Key_t>
bool GeoPublisher::containsRecord(const RecordMap_t<Key_t>& storeage,
                                  const Key_t node, 
                                  const DBRecord record) const {
    const auto [begin, end] = storeage.equal_range(node);
    for (auto itr = begin; itr != end ; ++itr) {
        if (itr->second == record) {
            return true;
        }
    }
    return false;
}



template <class N, typename T>
void GeoPublisher::publishNode(N node, T keyT) {
    static_assert(std::is_same_v<GeoVFullPhysVol *, N> || std::is_same_v<GeoAlignableTransform *, N>,
                  "ERROR!!! The node type is not currently supported by 'GeoPublisher'.\n If in doubt, please ask to `geomodel-developers@cern.ch'.\n");
    if (!node) {
        THROW_EXCEPTION("Node to publish must not be a nullptr");
    }
    if constexpr (std::is_same_v<GeoVFullPhysVol *, N>){
        if (!containsRecord(m_publishedFPV, node, keyT)) {
            m_publishedFPV.emplace(node, keyT);
        }
    } else if constexpr (std::is_same_v<GeoAlignableTransform *, N>) {
        if (!containsRecord(m_publishedAXF, node, keyT)){
            m_publishedAXF.emplace(node, keyT);
        }
    }
}

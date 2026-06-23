/* vim: set ft=cpp: */ // VIM modeline settings

/* 
 *  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "GeoModelKernel/throwExcept.h"

#include <ranges>


template<typename Key_t>
bool GeoPublisher::containsRecord(const RecordMap_t<Key_t>& storage,
                                  const DBRecord record,
                                  const Key_t node
                                  ) const {
    // get range of elements matching a specific key 
    const auto [begin, end] = storage.equal_range(record);
    for (auto itr = begin; itr != end ; ++itr) {
        if (itr->first == record) {
            return true;
        }
    }
    return false;
}



template <class N, typename T>
void GeoPublisher::publishNode(N node, T keyT) {
    static_assert(std::is_same_v<GeoVFullPhysVol *, N> || std::is_same_v<GeoAlignableTransform *, N>,
                  "ERROR!!! The node type is not currently supported by 'GeoPublisher'.\n Only instances of 'GeoVFullPhysVol' and 'GeoAlignableTransform' nodes can be published.\n In case of questions, please write to `geomodel-developers@cern.ch'.\n");
    if (!node) {
        THROW_EXCEPTION("Node to publish must not be a nullptr");
    }
    bool duplicateKey = false;
    std::string nodeClass;
    if constexpr (std::is_same_v<GeoVFullPhysVol *, N>){
        nodeClass = "GeoVFullPhysVol";
        if (!containsRecord(m_publishedFPV, keyT, node)) {
            m_publishedFPV.emplace(keyT, node);
        } else {
            duplicateKey = true;
        }
    } else if constexpr (std::is_same_v<GeoAlignableTransform *, N>) {
        nodeClass = "GeoAlignableTransform";
        if (!containsRecord(m_publishedAXF, keyT, node)){
            m_publishedAXF.emplace(keyT, node);
        } else {
            duplicateKey = true;
        }
    }
    if (duplicateKey) {
        THROW_EXCEPTION("The key " << keyT << " has been used to publish a " << nodeClass << " already! Please use a different key. Also, you can  publish the same '" << nodeClass << "' node multiple times; but you have to use a different key for each of them.");
    }
}

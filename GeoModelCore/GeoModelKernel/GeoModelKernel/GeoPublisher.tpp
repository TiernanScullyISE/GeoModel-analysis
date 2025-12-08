/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelKernel/throwExcept.h"

template <class N, typename T>
void GeoPublisher::publishNode(N node, T keyT) {
    std::any key = keyT;
    static_assert(std::is_same_v<GeoVFullPhysVol *, N> || std::is_same_v<GeoAlignableTransform *, N>,
                  "ERROR!!! The node type is not currently supported by 'GeoPublisher'.\n If in doubt, please ask to `geomodel-developers@cern.ch'.\n");
    if (!node) {
        THROW_EXCEPTION("Node to publish must not be a nullptr");
    }
    if constexpr (std::is_same_v<GeoVFullPhysVol *, N>){
        m_publishedFPV.insert({node, key});
    } else if constexpr (std::is_same_v<GeoAlignableTransform *, N>) {
        m_publishedAXF.insert({node, key});
    }
}

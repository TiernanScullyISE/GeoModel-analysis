
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GEOMODELREAD_GeoIdObjMap_H
#define GEOMODELREAD_GeoIdObjMap_H

#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <vector>
#include <ranges>
#include <algorithm>
#include <cassert>

namespace GeoModelIO{
    /** @brief Helper container to store arbitrary elements by an Identifier. 
     *         Read & write access to the container are thread-safe. The container 
     *         is non-iterable. */ 
    template <typename ObjType> 
        class GeoIdObjMap {
            public:
                using Key_t = std::size_t;
                /** @brief Empty default constructor */
                GeoIdObjMap() = default;
                /** @brief Empty default destructor */
                ~GeoIdObjMap() = default;
                /** @brief Move contructor */
                GeoIdObjMap(GeoIdObjMap&& other){
                    (*this) = std::move(other);
                }
                /** @brief Move assignment */
                GeoIdObjMap& operator=(GeoIdObjMap&& other) {
                    if (this != &other) {
                        std::unique_lock lockO{other.m_mutex};
                        std::unique_lock lockT{m_mutex};
                        m_map = std::move(other.m_map);
                    }
                    return *this;
                }
                /** @brief Fetch an object from the container which
                 *         has been stored before under the id
                 * @brief id: Identifier of the object of interest */
                ObjType get(const Key_t id) const{
                    std::shared_lock lock{m_mutex};
                    return m_map.size() > id ? m_map[id] : ObjType{};
                }
                /** @brief Inserts a new object under the Identifier id.
                 *         Returns whether the insert was a new one
                 *  @param newElem: Identifier - Obj pair to store. */
                bool insert(std::pair<Key_t, ObjType>&& newElem) {
                    std::unique_lock lock{m_mutex};
                    if (m_map.size() <= newElem.first) {
                        m_map.resize(newElem.first + 1);
                    }
                    assert(m_map.size() > newElem.first);
                    if (m_map[newElem.first]  || !newElem.second) {
                        return false;
                    }
                    m_map[newElem.first] = std::move(newElem.second);
                    return true;
                }
                /** @brief Returns the current size of the map */
                std::size_t size() const {
                    std::shared_lock lock{m_mutex};
                    return m_map.size();
                }
                /** @brief Returns the list of all stored keys */
                std::vector<Key_t> keys() const{
                    std::shared_lock lock{m_mutex};
                    std::vector<Key_t> toRet{};
                    toRet.reserve(size());
                    for (Key_t k = 0; k < size(); ++k) {
                        if (m_map[k]) toRet.push_back(k);
                    }
                    return toRet;
                }
                /** @brief Clears the map content */
                void clear() {
                    std::unique_lock lock{m_mutex};
                    m_map.clear();
                }
                /** @brief Returns whether the map is empty */
                bool empty() const{
                    std::shared_lock lock{m_mutex};
                    return m_map.empty();
                }
            private:
                mutable std::shared_mutex m_mutex{};
                std::vector<ObjType> m_map{};
        };
}

#endif
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GEOMODELKERNEL_GeoIntrusivePtr_H
#define GEOMODELKERNEL_GeoIntrusivePtr_H

#include <GeoModelKernel/RCBase.h>
#include <type_traits>
#include <concepts>
#include <utility>


template<typename GeoType> 
class GeoIntrusivePtr{
    public:
        template <typename GeoTypeGrp> friend class GeoIntrusivePtr;
        /// Default constructor
        GeoIntrusivePtr() noexcept = default;
        // Standard constructor taking a bare pointer
        GeoIntrusivePtr(GeoType* obj) noexcept:
            m_ptr{obj} {
             if (m_ptr) obj->ref();
        }
        /// Copy constructor 
         GeoIntrusivePtr(const GeoIntrusivePtr& other) noexcept:
            GeoIntrusivePtr{other.get()} {}

        /// Copy constructor for derived types
        template <typename GeoTypeGrp>
        GeoIntrusivePtr(const GeoIntrusivePtr<GeoTypeGrp>& other) noexcept
            requires(!std::is_same_v<GeoType,GeoTypeGrp> && 
                      std::is_base_of_v<GeoType, GeoTypeGrp>):
              GeoIntrusivePtr{other.get()} {}

        /// Move constructor
        GeoIntrusivePtr(GeoIntrusivePtr&& other) noexcept:
            GeoIntrusivePtr{} { 
            move(std::move(other));
        }
        /// Move constructor for derived types
        template <typename GeoTypeGrp>
        GeoIntrusivePtr(GeoIntrusivePtr<GeoTypeGrp>&& other) noexcept
            requires(!std::is_same_v<GeoType,GeoTypeGrp> && 
                      std::is_base_of_v<GeoType, GeoTypeGrp>):
            GeoIntrusivePtr{} { 
            move(std::move(other));
        }
        /// Assignment operator
        GeoIntrusivePtr& operator=(const GeoIntrusivePtr& other) noexcept {
            reset(other.get());
            return *this;
        }
        template <typename GeoTypeGrp>
         GeoIntrusivePtr& operator=(const GeoIntrusivePtr<GeoTypeGrp>& other) 
            requires(!std::is_same_v<GeoType,GeoTypeGrp> && 
                      std::is_base_of_v<GeoType, GeoTypeGrp>) {
            reset(other.get());
            return *this;
        }

        GeoIntrusivePtr& operator=(GeoType* other) noexcept {
            reset(other);
            return *this;
        }
        /// Move assignment operator
        GeoIntrusivePtr& operator=(GeoIntrusivePtr&& other) noexcept {
            move(std::move(other));
            return *this;
        }
        template <typename GeoTypeGrp>
         GeoIntrusivePtr& operator=(GeoIntrusivePtr<GeoTypeGrp>&& other) noexcept
            requires(!std::is_same_v<GeoType,GeoTypeGrp> && 
                      std::is_base_of_v<GeoType, GeoTypeGrp>) {
            move(std::move(other));
            return *this;
        }
        /// Reset the pointer
        void reset(GeoType* ptr = nullptr) {
            if (m_ptr == ptr) return;
            if (m_ptr) m_ptr->unref();
            m_ptr = ptr;
            if (m_ptr) m_ptr->ref();
        }
        template <typename GeoTypeGrp>
            void move(GeoIntrusivePtr<GeoTypeGrp>&& obj)
            requires(std::is_base_of_v<GeoType, GeoTypeGrp>) {
                if (m_ptr != obj.get()) {
                    if (m_ptr) m_ptr->unref();
                    m_ptr = obj.m_ptr;
                }
                obj.m_ptr = nullptr;
        }
        /// Destructor
        ~GeoIntrusivePtr() {
            if (m_ptr) m_ptr->unref();
        }
        /// Implicit conversion
        operator GeoType*() const { return m_ptr; }
        operator GeoType* const*() const { return &m_ptr; }

        /// Pointer access
        GeoType* operator->() const { return m_ptr; }
        /// Dereference the pointer
        GeoType& operator*() const { return *m_ptr; }
        /// Getter methods
        GeoType* get() const { return m_ptr; }
        /// Validity operator
        operator bool () const { return m_ptr != nullptr; }
        /// Invalidity operator
        bool operator!() const { return !m_ptr; }
        /// Comparison operator
        template <typename GeoTypeGrp>
        bool operator==(const GeoIntrusivePtr<GeoTypeGrp>& other) const
          requires(std::is_base_of_v<GeoType, GeoTypeGrp>)  {
            return m_ptr == other.m_ptr;
         }
        bool operator==(GeoType* other) const {
            return m_ptr == other;
        }
        template <typename GeoTypeGrp>
        bool operator !=(const GeoTypeGrp & other) const
          requires(std::is_base_of_v<GeoType, GeoTypeGrp>) {
            return !( (*this) == other );
        }

        /// Odering operator
        bool operator<(const GeoIntrusivePtr& other) const {
            return m_ptr < other.m_ptr;
        }       
    private:
        GeoType* m_ptr{nullptr};
};

/// Function to perform an easy pointer cast from one GeoIntrusivePtr type to another
template <typename ToCast, typename FromCast>
    GeoIntrusivePtr<ToCast> dynamic_pointer_cast(const GeoIntrusivePtr<FromCast>& from) 
        requires(std::is_base_of_v<FromCast, ToCast>) {
        ToCast* raw_ptr = dynamic_cast<ToCast*>(from.get());
        return GeoIntrusivePtr<ToCast>(raw_ptr);
    }
/// Function to perform an easy pointer const_cast from one GeoIntrusivePtr type to another
template <typename CastType>
    GeoIntrusivePtr<CastType> const_pointer_cast(const GeoIntrusivePtr<const CastType>& from) {
        CastType* raw_ptr = const_cast<CastType*>(from.get());
        return GeoIntrusivePtr<CastType>(raw_ptr);
    }
/// Create a GeoModel object and pipe it directly into the GeoIntrusivePtr
template <typename GeoObjType, class... Args> 
    GeoIntrusivePtr<GeoObjType> make_intrusive(Args... args) {
    return GeoIntrusivePtr<GeoObjType>{new GeoObjType(std::forward<Args>(args)...)};
}
#endif
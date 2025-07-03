/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO_MODEL_XML_MATERIAL_MANAGER_H
#define GEO_MODEL_XML_MATERIAL_MANAGER_H

#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"

#include <memory>
#include <string>
#include <map>



class MaterialManager {

  friend class Gmx2Geo;

 public:

    /// @brief  Retrieve a GeoElement by name
    const GeoElement* getElement(const std::string & name) const;
    /// @brief  Retrieve a GeoElement by number from the periodic table
    const GeoElement* getElement(unsigned int atomicNumber) const;
    /// @brief  Creates a new GeoElement
    /// @param name Name inside the GeoElement map
    /// @param symbol Chemical symbol
    /// @param z Number in the periodic table
    /// @param a Atomic weight
    void addElement(const std::string &name, 
                    const std::string &symbol, 
                    double z, 
                    double a);
    /// @brief  Add a GeoElement* to the managed objects. If an element with the same name
    ///         has already been registered, then an exception is thrown
    void addElement(const GeoElement* elePtr);
    /// @brief Returns whether the element is already registered
    bool isElementDefined(const std::string& eleName) const;

    
    /// @brief Returns the GeoMaterial by name. The name is expected to start with
    ///        a namespace. If the Material couldn't be found in the registry, the
    ///        current namespace is attempted to find the Material
    /// @param name 
    /// @return 
    const GeoMaterial* getMaterial(const std::string & name) const;
    
    /// @brief Checks whether the matieral already exists in the database
    bool isMaterialDefined(const std::string& matName) const;

    /// @brief Sets the name space underwich the following material are going to be added
    void setMaterialNamespace(const std::string &name);
    /// @brief Returns the current Material namespace
    const std::string& materialNameSpace() const;

    /// @brief  Dumps the complete content of the Material database
    void printAll() const;
    /// @brief Creates a new material called <name> with density
    ///        If the name contains ::, it's assumed that the prestring
    ///        Is part of another namespace as the current one.
    void addMaterial(const std::string &name, double density);

    /// @brief Registers a new material to the database
    void addMaterial(GeoMaterial* matPtr);

    /// @brief: Adds a new Material component to the current factory by name.
    ///         It's first checked whether the name corresponds to an element
    ///         in the Material database otherwise, the Element database is queried.
    void addMatComponent(const std::string &name, double fraction);

    /// @brief  Finalizes the Material definition in the current Material factory.
    void lockMaterial();
    /// @brief  Returns the singelton to this instance of the Material Manager
    static MaterialManager* getManager();
 
    virtual ~MaterialManager();

    using ElementPtr = GeoIntrusivePtr<const GeoElement>;
    using MaterialPtr = GeoIntrusivePtr<GeoMaterial>;
    using ConstMaterialPtr = GeoIntrusivePtr<const GeoMaterial>;

    /// @brief  Small helper struct to build complex materials
    struct MaterialFactory {
       
       
       ~MaterialFactory();
        MaterialFactory(MaterialPtr mat);

        void addComponent(const ConstMaterialPtr& mat, double fraction);
        void addComponent(const ElementPtr& ele, double fraction);

        const GeoMaterial* get() const;
      private:
        using ElementComponent = std::pair<ElementPtr, double>;
        std::vector<ElementComponent> m_components{};
        double m_totFraction{0.};
        MaterialPtr m_material{};
    };

 protected:
    MaterialManager();
    static MaterialManager* s_instance;
  private:
    // Map of elements indexed by Name
    using ElementMap = std::map<std::string, ElementPtr>;
    ElementMap m_elements{};
    
    /// Map of materials indexed by fully qualified name Namespace::MaterialName
    using MaterialMap = std::map<std::string, MaterialPtr>;
    MaterialMap m_materials{};

    /// @brief The namespace under which all materials are being added
    ///        to the Manager. The namespace is as well used if the 
    ///        Material cannot be found in the MaterialMap. Then
    ///        it's prepended to the material name
    std::string m_currentNameSpace{};

    // Build special materials
    void buildSpecialMaterials();

    std::unique_ptr<MaterialFactory> m_factory{};
};

#endif

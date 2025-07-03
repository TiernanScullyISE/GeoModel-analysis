/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelHelpers/MaterialManager.h"


#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/Units.h"

#include "GeoModelKernel/throwExcept.h"
#include "GeoModelHelpers/StringUtils.h"

#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <utility>

namespace {
   std::string replaceDColon(const std::string& name) {
        return GeoStrUtils::replaceExpInString(name, "__dColon__", "::");
   }
   constexpr double atomicDensity = GeoModelKernelUnits::gram / GeoModelKernelUnits::mole;
   constexpr double volDensity = GeoModelKernelUnits::gram / GeoModelKernelUnits::cm3;
   constexpr double invAtomicDensity = 1. / atomicDensity;
   constexpr double invVolDensity = 1. / volDensity;
}


const GeoMaterial* MaterialManager::MaterialFactory::get() const { return m_material; }
MaterialManager::MaterialFactory::~MaterialFactory() {
    if (!m_material) return;

    if (m_components.empty()) {
      m_material->lock();
      return;
    }
    const double inv_totalFraction = 1. / m_totFraction;
    for(const auto& [element, fraction] : m_components) {
      m_material->add(element, fraction * element->getA() * inv_totalFraction);
    }
    m_material->lock();
}

MaterialManager::MaterialFactory::MaterialFactory(MaterialPtr mat):
    m_material{std::move(mat)} {
}

void MaterialManager::MaterialFactory::addComponent(const ConstMaterialPtr& mat, double fraction) {
    for (unsigned int ele = 0 ; ele < mat->getNumElements(); ++ele) {
        ElementPtr elePtr{mat->getElement(ele)};
        addComponent(elePtr, mat->getFraction(ele) * fraction);
    }
}
void MaterialManager::MaterialFactory::addComponent(const ElementPtr& ele, double fraction) {
    m_components.emplace_back(std::make_pair(ele, fraction));
    m_totFraction += fraction;
}

MaterialManager* MaterialManager::s_instance{nullptr};


MaterialManager::MaterialManager() {
    buildSpecialMaterials();
}

MaterialManager::~MaterialManager() {
  s_instance = nullptr;
};

MaterialManager* MaterialManager::getManager() {
  if(!s_instance) {
    s_instance = new MaterialManager();
  }
  return s_instance;
}

bool MaterialManager::isMaterialDefined(const std::string& matName) const {
  return m_materials.find(replaceDColon(matName))!= m_materials.end();
}

const GeoMaterial* MaterialManager::getMaterial(const std::string & name) const {
    //std::cout << "Material Manager: attempt at retrieving material : "<<name<<std::endl;
    // step 1 - check for simple name
    const std::string matName = replaceDColon(name);
    MaterialMap::const_iterator matItr = m_materials.find(matName);
    if (matItr != m_materials.end()) {
        return matItr->second;
    }
    // step 2 - check for material in namespace
    std::string nameNS= m_currentNameSpace+"::"+matName;
    matItr = m_materials.find(nameNS);
    if (matItr != m_materials.end()) {      
      return matItr->second;
    }
    printAll();
    THROW_EXCEPTION("Could not find the material "<<matName);
    return nullptr;
}

const GeoElement* MaterialManager::getElement(const std::string & name) const {
  ElementMap::const_iterator elementIt = m_elements.find(name);
  return  elementIt== m_elements.end() ? nullptr : elementIt->second;
}

const GeoElement* MaterialManager::getElement(unsigned int atomicNumber) const {
    for(const auto& [name, element]: m_elements) {
      if((unsigned)(element->getZ())==atomicNumber) {
        return element;
      }
    }
    return nullptr;
}

void MaterialManager::printAll() const {
  std::cout << "============Material Manager Element List========================" << std::endl;

  for (const auto&[ name,  el]  : m_elements) {    
    std::cout << el->getSymbol() << '\t' << el->getZ() << '\t'
              << el->getA() * invAtomicDensity << '\t' <<name << std::endl;
  }
  for (const auto&[material, mat]: m_materials) {
    std::cout << "Material: " << material <<  " Density " << mat->getDensity() * invVolDensity << std::endl;
    for (size_t i = 0; i< mat->getNumElements(); ++i) {
      std::cout << " ***** ***** "
         << int (mat->getFraction(i)*100) << "% \t"
         << mat->getElement(i)->getName() << std::endl;
    }
  }
}

void MaterialManager::addElement(const std::string &name, const std::string &symbol, double z, double a) {
    auto newElement{make_intrusive<GeoElement>(name,symbol,z,a*atomicDensity)};
    addElement(newElement);
}

void MaterialManager::addElement(const GeoElement* el) {
    GeoIntrusivePtr<const GeoElement> newElement{el};
    auto result = m_elements.insert(std::make_pair(newElement->getName(), newElement));
    if(!result.second) {
      THROW_EXCEPTION("Attempted to redefine element " << newElement->getName());
    }
}


const std::string& MaterialManager::materialNameSpace() const {
   return m_currentNameSpace;
}
void MaterialManager::setMaterialNamespace(const std::string &name) {    
    m_currentNameSpace = name;
    lockMaterial();
}
void MaterialManager::lockMaterial() { m_factory.reset(); }

void MaterialManager::addMaterial(const std::string &name, double density) {
    MaterialPtr newMat{make_intrusive<GeoMaterial>(name, density * volDensity)};
    addMaterial(newMat);
}

void MaterialManager::addMaterial(GeoMaterial* mat) {
  
  std::string matName = replaceDColon(mat->getName());
  if (matName.find("::") == std::string::npos) {
     if (materialNameSpace().empty()) {
         THROW_EXCEPTION("Material " << matName << " has not beed defined within a namespace");
     } else {
        matName = materialNameSpace() + "::" + matName;
     }
  }
  MaterialPtr matPtr{mat};
  auto result = m_materials.insert(std::make_pair(matName, matPtr));
  if(!result.second) {
    THROW_EXCEPTION("Attempted to redefine material " << matName);
  }
  m_factory = std::make_unique<MaterialFactory>(matPtr);
}
bool MaterialManager::isElementDefined(const std::string& eleName) const {
  return m_elements.find(eleName)!=m_elements.end();
}

void MaterialManager::addMatComponent(const std::string &name, double fraction) {
  if (!m_factory){
     THROW_EXCEPTION("No Material factory defiend at the moment");
  }
  std::string cmpName{replaceDColon(name)};
  if(!isMaterialDefined(name) || cmpName.find("::") == std::string::npos) {
    // New components is an element    
    const GeoElement* element = getElement(name);
    m_factory->addComponent(element, fraction);
  } else {   
    const GeoMaterial* material = getMaterial(name);
    m_factory->addComponent(material, fraction);

  }
}
void MaterialManager::buildSpecialMaterials() {
  // Ether  
  GeoIntrusivePtr<GeoElement> ethElement{make_intrusive<GeoElement>("Ether","ET",500.0,0.0)};
  m_elements.insert(std::make_pair("Ether",ethElement));
  GeoIntrusivePtr<GeoMaterial> ether{make_intrusive<GeoMaterial>("special::Ether",0.0)};
  ether->add(ethElement,1.);
  ether->lock();
  m_materials.insert(std::make_pair("special::Ether", ether));
  // HyperUranium
  GeoIntrusivePtr<GeoMaterial> hu{make_intrusive<GeoMaterial>("special::HyperUranium",0.0)};
  hu->add(ethElement,1.);
  hu->lock();
  m_materials.insert(std::make_pair("special::HyperUranium", hu));
}

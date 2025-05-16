/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOVGEOMETRYPLUGIN_H
#define GEOMODELKERNEL_GEOVGEOMETRYPLUGIN_H

/**
 * @class: GeoVGeometryPlugin
 *
 * @brief Abstact Base Class for a geometry plugin.
 *	  The geometry plugin builds the raw geometry
 */

#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoPublisher.h" // to publish lists of FullPhysVol and AlignableTransform nodes in create()

#include <memory> // smart pointers

class GeoPhysVol;
class GeoVGeometryPlugin 
{
 public:

    //! Default constructor.
    GeoVGeometryPlugin() = default;
     
    //! Parametrized constructor for plugins that publish lists of nodes 
    GeoVGeometryPlugin(const std::string& name) : 
        m_pluginName{name} {  m_publisher->setName(m_pluginName); } 
    

    virtual ~GeoVGeometryPlugin()  = default;

    //! Create the system geometry.
    /// Note: this is a pure virtual method, so you need to implement it in your derived plugin class
    virtual void create ( GeoVPhysVol* world, bool publish = false ) = 0;

    //! Returns the plugin's name
    std::string getName() const { return m_pluginName; }

    //! Returns the Publisher that publishes the lists of the GeoFullPhysVol and AlignableTransform nodes
    GeoPublisher* getPublisher() { return m_publisher.get(); }

 protected:

  //! A GeoPublisher instance is used to publish lists of nodes.
  std::unique_ptr<GeoPublisher> m_publisher{std::make_unique<GeoPublisher>()};
  
 private:

  /// We prohibit copy constructor, and assignment operator
  GeoVGeometryPlugin(const GeoVGeometryPlugin &right)=delete;
  GeoVGeometryPlugin & operator=(const GeoVGeometryPlugin &right)=delete;


  //! The name of the plugin, used to store plugin's published nodes. 
  std::string m_pluginName;

};

#endif

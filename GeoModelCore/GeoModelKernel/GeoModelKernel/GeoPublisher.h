/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELKERNEL_GEOSTORE_H
#define GEOMODELKERNEL_GEOSTORE_H

/**
 * @class GeoPublisher
 *
 * @brief Class for a generic GeoModel publisher.
 * The interface allows to store pointers to two types of objects
 *   1. Full Physical Volumes
 *   2. Alignable Transforms
 * and to publish the lists.
 *
 * A pointer to a GeoPublisher object can be passed to GeoVGeometryPlugin::create().
 * This allows for storing pointers to full physical volumes and alignable transforms
 * while the plugin is building raw detector geometries. The stored pointers can
 * be used later for constructing the readout geometry layer, and for applying
 * alignment corrections
 */

// Author: Riccardo Maria Bianchi <riccardo.maria.bianchi@cern.ch> - Aug 2020
// Major updates: 
// - Jan 2021 - riccardo.maria.bianchi@cern.ch - Added support for XML auxiliary data
// - Jun 2026 - riccardo.maria.bianchi@cern.ch - Moved to std::map with user-defined key as "key" and Geo node as "value"


// C++ includes
#include <any> // needs C++17
#include <map>
#include <vector>
#include <unordered_map>
#include <string>
#include <variant>
#include <iostream>



class GeoAlignableTransform;
class GeoVFullPhysVol;


class GeoPublisher
{
 public:
  GeoPublisher() = default;
  virtual ~GeoPublisher() = default;

  using DBRecord = std::variant<int,long,float,double,std::string>;
  using AuxTableDefs = std::unordered_map<std::string, std::pair<std::vector<std::string>, std::vector<std::string>>>;
  using AuxTableData =  std::unordered_map<std::string, std::vector<std::vector<DBRecord>>>;


  template<class N, typename T> void publishNode(N node, T keyT);

  template <typename Node_t> 
  using RecordMap_t = std::map<DBRecord, Node_t>;

  RecordMap_t<GeoVFullPhysVol*> getPublishedFPV() const ;
  RecordMap_t<GeoAlignableTransform*> getPublishedAXF() const;

  void setName(const std::string& name);
  std::string getName()  const { return m_name; }

  void storeDataTable(const std::string& tableName, 
                      const std::vector<std::string>& colNames, 
                      const std::vector<std::string>& colTypes, 
                      std::vector<std::vector<DBRecord>> tableData );

    std::pair<AuxTableDefs, AuxTableData> getPublishedAuxData() const { 
      return std::make_pair(m_auxiliaryTablesVar, m_auxiliaryTablesVarData); 
    }

 private:
    template <typename Key_t> 
      /** @brief Checks whether the AlignableNode / FullPhysVol has already
       *         been published under the given record
       *  @param storeage: Map to scan
       *  @param record: The record to check (the user-defined 'key')
       *  @param node: Pointer to the AlignableNode / FullPhysVol
       **/
      bool containsRecord(const RecordMap_t<Key_t>& storeage,
                          const DBRecord record,
                          const Key_t node
                          ) const;

  RecordMap_t<GeoVFullPhysVol*> m_publishedFPV{};
  RecordMap_t<GeoAlignableTransform*> m_publishedAXF{};

  std::string m_name{};

  // cache to store custom tables to store auxiliary data in the DB:
  // ---> map( tableName, columnsNames, columnsTypes )
  AuxTableDefs m_auxiliaryTablesVar{};
  AuxTableData m_auxiliaryTablesVarData{};


}; 

// include the implementation of the class' templated member functions
#include "GeoPublisher.tpp"

#endif

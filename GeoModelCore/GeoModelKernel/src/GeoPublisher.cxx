/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Author: Riccardo Maria Bianchi @ CERN
// August 2020


// local includes
#include <GeoModelKernel/GeoPublisher.h>
#include <GeoModelKernel/GeoVFullPhysVol.h>
#include <GeoModelKernel/GeoAlignableTransform.h>

// C++ includes
#include <iostream>
#include <utility>



std::map<GeoVFullPhysVol*, std::any> GeoPublisher::getPublishedFPV() const {
    return m_publishedFPV;
}

std::map<GeoAlignableTransform*, std::any> GeoPublisher::getPublishedAXF() const {
    return m_publishedAXF;
}


void GeoPublisher::setName(const std::string& name) {
    m_name = name;
}

/*
void GeoPublisher::storeDataTable( std::string tableName, std::vector<std::string> colNames, std::vector<std::string> colTypes, std::vector<std::vector<std::string>> tableData )
{
    m_auxiliaryTablesStr[ tableName ] = std::make_pair(colNames, colTypes);
    m_auxiliaryTablesStrData[ tableName ] = tableData;
}
*/

void GeoPublisher::storeDataTable( const std::string& tableName, 
                                    const std::vector<std::string>& colNames, 
                                    const std::vector<std::string>& colTypes, 
                                    std::vector<std::vector<DBRecord>> tableData )
{
    m_auxiliaryTablesVar[ tableName ] = std::make_pair(colNames, colTypes);
    m_auxiliaryTablesVarData[ tableName ] = std::move(tableData);
}


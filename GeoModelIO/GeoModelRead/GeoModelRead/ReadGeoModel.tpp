/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/* vim: set ft=cpp: */ // VIM modeline options
/*
 * ReadGeoModel.tpp
 *
 * Created on: Sep 8, 2020
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 *
 * The file contains the definitions of the templated functions of the ReadGeoModel class.
 *
 */

#include "GeoModelKernel/GeoFullPhysVol.h"

#include "GeoModelHelpers/variantHelpers.h"
#include "GeoModelKernel/throwExcept.h"



namespace GeoModelIO {

    template <typename T, class N> std::map<T,N> 
        ReadGeoModel::getPublishedNodes(std::string publisherName, bool doCheckTable) const {
 

        std::map<T, N> mapNodes;
        std::string keyType = "";

        DBRowsList vecRecords;

        static_assert(std::is_same_v<GeoFullPhysVol*, N> || std::is_same_v<GeoAlignableTransform*, N> ,
                    "ERROR! The node type is not currently supported. If in doubt, please ask to 'geomodel-developers@cern.ch'.\n");

        if constexpr( std::is_same_v<GeoFullPhysVol*, N> ) {
            if(doCheckTable){ 
                bool tableExists = m_dbManager->checkTableFromDB("PublishedFullPhysVols_"+publisherName);
                if(!tableExists) return mapNodes;
            }
            vecRecords = m_dbManager->getPublishedFPVTable( publisherName );
        } else if constexpr ( std::is_same_v<GeoAlignableTransform*, N> ) {
            if(doCheckTable){ 
                bool tableExists = m_dbManager->checkTableFromDB("PublishedAlignableTransforms_"+publisherName);
                if(!tableExists) return mapNodes;
            }
            vecRecords = m_dbManager->getPublishedAXFTable( publisherName );
        }
        unsigned ii = 0;
        for( auto const &record : vecRecords ) {
            // record[0] is the record's ID in the DB table, we skip that.
            const std::string keyStr  = GeoModelHelpers::variantHelper::getFromVariant_String(record[1], "getPublishedNodes:keyStr");
            const unsigned volID   = GeoModelHelpers::variantHelper::getFromVariant_Int(record[2], "getPublishedNodes:volID");
            if(0==ii) keyType   = GeoModelHelpers::variantHelper::getFromVariant_String(record[3], "getPublishedNodes:keyType"); //this is the same for all records. TODO: it should be stored in a metadata table
            ++ii;
            N volPtr = nullptr;
            if constexpr ( std::is_same_v<GeoFullPhysVol*, N> ) {
                volPtr = getBuiltFullPhysVol(volID);
            } else if constexpr ( std::is_same_v<GeoAlignableTransform*, N> ) {
                volPtr = getBuiltAlignableTransform(volID);
            } else {
                static_assert(false, "Unsupported type");
            }
            if (!volPtr) {
                THROW_EXCEPTION("Failed to fetch element "<<volID<<" of type "<<typeid(N).name());
            }

            //TODO: check if we can get rid of stoul/stoi...
            if constexpr ( std::is_same_v<unsigned, T> ) {
                unsigned int key = std::stoul( keyStr );
                mapNodes.insert( {key, volPtr} );
            } 
            else if constexpr ( std::is_same_v<int, T> ) {
                int key = std::stoi( keyStr );
                mapNodes.insert( {key, volPtr} );
            } 
            else if constexpr ( std::is_same_v<std::string, T> ) {
                // OK! key is string already, so we use keyStr.
                mapNodes.insert( {keyStr, volPtr} );
            } 
            else {
                const std::string errMsg = "ERROR! Key type '" + keyType + "' is not currently supported.\n"
                          + "For the moment, unsigned int, int, and string are supported.\n" 
                          + "If in doubt, please ask to 'geomodel-developers@cern.ch'.\n"
                          + "Exiting...\n";
                THROW_EXCEPTION(errMsg);
            }
        }
        return mapNodes;
    }

} // closes namespace

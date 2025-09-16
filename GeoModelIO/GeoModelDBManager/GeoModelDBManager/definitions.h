// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/*
 * This header file provides global definitions used 
 * in the I/O of GeoModel into and from an SQLite file. 
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: May, 2024
 *
 *  Updated: 
 *  - Sep 2025, riccardo.maria.bianchi@cern.ch
 *    Added data type for published FPV and AXF records
 *            
 *
 */


#ifndef GEOMODELDBMANAGER_DEFINITIONS_H
#define GEOMODELDBMANAGER_DEFINITIONS_H

#include <vector>
#include <variant>
#include <string>
// define the data types used to retrieve and store data from the DB
using DBRecordEntry = std::variant<int,long,float,double,std::string>;
using DBRowEntry = std::vector<DBRecordEntry>;
using DBRowsList = std::vector<DBRowEntry>;

#endif

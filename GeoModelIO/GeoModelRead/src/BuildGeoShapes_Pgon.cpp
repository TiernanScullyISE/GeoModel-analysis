/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_Pgon.h"

#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelHelpers/variantHelpers.h"
#include "GeoModelKernel/throwExcept.h"

#include <vector>
#include <iostream>

namespace GeoModelIO {

BuildGeoShapes_Pgon::BuildGeoShapes_Pgon(DBRowsList&& allPconData,
                                        DBRowsList&& allVertexData):
    BuildGeoShapes(GeoPgon::getClassType(), std::move(allPconData), std::move(allVertexData)){}
void BuildGeoShapes_Pgon::buildShape(const DBRowEntry row) {
    if (m_auxillaryData.empty()) {
        THROW_EXCEPTION("ERROR! GeoPgon shape has no ZPlanes data!! [m_shape_data.size() == 0]");
    }

    // === get shape numeric data from the DB row
    // shape ID
    const int shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Pgon:shapeID");
    // shape volume
    const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Pgon:shapeVolume");
    // shape parameters
    const double SPhi = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "Pgon:SPhi");
    const double DPhi = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "Pgon:DPhi");
    const int NSides = GeoModelHelpers::variantHelper::getFromVariant_Int(row[4], "Pgon:NSides");
    const int NZPlanes = GeoModelHelpers::variantHelper::getFromVariant_Int(row[5], "Pgon:NZPlanes");
    // pointers to variable shape data stored in a separate table
    const int dataStart = GeoModelHelpers::variantHelper::getFromVariant_Int(row[6], "Pgon:dataStart");
    const int dataEnd = GeoModelHelpers::variantHelper::getFromVariant_Int(row[7], "Pgon:dataEnd");

    // build the basic GeoPgon shape
    auto shape = make_intrusive<GeoPgon>(SPhi, DPhi, NSides);

    // and now loop over the additional shape's data, 
    // to get the parameters of all Z planes

    // extract subvector
    // NOTE: we use (dataStart-1) to cope with the difference between the DB rows starting from '1', 
    //       which is what the 'dataStart' stores, and the vector items, which start '0'; 
    //       also, the constructor of the sub-vector takes the element from 'begin+dataStart-1' included
    //       and 'begin+dataEnd' excluded.
    assert(dataStart> 0 && dataEnd<= m_auxillaryData.size());
    const DBRowsList zplanesData(m_auxillaryData.begin() + (dataStart-1),
                                 m_auxillaryData.begin() + (dataEnd) );
    
    if (zplanesData.empty()) {
        THROW_EXCEPTION("ERROR! GeoPgon shape ZPlanes data have not been retrieved!!");
    }
    if (NZPlanes != zplanesData.size()) {
        THROW_EXCEPTION("ERROR! GeoPgon shape : size of ZPlanes data does not correspond to the number of ZPlanes declared!!");
    }
    // loop over the data defining the ZPlanes
    for (const DBRowEntry &dataRow : zplanesData){
        const double zpos = GeoModelHelpers::variantHelper::getFromVariant_Double(dataRow[1], "Pgon:data_ZPos");
        const double rmin = GeoModelHelpers::variantHelper::getFromVariant_Double(dataRow[2], "Pgon:data_ZRMin");
        const double rmax = GeoModelHelpers::variantHelper::getFromVariant_Double(dataRow[3], "Pgon:data_ZRMax");
        // add a Z plane to the GeoPgon
        shape->addPlane(zpos, rmin, rmax);
    }

    // sanity checks on the resulting Pgon shape
    if (shape->getNPlanes() != NZPlanes) {
        THROW_EXCEPTION("ERROR! GeoPgon actual number of planes: "<<shape->getNPlanes()<< 
                        " is not equal to the original size! --> "<<NZPlanes);
    }
    if (!shape->isValid()){
        THROW_EXCEPTION("ERROR! GeoPgon shape is not valid!!");
    }
    storeBuiltShape(shapeId, std::move(shape));
}
}
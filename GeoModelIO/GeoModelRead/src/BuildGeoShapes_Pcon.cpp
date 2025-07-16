/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Created on: May 7, 2024
 * Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

#include "BuildGeoShapes_Pcon.h"

#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelHelpers/variantHelpers.h"
#include "GeoModelKernel/throwExcept.h"

#include <vector>
#include <iostream>

namespace GeoModelIO {

BuildGeoShapes_Pcon::BuildGeoShapes_Pcon(DBRowsList&& allPconData,
                                        DBRowsList&& allVertexData):
    BuildGeoShapes{GeoPcon::getClassType(), std::move(allPconData), 
                   std::move(allVertexData)} {}

void BuildGeoShapes_Pcon::buildShape(const DBRowEntry row) {
    if (m_auxillaryData.empty()) {
        THROW_EXCEPTION("ERROR! GeoPcon shape has no ZPlanes data!! [m_shape_data.size() == 0]");
    }

    // === get shape numeric data from the DB row
    // shape ID
    const int shapeId = GeoModelHelpers::variantHelper::getFromVariant_Int(row[0], "Pcon:shapeID");
    // shape volume
    const double shapeVolume = GeoModelHelpers::variantHelper::getFromVariant_Double(row[1], "Pcon:shapeVolume");
    // shape parameters
    const double SPhi = GeoModelHelpers::variantHelper::getFromVariant_Double(row[2], "Pcon:SPhi");
    const double DPhi = GeoModelHelpers::variantHelper::getFromVariant_Double(row[3], "Pcon:DPhi");
    const int NZPlanes = GeoModelHelpers::variantHelper::getFromVariant_Int(row[4], "Pcon:NZPlanes");
    // pointers to variable shape data stored in a separate table
    const int dataStart = GeoModelHelpers::variantHelper::getFromVariant_Int(row[5], "Pcon:dataStart");
    const int dataEnd = GeoModelHelpers::variantHelper::getFromVariant_Int(row[6], "Pcon:dataEnd");

    // build the basic GeoPcon shape
    auto pcon = make_intrusive<GeoPcon>(SPhi, DPhi);

    // and now loop over the additional shape's data, 
    // to get the parameters of all Z planes

    // get ZPlanes' data, extract subvector
    // NOTE: we use (dataStart-1) to cope with the difference between the DB rows starting from '1', 
    //       which is what the 'dataStart' stores, and the vector items, which start '0'; 
    //       also, the constructor of the sub-vector takes the element from 'begin+dataStart-1' included
    //       and 'begin+dataEnd' excluded.
    assert(dataStart > 0 && dataEnd <= m_auxillaryData.size());
    const DBRowsList zplanesData(m_auxillaryData.begin() + (dataStart-1),
                                 m_auxillaryData.begin() + (dataEnd) );
    if (zplanesData.empty()) {
        THROW_EXCEPTION("ERROR! GeoPcon shape ZPlanes data have not been retrieved!!");
    }
    if (NZPlanes != zplanesData.size()) {
        THROW_EXCEPTION("ERROR! GeoPcon shape : size of ZPlanes data does not correspond to the number of ZPlanes declared!!");
    }
    // loop over the data defining the ZPlanes
    for (const DBRowEntry &dataRow : zplanesData){
        const double zpos = GeoModelHelpers::variantHelper::getFromVariant_Double(dataRow[1], "Pcon:data_ZPos");
        const double rmin = GeoModelHelpers::variantHelper::getFromVariant_Double(dataRow[2], "Pcon:data_RMin");
        const double rmax = GeoModelHelpers::variantHelper::getFromVariant_Double(dataRow[3], "Pcon:data_RMax");
        // add a Z plane to the GeoPcon
        pcon->addPlane(zpos, rmin, rmax);
    }

    // sanity checks on the resulting Pcon shape
    if (pcon->getNPlanes() != NZPlanes) {
        THROW_EXCEPTION("ERROR! GeoPcon actual number of planes: "<<pcon->getNPlanes()
                     <<" is not equal to the original size! --> "<<NZPlanes);
    }
    if (!pcon->isValid()) {
        THROW_EXCEPTION("ERROR! GeoPcon shape is not valid!!");
    }

    storeBuiltShape(shapeId, std::move(pcon));
}
}
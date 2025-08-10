/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration   
*/


#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelHelpers/StringUtils.h"
#include <iostream>

#include <unistd.h>
#include <stdlib.h>

using namespace GeoStrUtils;

int main(int argc, char ** argv) {
    unsigned nTrials{50};
    std::string dbFile{};

    for (int arg = 1; arg < argc; ++arg) {
        std::string the_arg{argv[arg]};
        
        if (the_arg == "--geoFile" && arg + 1 < argc) {
            dbFile = argv[arg+1];
            ++arg;
        } else if (the_arg == "--nTrials" && argc + 1 < argc) {
            nTrials = atoi(argv[arg+1]);
            ++arg;
        } else {
            std::cerr<<"Unsupported argument "<<the_arg<<std::endl;
            return EXIT_FAILURE;
        }
    }
    if (dbFile.empty()) {
        std::cerr<<"Database file must not be empty "<<std::endl;
        return EXIT_FAILURE;
    }
    if (!nTrials){
        std::cerr<<"No trials? "<<std::endl;
        return EXIT_FAILURE;
    }

    while ((nTrials--) > 0) {
        auto gmMgr = std::make_unique<GMDBManager>(dbFile);
        auto readGeo = std::make_unique<GeoModelIO::ReadGeoModel>(std::move(gmMgr));

        PVConstLink world = readGeo->buildGeoModel();
        if (!world) {
             return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration   
*/


#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelHelpers/StringUtils.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoSerialIdentifier.h"
#include "GeoModelKernel/GeoSerialDenominator.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoUnidentifiedShape.h"

#include "GeoModelHelpers/printVolume.h"
#include "GeoModelHelpers/ThreadPool.h"

#include "GeoModelHelpers/TransformSorter.h"
#include "GeoModelHelpers/TransformToStringConverter.h"
#include "GeoModelHelpers/GeoShapeSorter.h"
#include "GeoModelHelpers/GeoMaterialSorter.h"
#include <iostream>

#include <unistd.h>
#include <stdlib.h>

using namespace GeoStrUtils;


#define COMPARE_PROPERTY(CLASS_ID, GETTER_METHOD) \
    if (const auto* castNodeA = dynamic_cast<const CLASS_ID*>(nodeA); castNodeA != nullptr ) { \
        const auto* castNodeB = dynamic_cast<const CLASS_ID*>(nodeB);             \
        if (castNodeA->GETTER_METHOD() != castNodeB->GETTER_METHOD()) {           \
            std::cerr<<__FILE__<<": "<<__LINE__<<" - "<<#CLASS_ID<<" differs in " \
                     <<#GETTER_METHOD<<" - "<<castNodeA->GETTER_METHOD()<<" vs. " \
                     <<castNodeB->GETTER_METHOD()<<"."<<std::endl;                \
        } else { continue; } \
    }
/** @brief Checks whether two physical volumes are equal in terms of their tree 
 *         structure -> Same number of child nodes and the i-th node of
 *         both volumes is also the same
 *  @param volA: Reference to volume A for comparison
 *  @param volB: Reference to volume B for comparison  */
bool equalVolumes(const PVConstLink& volA, const PVConstLink& volB) {
    if (!volA) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Volume A not given"<<std::endl;
        return false;
    }
    if (!volB) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Volume B not given"<<std::endl;
        return false;
    }
    if (volA == volB) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" The nodes must not be identical"<<std::endl;
        return false;
    }
    if (volA->getNChildNodes() != volB->getNChildNodes()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Volume A \n "<<printNodes(volA)
                    <<"\n\n is different from volume B \n "<<printNodes(volB)<<std::endl;
        return false;
    }
    {
        static const GeoShapeSorter shapeSorter{};
        static const GeoMaterialSorter matSorter{};
        const auto* logVolA = volA->getLogVol();
        const auto* logVolB = volB->getLogVol();
        if (matSorter.compare(logVolA->getMaterial(), logVolB->getMaterial())){
            std::cerr<<__FILE__<<":"<<__LINE__<<" The volumes have different material "<<std::endl;
            return false;
        }
        const auto* shapeA = logVolA->getShape();
        const auto* shapeB = logVolB->getShape();
        if (shapeA->typeID() != shapeB->typeID()) {
            std::cerr<<__FILE__<<":"<<__LINE__<<" The shapes are different "<<shapeA->type()
                <<" vs. "<<shapeB->type()<<std::endl;
            return false;
        }
        if (shapeA->typeID() != GeoUnidentifiedShape::getClassTypeID() && 
            shapeSorter.compare(shapeA, shapeB) != 0) {
            std::cerr<<__FILE__<<":"<<__LINE__<<" The logical volumes differ "<<std::endl;
            return false;
        }
    }

    for (unsigned node = 0 ; node < volA->getNChildNodes(); ++node) {
        const GeoGraphNode* nodeA = *volA->getChildNode(node);
        const GeoGraphNode* nodeB = *volB->getChildNode(node);
        if (typeid(*nodeA) != typeid(*nodeB)) {
            std::cerr<<__FILE__<<":"<<__LINE__<<" Types of the "<<node
                    <<"-th node are different "<<typeid(*nodeA).name()
                    <<" vs. "<<typeid(*nodeB).name()<<std::endl;
            std::cerr<<printNodes(volA)<<std::endl;
            std::cerr<<"\n\n"<<printNodes(volB)<<std::endl;
            return false;
        }
        if(const auto* trfA = dynamic_cast<const GeoTransform*>(nodeA); trfA != nullptr) {
            const auto* trfB = static_cast<const GeoTransform*>(nodeB);
            static const GeoTrf::TransformSorter sorter{};
            if (sorter.compare(trfA->getDefTransform(), trfB->getDefTransform()) != 0) {
                std::cerr<<__FILE__<<":"<<__LINE__<<" The transforms differ "
                    <<GeoTrf::toString(trfA->getDefTransform())<<" vs. "
                    <<GeoTrf::toString(trfB->getDefTransform())<<". "<<std::endl;
                return false;
            } else { continue; }
        }  
        if (const auto* chVolA = dynamic_cast<const GeoVPhysVol*>(nodeA); chVolA != nullptr) {
            const auto* chVolB = dynamic_cast<const GeoVPhysVol*>(nodeB);
            if (!equalVolumes(chVolA, chVolB)) {
                std::cerr<<__FILE__<<":"<<__LINE__<<" The "<<node<<"-th volume A \n "
                    <<"\n\n is different volume B \n "<<std::endl;
                return false;
            } else { continue; }
        }
        if (const auto* serialTrfA = dynamic_cast<const GeoSerialTransformer*>(nodeA); serialTrfA != nullptr) {
            const auto* serialTrfB = static_cast<const GeoSerialTransformer*>(nodeB);
            if (!equalVolumes(serialTrfA->getVolume(), serialTrfB->getVolume())) {
                std::cerr<<__FILE__<<":"<<__LINE__<<" The serial ID volumes differ" <<std::endl;
                return false;
            } else if (serialTrfA->getNCopies() != serialTrfB->getNCopies()) {
                std::cerr<<__FILE__<<":"<<__LINE__<<" The number of serial copies differ"<<std::endl;
                return false;
            }
            static const GeoTrf::TransformSorter sorter{};
            for (unsigned c = 0; c < serialTrfA->getNCopies(); ++c) {
                if (sorter.compare(serialTrfA->getTransform(c), 
                                   serialTrfB->getTransform(c)) != 0) {
                    std::cerr<<__FILE__<<":"<<__LINE__<<" The "<<c<<"-th volume of the "
                             <<node<<"-th child node differs "
                             <<GeoTrf::toString(serialTrfA->getTransform(c))<<" vs. "
                             <<GeoTrf::toString(serialTrfB->getTransform(c))<<std::endl;
                    return false;
                }
            }
        }
        COMPARE_PROPERTY(GeoIdentifierTag, getIdentifier);
        COMPARE_PROPERTY(GeoNameTag, getName);
        COMPARE_PROPERTY(GeoSerialDenominator, getBaseName);
        COMPARE_PROPERTY(GeoSerialIdentifier, getBaseId);
 
    }
    return true;
}



int main(int argc, char ** argv) {
    unsigned nTrials{50};
    int nThreads{-1};
    std::string dbFile{};

    for (int arg = 1; arg < argc; ++arg) {
        std::string the_arg{argv[arg]};
        
        if (the_arg == "--geoFile" && arg + 1 < argc) {
            dbFile = argv[arg+1];
            ++arg;
        } else if (the_arg == "--nTrials" && arg + 1 < argc) {
            nTrials = atoi(argv[arg+1]);
            ++arg;
        } else if (the_arg == "--nThreads" && arg + 1 < argc) {
            nThreads = atoi(argv[arg+1]);
            ++arg;
        } else {
            std::cerr<<"Unsupported argument "<<the_arg<<std::endl;
            std::cerr<<"Supported arguments are \n"
            <<"   --geoFile: <Location of the geometry file \n"
            <<"   --nTrials: <Number of trials to read in the geometry> \n"
            <<"   --nThreads: <Number of threads to use for the geometry reading \n"<<std::endl;
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
    PVConstLink refWorld{};
    while ((nTrials--) > 0) {
        GeoThreading::ThreadPool::getPool(nThreads);
        auto gmMgr = std::make_unique<GMDBManager>(dbFile);
        auto readGeo = std::make_unique<GeoModelIO::ReadGeoModel>(std::move(gmMgr));
        readGeo->inspectChildConnection();
        PVConstLink world = readGeo->buildGeoModel();
        if (!world) {
             return EXIT_FAILURE;
        }
        std::cout<<"ReadStabilityTest() - World "<<(nTrials)<<" loaded "<<std::endl;
        if (!refWorld) {
            refWorld = world;
        } else if (!equalVolumes(refWorld, world)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelHelpers/GeoPhysVolSorter.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/throwExcept.h"

#include "GeoModelHelpers/TransformSorter.h"
#include "GeoModelHelpers/GeoLogVolSorter.h"
#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelHelpers/getChildNodesWithTrf.h"



int GeoPhysVolSorter::compare(const GeoVPhysVol* a, const GeoVPhysVol* b) const {
    /// If one of the given volumes is a full physical volume, let's assume that they've been
    /// put on purpose into the node. Ensure that they're added to the set eventhough they
    /// are equivalent in the terms of this sorter.
    if (typeid(*a) == typeid(GeoFullPhysVol) || typeid(*b) == typeid(GeoFullPhysVol)) {
        return a == b ? 0 : (a < b ? -1 : 1);
    }
    
    /// A check on different logical volumes is already a good start
    {
        static const GeoLogVolSorter sorter{};
        const int shapeCmp = sorter.compare(a->getLogVol(), b->getLogVol());
        if (shapeCmp) return shapeCmp;
    }

    static const GeoTrf::TransformSorter sorter{};

    GeoVolumeCursor cursA{a}, cursB{b};
    while (!cursA.atEnd() && !cursB.atEnd()) {
        
        GeoChildNodeWithTrf childA{cursA};
        GeoChildNodeWithTrf childB{cursB};
        cursA.next();
        cursB.next();
        /// Check whether there's an alignable transform somewhere
        if (childA.isAlignable != childB.isAlignable) {
            return childA.isAlignable ? -1 : 1;
        }
        /// Check whether the voumes are full physical volumes
        if (childA.isSensitive || childB.isSensitive) {
            return childA.volume < childB.volume ? - 1 : 1;
        }
        /// Check equivalance of the transformations
        const int transCmp = sorter.compare(childA.transform,
                                            childB.transform);
        if (transCmp) return transCmp;
        /// Every day greets the marmot
        const int childCmp = compare(childA.volume, childB.volume);
        if (childCmp) return childCmp;
    }
    if (cursA.atEnd() != cursB.atEnd()) {
        if (cursA.atEnd()) return -1;
        return 1;
    }
    return 0;
}
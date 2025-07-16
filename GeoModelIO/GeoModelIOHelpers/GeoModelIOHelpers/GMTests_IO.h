// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/*
 * This header file provides helper functions to perform different tests on an existing GeoModel tree.
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: Feb, 2023
 *
 */

#ifndef GMTESTS_IO_H
#define GMTESTS_IO_H

#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModelIOHelpers/GMIO.h"

namespace GeoModelIO {

class TestIO {
   public:
    TestIO(){};

    /// This function tests the number of nodes in the workflow
    /// Write-Read-Write. In this test, a GeoVPhysVol is taken as a 'world'
    /// volume and the GeoModel tree is persistified into a DB file. Then the
    /// file is loaded and the GeoModel tree restored. Then the in-memory
    /// GeoModel tree is persistified again into a new DB file.
    /// This tests the IO tools that dump the in-memory GeoModel tree to file,
    /// the ones that restore it from the DB file into memory, and the DB I/O
    /// methods.
    /// The number of nodes are taken from the DB, by using the tools offered by
    /// the DB Manager.
    static std::pair<std::string, bool> test_compareWriteReadWriteFromVol(const PVConstLink world, 
                                                                          const unsigned loglevel = 0, 
                                                                          const bool forceDelete = false) {
        std::string testname = "compareWriteReadWriteFromVol";
        if (loglevel >= 1) {
            std::cout << "\n-- Test: '" << testname << "' -- \n";
        }
        // test DBs
        const std::string db1_name = "geometry_db_test_1.db";
        const std::string db2_name = "geometry_db_test_2.db";

        std:: cout << "1 -- Writing the geometry to file - 1st time" << std::endl;
        auto db1 = GeoModelIO::IO::saveToDB(world, db1_name, loglevel, true);

        std::cout << "2 -- Get number of nodes from the DB we just saved" << std::endl;
        auto mmap_1 = GeoModelIO::IO::countNodesFromDB(*db1);

        std::cout << "3 -- Reading back the geometry from file" << std::endl;
        PVConstLink world2 = GeoModelIO::IO::loadDB(db1_name, loglevel);

        std::cout << "4 -- Writing the geometry to file - 2nd time" << std::endl;
        auto db2 = GeoModelIO::IO::saveToDB(world2, db2_name, loglevel, true);

        std::cout << "5 -- Get number of nodes from the DB we created from the restored GeoModel tree" << std::endl;
        auto mmap_2 = GeoModelIO::IO::countNodesFromDB(*db2);

        std::cout <<  "6 -- Comparison" << std::endl;
        bool testok = false;
        assert(mmap_1.size() == mmap_2.size());
        if (mmap_1 == mmap_2) {
            if (loglevel >= 1) {
                std::cout
                    << "OK! The number of nodes that were persistified is "
                       "equal to the "
                       "number of restored-then-persistified-again "
                       "(Write-Read-Write) nodes.\n";
                std::cout << "----\n";
            }
            testok = true;
        } else {
            if (loglevel >= 1) {
                std::cout << "!!! ERROR !!! The number of nodes that were "
                             "persistified is "
                             "NOT equal to the "
                             "number of restored-then-persistified-again "
                             "(Write-Read-Write) nodes!!!\n";
                if (loglevel >= 2) {
                    GeoModelIO::IO::printCompareTwoNodesMaps(mmap_1, mmap_2);
                }
                GeoModelIO::IO::printDifferencesBetweenTwoNodesMaps(mmap_1, mmap_2);
                std::cout << "----\n";
            }
            testok = false;
        }
        return std::make_pair(testname, testok);
    }

    /// This functions compares the number of nodes of the in-memory GeoModel
    /// tree to the number of the saved-then-restored nodes. The number of nodes
    /// are taken from the cache of the GeoNodeAction that is used to traverse
    /// the GeoModel tree during the I/O operation, instead of being taken from
    /// the DB.
    static std::pair<std::string, bool> test_compareMemoryVsRestoredFromVol(const PVConstLink world, 
                                                                            const unsigned loglevel = 0) {
        std::string testname = "compareMemoryVsRestoredFromVol";
        if (loglevel >= 1) {
            std::cout << "\n-- Test: '" << testname << "' -- \n";
            std::cout << "1 -- Objects in the starting in-memory 'world' volume..."<< std::endl;
        }
        // printCountWorldMemoryObjects(world);
        auto mmap_1 = GeoModelIO::IO::countTreeMemoryNodesFromVolume(world);
        if (loglevel >= 1) {
            std::cout << "2 -- Saving the in-memory 'world' volume into a .db "
                         "file and restoring the GeoModel tree from it..."
                      << std::endl;
        }
        std::string dbname = "test_geometry_db_memory.db";
        auto db = GeoModelIO::IO::saveToDB(world, dbname, loglevel, true);
        PVConstLink world2 = GeoModelIO::IO::loadDB(dbname, loglevel);
        if (loglevel >= 1) {
            std::cout << "3 -- Objects in the restored in-memory 'world' volume..."<< std::endl;
        }
        // printCountWorldMemoryObjects(world2);
        auto mmap_2 = GeoModelIO::IO::countTreeMemoryNodesFromVolume(world2);
        // TODO: remove temporary .db files when not needed anymore
        // TODO: add number-by-number comparison as with the other tests!!

        bool testok = false;
        assert(mmap_1.size() == mmap_2.size());
        if (mmap_1 == mmap_2) {
            if (loglevel >= 1) {
                std::cout << "OK! The number of starting in-memory nodes is "
                             "equal to the number of the restored in-memory nodes.\n";
                std::cout << "----\n";
            }
            testok = true;
        } else {
            if (loglevel >= 1) {
                std::cout << "!!! ERROR !!! The number of the starting in-memory nodes is DIFFERENT from "
                          << "the number of the restored in-memory nodes!!!\n";
                if (loglevel >= 2) {
                    std::cout << "*** Starting in-memory nodes:\n";
                    GeoModelIO::IO::printNodesMap(mmap_1);
                    std::cout << "*** Restored in-memory nodes:\n";
                    GeoModelIO::IO::printNodesMap(mmap_2);
                }
                GeoModelIO::IO::printDifferencesBetweenTwoNodesMaps(mmap_1, mmap_2);
                std::cout << "----\n";
            }
            testok = false;
        }
        return std::make_pair(testname, testok);
    }

    /// Utility function to print the number of nodes of a GeoModel tree,
    /// starting from a GeoVPhysVol taken as a 'world' volume.
    static void printCountWorldMemoryObjects(const PVConstLink world) {
        GeoModelIO::IO::printNodesMap(GeoModelIO::IO::countTreeMemoryNodesFromVolume(world));
    }

    static std::pair<bool, std::map<std::string, bool>> runAllTests(PVConstLink world, 
                                                                    const unsigned loglevel = 0, 
                                                                    const unsigned printtests = 0) {
        std::pair<bool, std::map<std::string, bool>> tests;
        bool testall = false;
        // TEST I/O: loading/restoring
        std::pair<std::string, bool> test1 =
            GeoModelIO::TestIO::test_compareWriteReadWriteFromVol(world, loglevel);
        std::pair<std::string, bool> test2 =
            GeoModelIO::TestIO::test_compareMemoryVsRestoredFromVol(world, loglevel);
        // TEST I/O: DB
        if (loglevel > 0) {
            std::cout << "\nSaving the 'world' volume to a .db file, for tests...\n";
        }
        // set an overall test return status, if all tests passed
        if (test1.second && test2.second) {
            testall = true;
        }
        // gather and return results
        std::map<std::string, bool> tmap;
        tmap[test1.first] = test1.second;
        tmap[test2.first] = test2.second;
        tests.first = testall;
        tests.second = tmap;
        if (printtests >= 1) {
            std::cout << "\n---\nThe overall result of the tests is: "
                      << testall << " [" << (testall ? "PASSED." : "FAILED!")
                      << "] \n";
            std::cout << "Single results: \n";
            for (auto& tt : tests.second) {
                std::cout << "test: '" << tt.first << "' ==> " << tt.second
                          << " [" << (tt.second ? "PASSED." : "FAILED!") << "]"
                          << " -- [overall: " << (testall ? "PASSED." : "FAILED!") << "]"
                          << std::endl;
            }
        }
        return tests;
    }
};  // end class
}  // namespace GeoModelIO

#endif

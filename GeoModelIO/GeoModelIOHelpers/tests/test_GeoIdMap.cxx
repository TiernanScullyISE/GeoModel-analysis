// ThreadPoolTest.cpp

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include "GeoModelHelpers/ThreadPool.h"
#include "GeoModelRead/GeoIdObjMap.h"

#include <ranges>
#include <algorithm>
#include <unordered_set>
using namespace std::chrono_literals;

using TestMap_t = GeoModelIO::GeoIdObjMap<unsigned>;

constexpr unsigned nElem = 1000000;
constexpr unsigned nBatches = 50;
constexpr unsigned batchSize = nElem / nBatches;

class GeoIdTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure we start with a fresh pool for each test
        GeoThreading::ThreadPool::getPool(-1);
    }

};


TEST_F(GeoIdTest, ConcurrentFilling) {
    auto& pool = GeoThreading::ThreadPool::getPool();
    TestMap_t fillMe{};

    for (unsigned start = 0; start < nElem; ) {
        unsigned int end = std::min(nElem, start + batchSize);
        pool.appendTask([start,end, &fillMe](){
            for (unsigned int i = start; i < end; ++i) {
                EXPECT_TRUE(fillMe.insert(std::make_pair(i, i+1)));
            }
        });
        start = end;
    }
    pool.drainQueue(); // Ensure task runs
    EXPECT_EQ(fillMe.size(), nElem);
    auto keys = fillMe.keys();
    EXPECT_EQ(keys.size(), nElem);
    std::ranges::sort(keys, [](const auto& a, const auto&b){ return a <b;});

    for (unsigned int k = 0; k < keys.size() ; ++k) {
        EXPECT_EQ(keys[k], k);
        EXPECT_EQ(fillMe.get(k), k+1);
    }
}
TEST_F(GeoIdTest, DuplicateRemoval) {
    auto& pool = GeoThreading::ThreadPool::getPool();

    const std::unordered_set<unsigned> preFilled{666, 259, 202, 630, 25220,
                                                 6392,621, 702}; 
    TestMap_t fillMe{};
    for (const unsigned preFill : preFilled) {
        fillMe.insert(std::make_pair(preFill, preFill - 1));
    }
    for (unsigned start = 0; start < nElem; ) {
        unsigned int end = std::min(nElem, start + batchSize);
        pool.appendTask([start,end, &fillMe, &preFilled](){
            for (unsigned int i = start; i < end; ++i) {
                EXPECT_EQ(fillMe.insert(std::make_pair(i, i+1)),
                          !preFilled.count(i));
            }
        });
        start = end;
    }
    pool.drainQueue();
}
TEST_F(GeoIdTest, ConcurrentDuplicates) {
    auto& pool = GeoThreading::ThreadPool::getPool();

    TestMap_t fillMe{};
    for (unsigned int batch =0; batch < nBatches; ++batch) {
        pool.appendTask([&fillMe](){
             for (unsigned int b =0 ; b < batchSize; ++b){
                fillMe.insert(std::make_pair(b,b+1));
             }
        });
    }
    pool.drainQueue();
    EXPECT_EQ(fillMe.size(), batchSize);
}

TEST_F(GeoIdTest, MoveTest) {
    TestMap_t fillMe{};
    for (unsigned k =0 ; k < batchSize; ++k) {
        fillMe.insert(std::make_pair(k,k));
    }
    EXPECT_EQ(fillMe.size(), batchSize);
    /// Move the memory to a new map
    TestMap_t moveTo{std::move(fillMe)};
    EXPECT_EQ(moveTo.size(), batchSize);
    EXPECT_TRUE(fillMe.empty());
    /// Move the memory back
    fillMe = std::move(moveTo);
    EXPECT_EQ(fillMe.size(), batchSize);
    EXPECT_TRUE(moveTo.empty());
    
}

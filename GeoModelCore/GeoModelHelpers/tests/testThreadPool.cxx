// ThreadPoolTest.cpp

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include "GeoModelHelpers/ThreadPool.h"

using namespace GeoThreading;
using namespace std::chrono_literals;

class ThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure we start with a fresh pool for each test
        ThreadPool::closePool();
    }

    void TearDown() override {
        // Clean up any pool after each test
        ThreadPool::closePool();
    }
};

TEST_F(ThreadPoolTest, SingleTaskExecution) {
    ThreadPool& pool = ThreadPool::getPool(2);
    std::atomic<bool> taskExecuted{false};

    pool.appendTask([&taskExecuted]() {
        taskExecuted = true;
    });

    pool.drainQueue(); // Ensure task runs
    EXPECT_TRUE(taskExecuted);
}

TEST_F(ThreadPoolTest, MultipleTasksExecution) {
    ThreadPool& pool = ThreadPool::getPool(4);
    std::atomic<int> counter{0};
    constexpr unsigned nTests = 2500;
    for (unsigned i = 0; i < nTests; ++i) {
        pool.appendTask([&counter]() {
            const unsigned wait = std::rand() % 15 + 1;
            std::this_thread::sleep_for(wait*1ms);
            ++counter;
        });
    }

    pool.drainQueue();
    EXPECT_EQ(counter.load(), nTests);
}

TEST_F(ThreadPoolTest, QueueSizeDecreasesAfterDraining) {
    ThreadPool& pool = ThreadPool::getPool(2);
    constexpr unsigned nTests = 10;
    for (int i = 0; i < nTests; ++i) {
        pool.appendTask([]() { 
            const unsigned wait = std::rand() % 15 + 5;
            std::this_thread::sleep_for(wait*1ms); 
        });
    }

    EXPECT_EQ(pool.queue(), nTests);
    pool.drainQueue();
    EXPECT_EQ(pool.queue(), 0);
}

TEST_F(ThreadPoolTest, ThreadCountIsCorrect) {
    ThreadPool& pool = ThreadPool::getPool(3);
    // One thread is reserved for distributeTasks
    EXPECT_EQ(pool.nThreads(), 3);  // User requested 3 threads
}

TEST_F(ThreadPoolTest, TasksAreExecutedInParallel) {
    ThreadPool& pool = ThreadPool::getPool(4);
    std::atomic<int> runningTasks{0};
    std::atomic<int> peakConcurrency{0};
    constexpr unsigned nTests = 100;
    for (int i = 0; i < nTests; ++i) {
        pool.appendTask([&]() {
            int current = ++runningTasks;
            peakConcurrency.store(std::max(peakConcurrency.load(), current));
            const unsigned wait = std::rand() % 35 + 5;
            std::this_thread::sleep_for(wait * 1ms);
            --runningTasks;
        });
    }

    pool.drainQueue();
    EXPECT_EQ(peakConcurrency.load(), 4); // At least 2 concurrent tasks
}
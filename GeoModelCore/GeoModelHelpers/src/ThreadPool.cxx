/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GeoModelHelpers/ThreadPool.h"
#include "GeoModelKernel/throwExcept.h"
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <ranges>
#include <iostream>
#include <algorithm>

namespace{
    constexpr unsigned logLevel = 0;
}
#define PRINT_MSG(MSG)                                 \
    if (logLevel >= 1) {                               \
        std::scoped_lock{s_coutMutex};                 \
        std::cout<<__func__<<"() - "<<__LINE__<<" ("   \
                 <<std::this_thread::get_id()<<") "    \
                 <<MSG<<std::endl;                     \
    }


namespace GeoThreading{
    ThreadPool* ThreadPool::s_pool = nullptr;
    std::mutex ThreadPool::s_coutMutex{};
    std::mutex ThreadPool::s_singletonMutex{};


    ThreadPool& ThreadPool::getPool(unsigned n) {
        std::scoped_lock guard{s_singletonMutex};
        if (!s_pool) s_pool = new ThreadPool(n);
        return *s_pool;
    }
    ThreadPool::~ThreadPool() {
        m_active = false;
        m_workers.clear();
    }
    void ThreadPool::closePool() {
        std::scoped_lock guard{s_singletonMutex};
        if (!s_pool) return;
        s_pool->drainQueue();
        delete s_pool;
        s_pool = nullptr;
    }
    ThreadPool::ThreadPool(unsigned nThreads) {
        nThreads = std::min(nThreads, std::thread::hardware_concurrency());
        if (!nThreads) {
            return;
        }
        for (unsigned int n = 0; n <= nThreads; ++n){
            m_workers.emplace_back(std::make_unique<ThreadWorker>());
        }
        /// The first worker is taking care of the task distribution
        m_workers.front()->newTask(std::make_unique<ThreadTask>([this](){distributeTasks();}));
    }
    void ThreadPool::appendTask(TaskFunction_t && f) {
        /// No external threads registered
        if (m_workers.empty()) {
            f();
            return;
        }
        std::unique_lock lock{m_mutex};
        m_queue.emplace_back(std::make_unique<ThreadTask>(std::move(f)));
    }
    unsigned ThreadPool::queue() const {
        std::shared_lock lock{m_mutex};
        return m_queue.size();
    }
    void ThreadPool::distributeTasks() {
        do {
            while (queue()) {
                PRINT_MSG("New distribution iteration.");
                /// Find the first idle worker
                auto idle_worker = std::ranges::find_if(m_workers,
                                                        [](const std::unique_ptr<ThreadWorker>& worker){
                                                            return worker->isIdle();
                                                        });
                /// There's no worker idle at the moment. 
                /// Just wait until the next one becomes free
                if (idle_worker == m_workers.end()) {
                    PRINT_MSG("All worker are busy. Wait.");
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    continue;
                }
                /// Take the first task in the queue & assign it to the worker
                std::unique_lock lock{m_mutex};
                std::unique_ptr<ThreadTask>& theTask = m_queue.front();
                (*idle_worker)->newTask(std::move(theTask));
                /// Erase it from the queue
                m_queue.erase(m_queue.begin());
            }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        } while (m_active);
    }
    unsigned ThreadPool::nThreads() const { return m_workers.size() - 1; }
    void ThreadPool::drainQueue(){ 
        while (unsigned int n = queue()) {
            PRINT_MSG("Wait until the last "<<n<<" tasks are launched. ");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        while (unsigned int n = std::ranges::count_if(m_workers,
                [](const std::unique_ptr<ThreadWorker>& worker){
                    return !worker->isIdle();
                }) > m_active) {
            PRINT_MSG("Wait until the last "<<n<<" tasks are finished. ");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }


    ThreadPool::ThreadTask::ThreadTask(TaskFunction_t&& f):
        m_func{std::move(f)}{}
    void ThreadPool::ThreadTask::execute(){
        m_func();
    }

    ThreadPool::ThreadWorker::ThreadWorker() = default;
    ThreadPool::ThreadWorker::~ThreadWorker() {
        stop();
    }
    bool ThreadPool::ThreadWorker::isIdle() const{
        std::shared_lock lock{m_mutex};
        return !m_task;
    }
    void ThreadPool::ThreadWorker::newTask(std::unique_ptr<ThreadTask>&& task){
        std::unique_lock lock{m_mutex};
        if (m_task) {
            THROW_EXCEPTION("Cannot get a new task if there's still one");
        }
        m_task = std::move(task);
    }
    void ThreadPool::ThreadWorker::executeTask() {
        if (!m_task) {
            return;
        }
        m_task->execute();
        std::unique_lock lock{m_mutex};
        m_task.reset();
    }
    void ThreadPool::ThreadWorker::launch(std::stop_token stop) {
        PRINT_MSG("Spawn new thread "<<m_thread.get_id());
        do {
            PRINT_MSG("New execution round for the worker");
            while (isIdle()) {
                PRINT_MSG("No thread yet registered ");
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                if (stop.stop_requested()) {
                    break;
                }
            }
            executeTask();
        } while (!stop.stop_requested());
    }
    void ThreadPool::ThreadWorker::stop(){
        m_thread.request_stop();
        while (!isIdle()) {
            PRINT_MSG("Wait until the last task is finished before shutting down");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        PRINT_MSG("Shutdown thread: "<<m_thread.get_id()<<", "<<m_thread.joinable());
        m_thread.join();
    }
}
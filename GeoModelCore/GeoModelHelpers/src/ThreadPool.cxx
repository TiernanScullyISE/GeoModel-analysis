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
    constexpr auto threadSleep = std::chrono::microseconds(10);
}
#define PRINT_MSG(MSG)                                 \
    if (logLevel >= 1) {                               \
        (void)std::scoped_lock{s_coutMutex};           \
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
        drainQueue();
        m_workers.clear();
    }
    void ThreadPool::closePool() {
        std::scoped_lock guard{s_singletonMutex};
        if (!s_pool) return;
        delete s_pool;
        s_pool = nullptr;
    }
    ThreadPool::ThreadPool(unsigned nThreads) {
        nThreads = std::min(nThreads, std::thread::hardware_concurrency());
        for (unsigned int n = 0; n < nThreads; ++n){
            m_workers.emplace_back(std::make_unique<ThreadWorker>(this));
        }
    }
    void ThreadPool::appendTask(TaskFunction_t && f) {
        appendTask(std::make_unique<ThreadTask>(std::move(f)));
    }
    void ThreadPool::appendTask(std::unique_ptr<IThreadTask>&& task) {
        /// No external threads registered. Execute the task right away
        if (m_workers.empty()) {
            task->execute();
            return;
        }
        std::unique_lock lock{m_mutex};
        m_queue.emplace_back(std::move(task));
    }
    unsigned ThreadPool::queue() const {
        std::shared_lock lock{m_mutex};
        return m_queue.size();
    }
    std::unique_ptr<ThreadPool::IThreadTask> ThreadPool::nextTask() {
        std::unique_lock lock{m_mutex};
        TaskCont_t::iterator ready_task = std::ranges::find_if(m_queue,
                [](const std::unique_ptr<IThreadTask>& task){
                    return task->ready();
                });
        if (ready_task == m_queue.end()) {
            PRINT_MSG("No free task is ready.");
            return nullptr;
        }
        std::unique_ptr<IThreadTask> returnMe = std::move(*ready_task);
        PRINT_MSG("Assign new task to thread.");
        m_queue.erase(ready_task);
        return returnMe;
    }
    unsigned ThreadPool::nThreads() const { return m_workers.size(); }

    void ThreadPool::drainQueue(){ 
        unsigned int n = queue();
        do {
            PRINT_MSG("Wait until the last "<<n<<" tasks are launched. ");
            std::this_thread::sleep_for(threadSleep);
        } while (n = queue());
        // Wait until all threads are idle
        n = m_workers.size();
        do{
            PRINT_MSG("Wait until the last "<<n<<" tasks are finished. ");
            std::this_thread::sleep_for(threadSleep);
        } while  (n = std::ranges::count_if(m_workers,
                [](const std::unique_ptr<ThreadWorker>& worker){
                    return !worker->isIdle();
                }));
    }

    /************************************************************* 
     *                  ThreadTask
     **************************************************************/
    ThreadPool::ThreadTask::ThreadTask(TaskFunction_t&& f):
        m_func{std::move(f)}{}
    void ThreadPool::ThreadTask::execute(){ m_func(); }
    bool ThreadPool::ThreadTask::ready() const { return true; }
    /************************************************************* 
     *                  ThreadWorker
     **************************************************************/
    ThreadPool::ThreadWorker::ThreadWorker(ThreadPool* parent):
        m_parent{parent}{}
    ThreadPool::ThreadWorker::~ThreadWorker() {
        stop();
    }
    void ThreadPool::ThreadWorker::launch(std::stop_token stop) {
        PRINT_MSG("Spawn new thread "<<m_thread.get_id());
        do {
            m_idle = false;
            PRINT_MSG("New execution round for the worker");
            std::unique_ptr<IThreadTask> task = m_parent->nextTask();
            if (!task) {
                m_idle = true;
                PRINT_MSG("No thread yet registered ");
                std::this_thread::sleep_for(threadSleep);
            } else {
                task->execute();
            }
        } while (!stop.stop_requested());
        m_done = true;
    }
    void ThreadPool::ThreadWorker::stop() {
        if (m_done) {
            return; // Already stopped
        }
        m_thread.request_stop();
        while (!m_done) {
            PRINT_MSG("Wait until the last task is finished before shutting down");
            std::this_thread::sleep_for(threadSleep);
        }
        PRINT_MSG("Shutdown thread: "<<m_thread.get_id()<<", "<<m_thread.joinable());
        m_thread.join();
    }
    bool ThreadPool::ThreadWorker::isIdle() const { return m_idle; }
}
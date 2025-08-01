/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GeoModelHelpers_THREADPOOL_H
#define GeoModelHelpers_THREADPOOL_H

#include <thread>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <stop_token>

namespace GeoThreading {
    /** @brief Helper class to maintain a pool of threads. The threads
               are kept open until the pool closed. Users can send their tasks
               onto the queue of the pool. The tasks are then taken
               from the queue and assigned to the next free thread for
               processing. */
    class ThreadPool {
        public:
            /** @brief Interface class of which all thread tasks may inherit */
            class IThreadTask{
                public:
                    virtual ~IThreadTask() = default;
                    /** @brief Interface function for task execution. The
                     *         method is called by the worker to launch the task */
                    virtual void execute()  = 0;
                    /** @brief Task state flag. If the task is not ready
                     *          it's not scheduled for execution albeit it's 
                     *          first in the queue */
                    virtual bool ready() const = 0;
            };
            /** @brief Returns the singelton to the ThreadPool instance
                       if called for the first time a new pool is created
                       with the number of threads specified during the call.
                @param nThreads: Number of threads in the pool. Relevant if the pool
                                 is created. */
            static ThreadPool& getPool(unsigned int nThreads = 1);

            /** @brief Closes the current ThreadPool. The queue is drained and
                       it's ensured that all tasks are finished. Then the
                       pool is deleted. */
            static void closePool();
            /** @brief Function signature to submit a task to the ThreadPool */
            using TaskFunction_t = std::function<void()>;
            /** @brief Append a new task to the queue. In the easiest way,
             *         task are parsed via lambda functions
             *      pool.appendTask([](){foo();});
             * @param f: Created task function to be appended to the queue */
            void appendTask(TaskFunction_t && f);
            /** @brief Append a gneric task to the thread pool */
            void appendTask(std::unique_ptr<IThreadTask>&& task);
            /** @brief Returns how many tasks are currently in the queue */
            unsigned queue() const;
            /** @brief Returns the number of threads in the pool */
            unsigned nThreads() const;
            /** @brief Drains the queue. The method waits until all current tasks
             *         in the queue are moved to a thread worker and then waits until
             *         all thread workers have finished executing their tasks */
            void drainQueue();
        private:
            /** @brief Constructor of the thread pool
             *  @param nThreads: How many threads should be executed */
            ThreadPool(unsigned nThreads);
            /** @brief delete copy constructor */
            ThreadPool(const ThreadPool&) = delete;
            /** @brief delete move constructor */
            ThreadPool(ThreadPool&&) = delete;
            /** @brief delete copy assignment */
            ThreadPool& operator=(const ThreadPool&) = delete;
            /** @brief delete move assignment */
            ThreadPool& operator=(ThreadPool&&) = delete;
            /** @brief Destructor finalizing the tasks which are still active and
             *         then emptying the queue */
            ~ThreadPool();
            /** @brief Main function called by the workers. The thread pool returns the 
             *         next available task for processing, if there's any. The returned task 
             *         is removed from the queue */
            std::unique_ptr<IThreadTask> nextTask();

            class ThreadTask : public IThreadTask {
                public:
                    ThreadTask(TaskFunction_t&& f);
                    void execute() override final;
                    virtual bool ready() const override final;
                private:
                    TaskFunction_t m_func{[](){}}; 
            };

            class ThreadWorker{
                public:
                    /** @brief Constructor spawning a new thread */
                    ThreadWorker(ThreadPool* parent);
                    /** @brief Destructor */
                    ~ThreadWorker();
                    /** @brief Returns whether the work is free for a new task */
                    bool isIdle() const;
                    /** @brief Shuts down the worker. The current task is finalized
                     *         & the thread resources are freed */
                    void stop();
                private:
                    /** @brief Routine called by the thread. It checks whether new
                     *         tasks are available & executes them. Otherwise it sends
                               the thread to sleep 
                        @param stop: Control token to indicate that the thread needs to terminate. */
                    void launch(std::stop_token stop);
                    /** @brief Pointer to the thread pool instance spawning the Worker. */
                    ThreadPool* m_parent{};
                    /** @brief Thread object instantiated by the class */
                    std::jthread m_thread{[this](std::stop_token stop){launch(stop);}};
                    /** @brief flag indicating that the worker is sleeping */
                    std::atomic<bool> m_idle{true};
                    /** @brief Flag indicating that the worker is done */
                    std::atomic<bool> m_done{false};

            };
            /** @brief Singelton pointer */
            static ThreadPool* s_pool;
            static std::mutex s_singletonMutex;
            static std::mutex s_coutMutex;

            mutable std::shared_mutex m_mutex{};
            std::vector<std::unique_ptr<ThreadWorker>> m_workers{};
            using TaskCont_t = std::vector<std::unique_ptr<IThreadTask>>;
            TaskCont_t m_queue{};
    };
}

#endif

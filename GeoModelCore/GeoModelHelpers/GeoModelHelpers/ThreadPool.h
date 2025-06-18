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
namespace GeoThreading {
    /** @brief Helper class to maintain a pool of threads. The threads
               are kept open until the pool closed. Users can send their tasks
               onto the queue of the pool. The tasks are then taken
               from the queue and assigned to the next free thread for
               processing. */
    class ThreadPool{
        public:
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
            /** @brief Main function of the thread pool. The available tasks in the queue
             *         are distributed among the available thread workers. If no task is 
             *         in the queue the workers are kept in idle */
            void distributeTasks();

            class ThreadTask{
                public:
                    ThreadTask(TaskFunction_t&& f);
                    void execute();
                private:
                    TaskFunction_t m_func{[](){}}; 
            };

            class ThreadWorker{
                public:
                    /** @brief Constructor spawning a new thread */
                    ThreadWorker();
                    /** @brief Destructor */
                    ~ThreadWorker();
                    /** @brief Returns whether the work is free for a new task */
                    bool isIdle() const;
                    /** @brief Assign a new task to the worker */
                    void newTask(std::unique_ptr<ThreadTask>&& task);
                    /** @brief Shuts down the worker. The current task is finalized
                     *         & the thread resources are freed */
                    void stop();
                private:
                    /** @brief Routine called by the thread. It checks whether new
                     *         tasks are available & executes them. Otherwise it sends
                               the thread to sleep 
                        @param stop: Control token to indicate that the thread needs to terminate. */
                    void launch(std::stop_token stop);
                    /** @brief  Execute the task and release it after  */
                    void executeTask();
                    /** @brief Mutex object to assign tasks thread safely  */
                    mutable std::shared_mutex m_mutex{};
                    /** @brief Current task to process */
                    std::unique_ptr<ThreadTask> m_task{};
                    /** @brief Thread object instantiated by the class */
                    std::jthread m_thread{[this](std::stop_token stop){launch(stop);}};

            };
            /** @brief Singelton pointer */
            static ThreadPool* s_pool;
            static std::mutex s_singletonMutex;
            static std::mutex s_coutMutex;

            mutable std::shared_mutex m_mutex{};
            std::vector<std::unique_ptr<ThreadWorker>> m_workers{};
            std::vector<std::unique_ptr<ThreadTask>> m_queue{};
            std::atomic<bool> m_active{true};


    };
}

#endif

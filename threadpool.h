// Copyright (c) 2023-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php.

#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H

#include "bitcoin/sync.h"
#include "bitcoin/threadnames.h"

#include <condition_variable>
#include <future>
#include <queue>
#include <thread>

class ThreadPool {

    struct Task {
        virtual ~Task() {};
        virtual void operator()() = 0;
    };

    template <typename T>
    struct TaskImpl : Task {
        std::packaged_task<T()> task;

        TaskImpl(std::function<T()>&& func) : task(func) {}
        ~TaskImpl() override {}

        void operator()() override { task(); }
        std::future<T> get_future() { return task.get_future(); }
    };

private:
    Mutex cs_work_queue;
    std::queue<std::unique_ptr<Task>> m_work_queue GUARDED_BY(cs_work_queue);
    std::condition_variable m_condition;
    // Stop indicator
    std::atomic<bool> m_stop{false};
    // Worker threads
    std::vector<std::thread> m_workers;

    void WorkerThread(const std::string& thread_name) EXCLUSIVE_LOCKS_REQUIRED(!cs_work_queue)
    {
        util::ThreadRename(std::string{thread_name});
        while (!m_stop.load()) {
            ProcessTask();
        }
    }

public:
    ThreadPool() {}

    ~ThreadPool()
    {
        Stop(); // In case it hasn't being stopped.
    }

    void Start(int num_workers)
    {
        if (!m_workers.empty()) throw std::runtime_error("Thread pool already started");

        // Create the workers
        for (int i=0; i<num_workers; i++) {
            m_workers.push_back(std::thread(&ThreadPool::WorkerThread, this, "threadpool_worker_"+std::to_string(i)));
        }
    }

    void Stop()
    {
        // Notify workers and join them.
        m_stop = true;
        m_condition.notify_all();
        for (auto& worker : m_workers) {
            worker.join();
        }
        m_workers.clear();
        m_stop = false;
    }

    template <typename Callable, typename T = typename std::invoke_result<Callable>::type>
    std::future<T> Submit(const Callable&& func) EXCLUSIVE_LOCKS_REQUIRED(!cs_work_queue)
    {
        std::unique_ptr<Task> task = std::make_unique<TaskImpl<T>>(func);
        std::future<T> future = dynamic_cast<TaskImpl<T>*>(task.get())->get_future();

        LOCK(cs_work_queue);
        m_work_queue.push(std::move(task));
        m_condition.notify_one();
        return future;
    }

    size_t WorkQueueSize() EXCLUSIVE_LOCKS_REQUIRED(!cs_work_queue)
    {
        return WITH_LOCK(cs_work_queue, return m_work_queue.size());
    }

    void ProcessTask() EXCLUSIVE_LOCKS_REQUIRED(!cs_work_queue)
    {
        std::unique_ptr<Task> task;
        {
            // Wait for the task or until the stop flag is set
            WAIT_LOCK(cs_work_queue, wait_lock);
            m_condition.wait(wait_lock,[&]() EXCLUSIVE_LOCKS_REQUIRED(cs_work_queue) { return m_stop.load() || !m_work_queue.empty(); });

            // If stopped, exit worker.
            if (m_stop.load() && m_work_queue.empty()) {
                return;
            }

            // Pop the task
            task = std::move(m_work_queue.front());
            m_work_queue.pop();
        }

        // Execute the task
        (*task)();
    }
};

#endif // THREADPOOL_THREADPOOL_H

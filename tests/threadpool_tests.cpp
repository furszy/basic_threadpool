#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include "../threadpool.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(threadpool_tests)

BOOST_AUTO_TEST_CASE(threadpool_basic)
{
    // Test case 1
    // Start thread pool, submit operations and verify all submitted
    // tasks are executed.
    {
        int num_workers = 3;
        int num_tasks = 50;

        ThreadPool threadPool;
        threadPool.Start(num_workers);
        std::atomic<int> counter = 0;
        for (int i=0; i<num_tasks; i++) {
            threadPool.Submit([&counter]() {
                counter++;
            });
        }

        while (threadPool.WorkQueueSize() != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds{2});
        }
        BOOST_CHECK_EQUAL(counter, num_tasks);
    }

    // Test case 2
    // Maintain all threads busy except one.
    {
        int num_workers = 3;

        ThreadPool threadPool;
        threadPool.Start(num_workers);
        std::atomic<bool> stop = false;
        for (int i=0; i<num_workers-1; i++) {
            threadPool.Submit([&stop]() {
                while (!stop) {
                    std::this_thread::sleep_for(std::chrono::milliseconds{10});
                }
            });
        }

        // Now execute tasks on the single available worker
        // and check that all the tasks are executed.
        int num_tasks = 15;
        std::atomic<int> counter = 0;
        for (int i=0; i<num_tasks; i++) {
            threadPool.Submit([&counter]() {
                counter++;
            });
        }
        while (threadPool.WorkQueueSize() != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds{2});
        }
        BOOST_CHECK_EQUAL(counter, num_tasks);

        stop = true;
    }

    // Test case 3
    // Wait for work to finish.
    {
        int num_workers = 3;
        ThreadPool threadPool;
        threadPool.Start(num_workers);
        std::atomic<bool> flag = false;
        std::future<void> future = threadPool.Submit([&flag]() {
            std::this_thread::sleep_for(std::chrono::milliseconds{200});
            flag = true;
        });
        future.get();
        BOOST_CHECK(flag.load());
    }

    // Test case 4
    // Get result object from future.
    {
        int num_workers = 3;
        ThreadPool threadPool;
        threadPool.Start(num_workers);
        std::future<bool> future_bool = threadPool.Submit([]() {
            return true;
        });
        BOOST_CHECK(future_bool.get());

        std::future<std::string> future_str = threadPool.Submit([]() {
            return std::string("true");
        });
        BOOST_CHECK_EQUAL(future_str.get(), "true");
    }

    // Test case 5
    // Throw exception and catch it on the consumer side.
    {
        int num_workers = 3;
        ThreadPool threadPool;
        threadPool.Start(num_workers);
        std::future<void> future = threadPool.Submit([]() {
            throw std::runtime_error("something wrong happened");
        });
        try {
            future.get();
            BOOST_ASSERT_MSG(false, "error: did not throw an exception");
        } catch (const std::runtime_error& e) {
            BOOST_CHECK_EQUAL(e.what(), "something wrong happened");
        }
    }

    // Test case 6
    // All workers are busy, help them by processing tasks from outside
    {
        int num_workers = 3;

        ThreadPool threadPool;
        threadPool.Start(num_workers);
        std::atomic<bool> stop = false;
        for (int i=0; i<num_workers; i++) {
            threadPool.Submit([&stop]() {
                while (!stop) {
                    std::this_thread::sleep_for(std::chrono::milliseconds{10});
                }
            });
        }

        // Now submit tasks and check that none of them are executed.
        int num_tasks = 20;
        std::atomic<int> counter = 0;
        for (int i=0; i<num_tasks; i++) {
            threadPool.Submit([&counter]() {
                counter++;
            });
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
        BOOST_CHECK_EQUAL(threadPool.WorkQueueSize(), 20);

        // Now process them
        for (int i=0; i<num_tasks; i++) {
            threadPool.ProcessTask();
        }
        BOOST_CHECK_EQUAL(counter, num_tasks);
        stop = true;
    }

}

BOOST_AUTO_TEST_SUITE_END()

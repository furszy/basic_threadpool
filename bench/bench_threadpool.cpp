// Copyright (c) 2016-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bench.h"

#include "../threadpool.h"

static void BenchThreadPoolProcess(benchmark::Bench& bench)
{
    ThreadPool threadPool;
    threadPool.Start(/*num_workers=*/3);

    int NUM_OF_TASKS = 333;
    bench.minEpochIterations(100).run([&] {
        std::atomic<int> counter = 0;
        std::vector<std::future<void>> futures;
        for (int i=0; i<NUM_OF_TASKS; i++) {
            futures.emplace_back(threadPool.Submit([&counter]() {
                counter++;
            }));
        }
        for (auto& future : futures) future.get();
        assert(NUM_OF_TASKS == counter.load());
    });
}

BENCHMARK(BenchThreadPoolProcess);

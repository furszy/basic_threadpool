// Copyright (c) 2015-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bench.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

static constexpr int64_t DEFAULT_MIN_TIME_MS{10};

int main(int argc, char** argv) // TODO: esto no anda..
{
    try {
        benchmark::Args args;
        args.is_list_only = false;
        args.min_time = std::chrono::milliseconds(DEFAULT_MIN_TIME_MS);
        args.sanity_check = false;
        args.sanity_check = false;

        benchmark::BenchRunner::RunAll(args);

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        tfm::format(std::cerr, "Error: %s\n", e.what());
        return EXIT_FAILURE;
    }
}

cmake_minimum_required(VERSION 3.24)
project(threadpool)

set(CMAKE_CXX_STANDARD 17)

add_executable(threadpool main.cpp bitcoin/threadnames.h bitcoin/tinyformat.h bitcoin/threadsafety.h bitcoin/macros.h bitcoin/sync.cpp threadpool.h bitcoin/syserror.cpp bitcoin/syserror.h bitcoin/fs.h bitcoin/fs.cpp)

add_subdirectory(tests)
add_subdirectory(bench)
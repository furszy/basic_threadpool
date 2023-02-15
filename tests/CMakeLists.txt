set (Boost_USE_STATIC_LIBS OFF)
find_package (Boost REQUIRED COMPONENTS unit_test_framework)
include_directories (${Boost_INCLUDE_DIRS})

add_executable (test_run threadpool_tests.cpp)
target_link_libraries (test_run ${Boost_LIBRARIES})
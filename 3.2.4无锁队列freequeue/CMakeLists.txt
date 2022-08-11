cmake_minimum_required(VERSION 3.1)

project(C++11)
# 设置C++标准为 C++ 11
# set(CMAKE_CXX_STANDARD 11)
# 设置C++标准为 C++ 14
set(CMAKE_CXX_STANDARD 14)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g")
add_executable(1_test_i++ 1_test_i++.c)
add_executable(2_test_i++_threads 2_test_i++_threads.c)
add_executable(3_test_mutex_spin_lock 3_test_mutex_spin_lock.cpp)
add_executable(4_test_lock_free_queue 4_test_lock_free_queue.cpp)
target_link_libraries(2_test_i++_threads pthread)
target_link_libraries(3_test_mutex_spin_lock pthread)
target_link_libraries(4_test_lock_free_queue pthread)
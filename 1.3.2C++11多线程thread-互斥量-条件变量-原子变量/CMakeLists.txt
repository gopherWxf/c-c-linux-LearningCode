cmake_minimum_required(VERSION 3.1)

project (C++11)
# 设置C++标准为 C++ 11
# set(CMAKE_CXX_STANDARD 11)
# 设置C++标准为 C++ 14
set(CMAKE_CXX_STANDARD 14)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g")

add_executable(1-thread  1-thread.cpp)
target_link_libraries(1-thread pthread)

add_executable(1-thread1-simple  1-thread1-simple.cpp)
target_link_libraries(1-thread1-simple pthread)


add_executable(1-thread2-pack 1-thread2-pack.cpp zero_thread.cpp)
target_link_libraries(1-thread2-pack pthread)


add_executable(1-2-4-lock  1-2-4-lock.cpp)
target_link_libraries(1-2-4-lock pthread)

add_executable(1-2-4-unique_lock  1-2-4-unique_lock.cpp)
target_link_libraries(1-2-4-unique_lock pthread)

add_executable(1-2-mutex1  1-2-mutex1.cpp)
target_link_libraries(1-2-mutex1 pthread)

add_executable(1-2-mutex2-dead-lock  1-2-mutex2-dead-lock.cpp)
add_executable(1-2-recursive_mutex1  1-2-recursive_mutex1.cpp)

add_executable(1-2-timed_mutex  1-2-timed_mutex.cpp)
target_link_libraries(1-2-timed_mutex pthread)

add_executable(1-2-shared_ptr-mutex-notify  1-2-shared_ptr-mutex-notify.cpp)
target_link_libraries(1-2-shared_ptr-mutex-notify pthread)


add_executable(1-3-condition-sync-queue  1-3-condition-sync-queue.cpp)
target_link_libraries(1-3-condition-sync-queue pthread)
add_executable(1-3-condition-sync-queue2  1-3-condition-sync-queue2.cpp)
target_link_libraries(1-3-condition-sync-queue2 pthread)

add_executable(1-4-atomic  1-4-atomic.cpp)
target_link_libraries(1-4-atomic pthread)

add_executable(1-5-call_once  1-5-call_once.cpp)
target_link_libraries(1-5-call_once pthread)

add_executable(1-5-future  1-5-future.cpp)
target_link_libraries(1-5-future pthread)

add_executable(1-5-package_task  1-5-package_task.cpp)

add_executable(1-5-promise  1-5-promise.cpp)
target_link_libraries(1-5-promise pthread)

add_executable(2-1-function 2-1-function.cpp)
add_executable(2-2-bind  2-2-bind.cpp)


add_executable(3-1-variable-parameter  3-1-variable-parameter.cpp)
add_executable(3-1-variable-parameter2  3-1-variable-parameter2.cpp)
add_executable(3-1-variable-parameter3  3-1-variable-parameter3.cpp)
add_executable(3-1-variable-parameter4  3-1-variable-parameter4.cpp)

add_executable(4-threadpool  4-threadpool.cpp zero_threadpool.cpp)
target_link_libraries(4-threadpool pthread)

add_executable(5-1-exception 5-1-exception.cpp)
add_executable(5-3-exception  5-3-exception.cpp)
add_executable(5-5-exception 5-5-exception_ptr.cpp)
add_executable(5-5-make_exception_ptr 5-5-make_exception_ptr.cpp)
add_executable(5-5-nested_exception 5-5-nested_exception.cpp)



#add_executable(111  222.cpp)
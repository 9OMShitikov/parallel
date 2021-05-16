cmake_minimum_required(VERSION 3.16.3)
project(_1)

set(CMAKE_CXX_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(_1 main.cpp)
target_link_libraries(_1 PRIVATE Threads::Threads)

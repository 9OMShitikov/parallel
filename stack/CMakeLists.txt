cmake_minimum_required(VERSION 3.19)
project(_3)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_executable(_3 main.cpp timer.cpp)
target_link_libraries(_3 PRIVATE Threads::Threads)
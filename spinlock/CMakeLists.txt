cmake_minimum_required(VERSION 3.19)
project(_2)

set(CMAKE_CXX_STANDARD 14)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(_2 main.cpp spinlock_TAS.cpp spinlock_TTAS.cpp ticket_lock.cpp)
target_link_libraries(_2 PRIVATE Threads::Threads)
#include <thread>
#include <atomic>
#include <cassert>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include "lock.h"
#include "spinlock_TAS.h"
#include "spinlock_TTAS.h"
#include "ticket_lock.h"
#include <new>

template <typename F, typename... Args>
double measureTime(F f, Args... args)
{
    auto start = std::chrono::steady_clock::now();
    f(args...);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    return elapsed_seconds.count();
}

template <typename Mutex>
void thread_job(Mutex& m, int& result, int increments) {
    for (int i = 0; i < increments; ++i) {
        m.lock();
        result += 1;
        m.unlock();
    }
}

template <typename Mutex>
double testMutex(int n_threads) {
    auto sum = new int(0);
    const int iters_per_run = 10000;
    const int increases = 3;

    Mutex mutex;
    std::vector<std::thread> threads;
    threads.reserve(n_threads);
    volatile std::atomic_size_t numThreadsReady = {0};

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < n_threads; ++i) {
        threads.emplace_back([&, i] {
            numThreadsReady++;
            while (numThreadsReady.load() != n_threads) {
                asm volatile("pause");
            }

            for (int j = 0; j < iters_per_run; ++j) {
                mutex.lock();
                for (int k = 0; k < increases; ++k) {
                    *sum += 1;
                }
                mutex.unlock();
            }
        });
    }
    for (int i = 0; i < n_threads; ++i) {
        threads[i].join();
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;

    assert(*sum == iters_per_run * increases * n_threads);
    return elapsed_seconds.count();
}


template<typename Mutex>
void measureToCSV(std::string name, int n_threads_max) {
    int n_runs = 3;

    std::ofstream results;
    results.open (name + ".csv");
    results << R"("Number of threads","time")"<<std::endl;

    for (int i = 1; i <= n_threads_max; ++i) {
        double time = 0;
        for (int j = 0; j < n_runs; ++j) {
            time += testMutex<Mutex>(i);
        }
        time /= n_runs;
        results << i <<","<<time<<std::endl;
    }
    results.close();
}

int main()
{
    int n_threads_max = 8;
    measureToCSV<spinlock_TAS>("TAS_lock", n_threads_max);
    //measureToCSV<spinlock_TTAS>("TTAS_lock", n_threads_max);
    //measureToCSV<ticket_lock>("ticket_lock", n_threads_max);
}
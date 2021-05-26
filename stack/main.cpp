#include <thread>
#include <atomic>
#include <cassert>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include "stack.h"
#include "timer.h"
#include <new>

template <typename F>
void measureToCSV(const std::string& name, int n_threads_max, F f) {
    int n_runs = 5;

    std::ofstream results;
    results.open (name + ".csv");
    results << R"("Number of threads","time")"<<std::endl;

    for (int i = 1; i <= n_threads_max; ++i) {
        double time = 0;
        timer clock;
        for (int j = 0; j < n_runs; ++j) {
            f(i, clock);
            time += clock.elapsed();
            clock.reset();
        }
        time /= n_runs;
        results << i <<","<<time<<std::endl;
    }
    results.close();
}

double testStackPushPopDivided(int n_threads, timer& clock) {
    const int iters_per_run = 100000;

    stack <int> Stack(n_threads);
    std::vector<std::thread> threads;
    threads.reserve(n_threads);
    volatile std::atomic_size_t numThreadsReady = {0};

    clock.start();
    for (int i = 0; i < n_threads; ++i) {
        threads.emplace_back([&Stack, &numThreadsReady, i, n_threads] {
            numThreadsReady++;
            while (numThreadsReady.load() != n_threads) {
                asm volatile("pause");
            }

            for (int j = 0; j < iters_per_run/n_threads; ++j) {
                Stack.push(j);
            }
            int buf;
            for (int j = 0; j < iters_per_run/n_threads; ++j) {
                assert(Stack.pop(buf));
            }
        });
    }
    for (int i = 0; i < n_threads; ++i) {
        threads[i].join();
    }
}

double testStackPushPopMixed(int n_threads, timer& clock) {
    const int iters_per_run = 100000;
    const int thread_init_stack_size = 1000;

    stack <int> Stack(n_threads);

    for (int i = 0; i < n_threads; ++i) {
        for (int j = 0; j < thread_init_stack_size; ++j) {
            Stack.push(j);
        }
    }

    std::vector<std::thread> threads;
    threads.reserve(n_threads);
    volatile std::atomic_size_t numThreadsReady = {0};

    clock.start();
    for (int i = 0; i < n_threads; ++i) {
        threads.emplace_back([&Stack, &numThreadsReady, i, n_threads] {
            numThreadsReady++;
            while (numThreadsReady.load() != n_threads) {
                asm volatile("pause");
            }

            int stack_added = thread_init_stack_size;
            for (int j = 0; j < 2 * iters_per_run/n_threads; ++j) {
                int should_pop = rand()%2;
                if (should_pop && stack_added <= 0) {
                    should_pop = 0;
                }
                if (should_pop) {
                    int buf;
                    stack_added--;
                    assert(Stack.pop(buf));
                } else {
                    stack_added++;
                    Stack.push(j);
                }
            }
        });
    }
    for (int i = 0; i < n_threads; ++i) {
        threads[i].join();
    }
}


int main()
{
    int n_threads_max = 12;
    measureToCSV("PushPopDivided", n_threads_max, testStackPushPopDivided);
    measureToCSV("PushPopMixed", n_threads_max, testStackPushPopMixed);
    return 0;
}
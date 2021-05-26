#ifndef _3_TIMER_H
#define _3_TIMER_H

#include <chrono>
#include <cassert>

class timer {
private:
    bool started = false;
    double elapsed_seconds = 0;
    std::chrono::time_point<std::chrono::steady_clock> start_point = std::chrono::steady_clock::now();
public:
    void start();
    double stop();
    [[nodiscard]] double elapsed() const;
    void reset();
};

#endif //_3_TIMER_H

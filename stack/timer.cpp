#include "timer.h"

void timer::start() {
    assert(!started);
    start_point = std::chrono::steady_clock::now();
    started = true;
}

double timer::stop() {
    assert(started);
    std::chrono::duration<double> d = std::chrono::steady_clock::now() - start_point;
    elapsed_seconds += d.count();
    started = false;
    return  elapsed_seconds;
}

[[nodiscard]] double timer::elapsed() const {
    if (started) {
        std::chrono::duration<double> d = std::chrono::steady_clock::now() - start_point;
        return d.count();
    }
    else {
        return elapsed_seconds;
    }
}

void timer::reset() {
    started = false;
    elapsed_seconds = 0;
}
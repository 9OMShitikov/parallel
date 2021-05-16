#include "spinlock_TAS.h"
#include <cassert>
spinlock_TAS::spinlock_TAS(): m_spin(0) {

}

spinlock_TAS::~spinlock_TAS() {
    assert(m_spin.load() == 0);
}

void spinlock_TAS::lock() {
    uint32_t expected;
    uint32_t counter = 0;
    do {
        expected = 0;
        counter++;
        if (counter == 3) {
            __asm__ __volatile__("pause":: :"memory");
        }
    } while (!m_spin.compare_exchange_weak(expected, 1, std::memory_order_acquire));
}

void spinlock_TAS::unlock() {
    m_spin.store(0, std::memory_order_release);
}

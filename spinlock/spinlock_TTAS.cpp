#include "spinlock_TTAS.h"
#include <cassert>
spinlock_TTAS::spinlock_TTAS(): m_spin(0) {

}

spinlock_TTAS::~spinlock_TTAS() {
    assert(m_spin.load() == 0);
}

void spinlock_TTAS::lock() {
    uint32_t expected;
    do {
        int counter = 0;
        while (m_spin.load(std::memory_order_relaxed)) {
            counter++;
            if (counter == 7) {
                counter = 0;
                __asm__ __volatile__("pause":: :"memory");
            }
        }
        expected = 0;
    } while (!m_spin.compare_exchange_weak(expected, 1, std::memory_order_acquire));
}

void spinlock_TTAS::unlock() {
    m_spin.store(0, std::memory_order_release);
}

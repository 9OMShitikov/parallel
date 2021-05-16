#ifndef _2_SPINLOCK_TTAS_H
#define _2_SPINLOCK_TTAS_H

#include <atomic>
#include "lock.h"

class spinlock_TTAS: public custom_mutex {
private:
    std::atomic<uint32_t> m_spin;
public:
    spinlock_TTAS();
    ~spinlock_TTAS();
    void lock();
    void unlock();
};


#endif //_2_SPINLOCK_TTAS_H

#ifndef _2_SPINLOCK_TAS_H
#define _2_SPINLOCK_TAS_H

#include <atomic>
#include "lock.h"

class spinlock_TAS: public custom_mutex {
private:
    std::atomic<uint32_t> m_spin;
public:
    spinlock_TAS();
    ~spinlock_TAS();
    void lock();
    void unlock();
};


#endif //_2_SPINLOCK_TAS_H

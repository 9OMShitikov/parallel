#ifndef _2_TICKET_LOCK_H
#define _2_TICKET_LOCK_H

#include "lock.h"
#include <atomic>

class ticket_lock: public custom_mutex {
private:
    alignas(64) std::atomic_size_t now_serving = {0};
    alignas(64) std::atomic_size_t next_ticket = {0};
public:
    void lock();
    void unlock();
};


#endif //_2_TICKET_LOCK_H

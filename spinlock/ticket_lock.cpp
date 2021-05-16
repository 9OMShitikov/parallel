#include "ticket_lock.h"
#include <iostream>

void ticket_lock::lock() {
    const uint32_t ticket = next_ticket.fetch_add(1 , std::memory_order_relaxed);
    while (now_serving.load(std::memory_order_acquire) != ticket)
    {
       __asm__ __volatile__("pause":::"memory");
    }
}

void ticket_lock::unlock(){
    const auto successor = now_serving.load(std::memory_order_relaxed) + 1;
    now_serving.store(successor, std::memory_order_release);
}
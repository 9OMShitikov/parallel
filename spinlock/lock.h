#ifndef _2_LOCK_H
#define _2_LOCK_H

class custom_mutex {
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
};
#endif //_2_LOCK_H

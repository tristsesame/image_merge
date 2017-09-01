#ifndef _LOCK_H_
#define _LOCK_H_

#include "stdafx.h"

namespace base{

class Lock
{
public:
    Lock();

    ~Lock();

    bool Try();

    void Acquire();

    void Release();

private:
    CRITICAL_SECTION cs_;
};

//
// Lock辅助类
// 构造函数-加锁
// 析构函数-释放锁
//
class AutoLock {
public:
    explicit AutoLock(Lock& lock) : lock_(lock) {
        lock_.Acquire();
    }

    ~AutoLock() {
        lock_.Release();
    }

private:
    Lock& lock_;
};

//
// Lock辅助类
// 构造函数-释放锁
// 析构函数-加锁
//
class AutoUnlock {
public:
    explicit AutoUnlock(Lock& lock) : lock_(lock) {
        lock_.Release();
    }

    ~AutoUnlock() {
        lock_.Acquire();
    }

private:
    Lock& lock_;
};

} // namespace Base

#endif // _LOCK_H_

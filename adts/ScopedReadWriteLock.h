
#ifndef ARCHIHW1_SCOPEREADWRITELOCK_H
#define ARCHIHW1_SCOPEREADWRITELOCK_H

#include <boost/thread/shared_mutex.hpp>

class ScopedReadWriteLock {
public:
    typedef boost::shared_mutex mutex_type;

    class ReadLock{
        ScopedReadWriteLock::mutex_type& mx;
    public:
        ReadLock(ScopedReadWriteLock::mutex_type& mx_) :
                mx(mx_){
            mx.lock_shared();
        }
        ~ReadLock(){
            mx.unlock_shared();
        }
    };

    class WriteLock{
        ScopedReadWriteLock::mutex_type& mx;
    public:
        WriteLock(ScopedReadWriteLock::mutex_type& mx_) :
                mx(mx_){
            mx.lock();
        }
        ~WriteLock(){
            mx.unlock();
        }
    };
};

#endif //ARCHIHW1_SCOPEREADWRITELOCK_H

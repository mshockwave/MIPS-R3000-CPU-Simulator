
#ifndef ARCHIHW1_SIZEDBLOCKINGQUEUE_H
#define ARCHIHW1_SIZEDBLOCKINGQUEUE_H

#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <cstdlib>

template <typename T, size_t N = 2>
class SizedBlockingQueue {

public:
    SizedBlockingQueue<T>() :
            queue(),
            mutex(),
            cond(){}

    void Push(T& value){
        boost::mutex::scoped_lock lk(mutex);
        if(queue.size() >= N){
            cond.wait(lk);
        }
        queue.push(value);
        cond.notify_all();
    }

    T& Peek(){
        boost::mutex::scoped_lock lk(mutex);
        return queue.front();
    }

    bool IsEmpty(){
        boost::mutex::scoped_lock lk(mutex);
        return queue.empty();
    }

    T& Pop(){
        boost::mutex::scoped_lock lk(mutex);
        if(queue.empty()){
            cond.wait(lk);
        }

        auto& v = queue.front();
        queue.pop();
        cond.notify_all();

        return v;
    }

private:
    std::queue<T> queue;
    boost::mutex mutex;
    boost::condition cond;
};


#endif //ARCHIHW1_SIZEDBLOCKINGQUEUE_H

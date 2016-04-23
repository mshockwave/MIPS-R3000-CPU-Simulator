
#ifndef ARCHIHW1_BLOCKINGQUEUE_H
#define ARCHIHW1_BLOCKINGQUEUE_H

#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

template <typename T>
class BlockingQueue {

public:

    BlockingQueue<T>() :
            queue(),
            mx(),
            cv(){}

    void Push(T& v) {
        boost::mutex::scoped_lock lk(mx);
        queue.push(v);
        cv.notify_all();
    }

    T& Pop(){
        boost::mutex::scoped_lock lk(mx);
        while(queue.empty()){
            //Block
            cv.wait(lk);
        }

        T& v = queue.front();
        queue.pop();
        return v;
    }

    T& Peek(){
        boost::mutex::scoped_lock lk(mx);
        return queue.front();
    }

    bool IsEmpty(){
        boost::mutex::scoped_lock lk(mx);
        return queue.empty();
    }

private:
    std::queue<T> queue;
    boost::mutex mx;
    boost::condition cv;
};

#endif //ARCHIHW1_BLOCKINGQUEUE_H


#ifndef ARCHIHW1_BLOCKINGQUEUE_H
#define ARCHIHW1_BLOCKINGQUEUE_H

#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>

template <typename T, size_t N = 50000>
class BlockingQueue {

public:
    
#ifndef NDEBUG
    friend class TestAdts;
#endif

    BlockingQueue<T>() :
            queue(N) {}

    void Push(T v) {
        queue.push(v);
    }

    T Pop(){
        
        T v = T();
        
        while(!queue.pop(v)){
            //Block
            boost::this_thread::sleep_for(boost::chrono::nanoseconds(1));
        }
        
        return v;
    }

    bool IsEmpty(){
        return queue.empty();
    }

private:
    boost::lockfree::spsc_queue<T> queue;
};

#endif //ARCHIHW1_BLOCKINGQUEUE_H

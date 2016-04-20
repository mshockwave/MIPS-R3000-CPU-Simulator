
#ifndef ARCHIHW1_EXECUTIONENGINE_H
#define ARCHIHW1_EXECUTIONENGINE_H

#include "Types.h"
#include "Context.h"
#include "Task.h"
#include "Instruction.h"
#include "Utils.h"

#include <thread>
#include <mutex>
#include <condition_variable>

class ExecutionEngine {
private:
    Context* mContext;

protected:
    class ClockHandle;
    ClockHandle& clock_handle;

public:

    static const uint8_t TotalThreadCount;

    class ClockHandle {
    private:
        uint8_t thread_counter;

    public:
        std::mutex& MutexTick;
        std::mutex& MutexCounter;
        std::condition_variable& CondVarTick;
        std::condition_variable& CondVarCounter;
        typedef std::lock_guard<std::mutex> local_lock_t;
        typedef std::unique_lock<std::mutex> lock_t;

        ClockHandle(std::mutex& mx_tick, std::mutex& mx_counter,
                    std::condition_variable& cv_tick,
                    std::condition_variable& cv_counter) :
                MutexTick(mx_tick),
                MutexCounter(mx_counter),
                CondVarTick(cv_tick),
                CondVarCounter(cv_counter),
                thread_counter(0) {}

        //template<uint8_t wait_count = TotalThreadCount>
        void Wait(){

        }

        void ResetThreadCounter(){
            local_lock_t lk(MutexCounter);
            thread_counter = 0;
        }
    };

    Context* GetContext(){
        //Wrapper for mutex in future
        return mContext;
    }

    ExecutionEngine(Context& ctx, ClockHandle& ch) :
            mContext(&ctx),
            clock_handle(ch) {}

    virtual void Start(){
        //Unreachable
    };
};

#endif //ARCHIHW1_EXECUTIONENGINE_H

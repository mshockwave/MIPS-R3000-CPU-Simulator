
#ifndef ARCHIHW1_EXECUTIONENGINE_H
#define ARCHIHW1_EXECUTIONENGINE_H

#include "Types.h"
#include "Context.h"
#include "Utils.h"
#include "Task.h"
#include "tasks/TaskHandle.h"

#include <functional>

#define FALLING_EDGE_FENCE() \
    clock.falling_edge.wait()

/*
 * Normal execution engine
 * For stages other than IF
 * */
class ExecutionEngine {

public:
    typedef std::function<void(ExecutionEngine*)> engine_runnable_t;

protected:
    Context* mContext;
    TaskHandle::ClockHandle& clock;
    engine_runnable_t runnable;

public:

    Context* GetContext(){
        //Wrapper for mutex in future
        return mContext;
    }
    const TaskHandle::ClockHandle& GetClock(){
        return const_cast<const TaskHandle::ClockHandle&>(clock);
    }

    /*
     * For special engine like IFEngine
     * */
    ExecutionEngine(Context& ctx, TaskHandle::ClockHandle& clk) :
            mContext(&ctx),
            clock(clk){ }

    ExecutionEngine(Context& ctx, TaskHandle::ClockHandle& clk,
                    const engine_runnable_t& runnable_) :
            ExecutionEngine(ctx, clk){
        runnable = runnable_;
    }

    virtual void Start();
};

namespace engines{

#define ENGINE_RUNNABLE() \
    [](ExecutionEngine* self)->void

    extern ExecutionEngine::engine_runnable_t IDEngineRunnable;
    extern ExecutionEngine::engine_runnable_t EXEngineRunnable;
    extern ExecutionEngine::engine_runnable_t DMEngineRunnable;
    extern ExecutionEngine::engine_runnable_t WBEngineRunnable;
} //namespace engines

#endif //ARCHIHW1_EXECUTIONENGINE_H


#ifndef ARCHIHW1_EXECUTIONENGINE_H
#define ARCHIHW1_EXECUTIONENGINE_H

#include "Types.h"
#include "Context.h"
#include "Task.h"
#include "tasks/TaskHandle.h"

#define FALLING_EDGE_FENCE() \
    clock.falling_edge.wait()

/*
 * Normal execution engine
 * For stages other than IF
 * */
class ExecutionEngine {

protected:
    Context* mContext;
    TaskHandle::ClockHandle& clock;

public:

    Context* GetContext(){
        //Wrapper for mutex in future
        return mContext;
    }

    ExecutionEngine(Context& ctx, TaskHandle::ClockHandle& clk) :
            mContext(&ctx),
            clock(clk){ }

    virtual void Start();
};

#endif //ARCHIHW1_EXECUTIONENGINE_H

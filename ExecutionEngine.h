
#ifndef ARCHIHW1_EXECUTIONENGINE_H
#define ARCHIHW1_EXECUTIONENGINE_H

#include "Types.h"
#include "Context.h"
#include "Task.h"

/*
 * Normal execution engine
 * For stages other than IF
 * */
class ExecutionEngine {

protected:
    Context* mContext;
    Instructions& mInstructions;

    void init(){
        task::InitInstructionMap();
        task::InitTasks();
    }

public:

    Context* GetContext(){
        //Wrapper for mutex in future
        return mContext;
    }

    ExecutionEngine(Context& ctx, Instructions& instructions) :
            mContext(&ctx),
            mInstructions(instructions) {
        ctx.setInstructionCount(static_cast<uint32_t>(mInstructions.length()));
        init();
    }

    virtual void Start();
};

#endif //ARCHIHW1_EXECUTIONENGINE_H

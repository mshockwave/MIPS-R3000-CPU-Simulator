
#ifndef ARCHIHW1_EXECUTIONENGINE_H
#define ARCHIHW1_EXECUTIONENGINE_H

#include "Types.h"
#include "Task.h"

class ExecutionEngine {
private:
    Context* mContext;
    Instructions& mInstructions;

    void init(){
        task::InitInstructionMap();
        task::InitTasks();
    }

    void dispatchTask(Instruction *instruction);

    void dispatchTask(Instruction *instruction, task_id_t taskId);

public:

    Context* getContext(){
        //Wrapper for mutex in future
        return mContext;
    }

    ExecutionEngine(Context& ctx, Instructions& instructions) :
            mContext(&ctx),
            mInstructions(instructions) {
        ctx.setInstructionCount(mInstructions.length());
        init();
    }

    void start();
};

#endif //ARCHIHW1_EXECUTIONENGINE_H

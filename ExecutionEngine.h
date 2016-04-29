
#ifndef ARCHIHW1_EXECUTIONENGINE_H
#define ARCHIHW1_EXECUTIONENGINE_H

#include "Types.h"
#include "Context.h"
#include "Task.h"
#include "Instruction.h"

class ExecutionEngine {
private:
    Context* mContext;
    Instructions& mInstructions;

    void init(){
        task::InitInstructionMap();
        task::InitTasks();
    }

    task_id_t dispatchTask(Instruction *instruction, task_id_t taskId);

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

    void Start();
};

#endif //ARCHIHW1_EXECUTIONENGINE_H

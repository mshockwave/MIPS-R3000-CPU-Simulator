
#ifndef ARCHIHW1_EXECUTIONENGINE_H
#define ARCHIHW1_EXECUTIONENGINE_H

#include "Types.h"
#include "Task.h"

class ExecutionEngine {
private:
    Context* mContext;

    void init(){
        task::InitInstructionMap();
        task::InitTasks();
    }

public:

    Context* GetContext(){
        //Wrapper for mutex in future
        return mContext;
    }

    ExecutionEngine(Context& ctx) :
            mContext(&ctx){
        init();
    }

    void DispatchTask(Instruction *instruction);

    void DispatchTask(Instruction* instruction, task_id_t taskId);
};

#endif //ARCHIHW1_EXECUTIONENGINE_H

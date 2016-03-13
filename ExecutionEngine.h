
#ifndef ARCHIHW1_EXECUTIONENGINE_H
#define ARCHIHW1_EXECUTIONENGINE_H

#include "Types.h"
#include "Task.h"

class ExecutionEngine {
private:
    static Context* sContext;

public:
    static void SetContext(Context &ctx){ sContext = &ctx; }
    static Context* GetContext(){
        //Wrapper for mutex in future
        return sContext;
    }

    static void Init(){
        task::InitInstructionMap();
        task::InitTasks();
    }

    static void DispatchTask(Instruction *instruction){
        //Extract op code
        uint32_t op = extractInstrBits(instruction->getBitsInstruction(), 31, 26);
        try{
            task_id_t taskId = task::FirstInstrOpMap.at(static_cast<uint8_t>(op));
            DispatchTask(instruction, taskId);
        }catch(...){
            //TODO: Error handling: op not found
        }
    }
    static void DispatchTask(Instruction* instruction, task_id_t taskId){
        if(taskId < (task::TASK_COUNT - 1/*excluding TASK_END*/)){
            task_id_t nextId = (task::TasksTable[taskId])(sContext, instruction);
            /*
             * Compiler's tail call optimization would eliminate this
             * (Probably...)
             * */
            DispatchTask(instruction, nextId);
        }
    }
};

#endif //ARCHIHW1_EXECUTIONENGINE_H

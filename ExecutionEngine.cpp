#include "ExecutionEngine.h"

void ExecutionEngine::dispatchTask(Instruction *instruction){
    //Extract op code
    uint32_t op = extractInstrBits(instruction->getBitsInstruction(), 31, 26);
    try{
        task_id_t taskId = task::FirstInstrOpMap.at(static_cast<uint8_t>(op));
        dispatchTask(instruction, taskId);
    }catch(...){
        //TODO: Error handling: op not found
    }
}

void ExecutionEngine::dispatchTask(Instruction *instruction, task_id_t taskId){
    if(taskId < (task::TASK_COUNT - 1/*excluding TASK_END*/)){
        task_id_t nextId = (task::TasksTable[taskId])(mContext, instruction);
        /*
         * Compiler's tail call optimization would eliminate this
         * (Probably...)
         * */
        dispatchTask(instruction, nextId);
    }
}

void ExecutionEngine::start() {

}
#include "ExecutionEngine.h"

void ExecutionEngine::nextTask() {
    const reg_t& pc = mContext->getPC();

    addr_t offset = pc - static_cast<addr_t>(0);
    uint32_t index = offset / WORD_WIDTH;

    Instructions::iterator itInstr = mInstructions.begin();
    Instruction& instruction = *(itInstr + index);

    //Extract op code
    uint32_t op = extractInstrBits(instruction.getBitsInstruction(), 31, 26);
    task::instr_task_map_t::iterator itOpMap = task::FirstInstrOpMap.find(static_cast<uint8_t>(op));
    if(UNLIKELY(itOpMap == task::FirstInstrOpMap.end())){
        //Not found
        //Halt
        return;
    }else{
        task_id_t taskId = itOpMap->second;
        dispatchTask(&instruction, taskId);
    }
}

void ExecutionEngine::dispatchTask(Instruction *instruction, task_id_t taskId){
    //Last catch
    if(UNLIKELY(taskId == task::OP_HALT || taskId == task::TASK_BAIL)) return;

    task_id_t nextId = (task::TasksTable[taskId])(mContext, instruction);

    if(nextId == task::OP_HALT || nextId == task::TASK_BAIL){
        //Do not dump snapshot for TASK_BAIL
        if(nextId == task::OP_HALT) mContext->dumpSnapshot();
        return;
    }

    /*
     * Compiler's tail call optimization would eliminate this
     * (Probably...)
     * */
    if(nextId == task::TASK_END){
        mContext->dumpSnapshot();
        mContext->incCycleCounter();
        nextTask();
    }else{
        //Instruction not finish, do not dump
        dispatchTask(instruction, nextId);
    }
}

void ExecutionEngine::start() {
    //Dump cycle zero state
    mContext->dumpSnapshot();
    mContext->incCycleCounter();

    nextTask();
}
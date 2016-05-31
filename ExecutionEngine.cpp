#include "ExecutionEngine.h"


task_id_t ExecutionEngine::dispatchTask(Instruction *instruction, task_id_t taskId){
    //Last catch
    if(UNLIKELY(taskId == task::OP_HALT || taskId == task::TASK_BAIL)) return taskId;

    task_id_t nextId = (task::TasksTable[taskId])(mContext, instruction);

    if(nextId == task::OP_HALT || nextId == task::TASK_BAIL){
        //Do not dump snapshot for TASK_BAIL
        if(nextId == task::OP_HALT) mContext->DumpSnapshot();
        return nextId;
    }

    if(nextId == task::TASK_END) {
        mContext->DumpSnapshot();
        mContext->IncCycleCounter();
        return nextId;
    }else{
        //Instruction not finish, do not dump
        return dispatchTask(instruction, nextId);
    }
}

void ExecutionEngine::Start() {
    //Dump cycle zero state
    mContext->DumpSnapshot();
    mContext->IncCycleCounter();

    while(true){
        const reg_t& pc = mContext->GetPC();

        if(pc < mContext->GetInstrStartAddress()){
            //Execute NOP until
            mContext->DumpSnapshot();
            mContext->IncCycleCounter();
            mContext->AdvancePC();
            continue;
        }

        addr_t offset = pc - (mContext->GetInstrStartAddress());
        uint32_t index = offset / WORD_WIDTH;

        Instructions::iterator itInstr = mInstructions.begin();
        Instruction instruction = *(itInstr + index);

        //Extract op code
        uint32_t op = extractInstrBits(instruction.GetBitsInstruction(), 31, 26);
        task::instr_task_map_t::iterator itOpMap = task::FirstInstrOpMap.find(static_cast<uint8_t>(op));
        if(UNLIKELY(itOpMap == task::FirstInstrOpMap.end())){
            //Not found
            //Halt
            return;
        }else{
            task_id_t taskId = itOpMap->second;
            taskId = dispatchTask(&instruction, taskId);
            if(taskId == task::OP_HALT || taskId == task::TASK_BAIL) return;
        }
    }
}
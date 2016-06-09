#include "ExecutionEngine.h"


task_id_t ExecutionEngine::dispatchTask(Instruction *instruction, task_id_t taskId){
    //Last catch
    if(UNLIKELY(taskId == task::TASK_BAIL)) return taskId;
    
    if(taskId == task::OP_HALT){
        if((++halt_counter) >= 1){
            // End Execution
            return task::OP_HALT;
        }else{
            mContext->AdvancePC();
            mContext->IncCycleCounter();
            return task::TASK_END;
        }
    }

    task_id_t nextId = (task::TasksTable[taskId])(mContext, instruction);

    if(nextId == task::OP_HALT || nextId == task::TASK_BAIL){
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

        /*
        if(pc < mContext->GetInstrStartAddress()){
            //Execute NOP until
            mContext->AdvancePC();
            mContext->DumpSnapshot();
            mContext->IncCycleCounter();
            
            mInstructions.HandleNOP();
            
            continue;
        }
         */

        addr_t offset = pc - (mContext->GetInstrStartAddress());
        uint32_t index = offset / WORD_WIDTH;
        
        DEBUG_BLOCK {
            Log::D("Execution Engine") << "=========================" << std::endl;;
            Log::D("Execution Engine") << "Ready to execute PC: 0x"
                                        << std::hex << static_cast<unsigned int>(pc) << std::endl;
            Log::D("Execution Engine") << "Index: "
                                        << std::dec << static_cast<unsigned int>(index) << std::endl;
            Log::D("Execution Engine") << "=========================" << std::endl;
        }

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
#include "IFEngine.h"

void IFEngine::Start(){

    bool stall = false;
    TaskHandle* task_obj = nullptr;

    while(true) {

        if (!(mContext->PcJump || stall || mContext->PcFlush)) {
            mContext->AdvancePC();
        } else {
            mContext->PcJump = false;
        }
        reg_t pc = mContext->GetPC();

        task_id_t next_task;
        Instruction *instr_ptr = nullptr;
        if (pc < mContext->GetInstrStartAddress()) {
            //Put NOP
            next_task = task::OP_NOP;
            instr_ptr = nullptr;
        } else {
            auto offset = (pc - mContext->GetInstrStartAddress()) / WORD_WIDTH;
            auto &instr = *(instructions.begin() + offset);
            instr_ptr = &instr;

            //Extract op code
            uint32_t op = extractInstrBits(instr.GetBitsInstruction(), 31, 26);
            if(op == 0x00){
                //R Type Instructions
                uint32_t func = extractInstrBits(instr.GetBitsInstruction(), 5, 0);
                task::instr_task_map_t::iterator itOpMap = task::RtypeInstrFuncMap.find(static_cast<uint8_t>(func));
                if (UNLIKELY(itOpMap == task::RtypeInstrFuncMap.end())) {
                    //Not found
                    //Halt
                    return;
                }
                next_task = itOpMap->second;
            }else{
                task::instr_task_map_t::iterator itOpMap = task::FirstInstrOpMap.find(static_cast<uint8_t>(op));
                if (UNLIKELY(itOpMap == task::FirstInstrOpMap.end())) {
                    //Not found
                    //Halt
                    return;
                }
                next_task = itOpMap->second;
            }
        }

        //TODO: Stop until five halts were read
        if (next_task == task::OP_HALT) break;

        auto &t = task::TasksTable[next_task];
        if(mContext->PcFlush && task_obj != nullptr){
            //Release previous
            delete task_obj;
        }
        if(mContext->PcFlush || task_obj == nullptr){
            task_obj = t.Get(mContext, instr_ptr, &clock);
        }

        mContext->PcFlush = false;

        //TODO: Error handling
        auto ret = task_obj->DoIF();
        stall = (ret == Error::PIPLINE_STALL);

        FALLING_EDGE_FENCE();

        //TODO: Print
        //TODO: Retrieve flush information here
    }
}

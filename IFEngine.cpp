#include "IFEngine.h"

void IFEngine::Start(){

    //bool stall = false;
    TaskHandle* task_obj = nullptr;

    //bool is_first_cycle = true;
    
    int halt_counter = 0;

    while(true) {

        reg_t pc = mContext->GetPC();
        mContext->PcQueue.Push(pc);

        task_id_t next_task;
        Instruction *instr_ptr = nullptr;
        if (pc < mContext->GetInstrStartAddress() || pc > mContext->GetInstrEndAddr()) {
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
                
                uint32_t instr_bits = instr.GetBitsInstruction();
                if(isEqualX(25, 21, instr_bits, 0x00000000)){
                    //sll $0, $0, $0
                    next_task = task::OP_NOP;
                }else{
                    //R Type Instructions
                    uint32_t func = extractInstrBits(instr_bits, 5, 0);
                    TRACE_DEBUG_BLOCK{
                        boost::mutex::scoped_lock lk(Log::Mux::D);
                        Log::D("IFEngine") << "R Type Func: " << (int)func << std::endl;
                    }
                    task::instr_task_map_t::iterator itOpMap = task::RtypeInstrFuncMap.find(static_cast<uint8_t>(func));
                    if (UNLIKELY(itOpMap == task::RtypeInstrFuncMap.end())) {
                        //Not found
                        //Halt
                        TRACE_DEBUG_BLOCK{
                            boost::mutex::scoped_lock lk(Log::Mux::D);
                            Log::D("IFEngine") << "R Type func not found!" << std::endl;
                        }
                        return;
                    }
                    next_task = itOpMap->second;
                }
                
            }else{
                TRACE_DEBUG_BLOCK{
                    boost::mutex::scoped_lock lk(Log::Mux::D);
                    Log::D("IFEngine") << "Op: " << (int)op << std::endl;
                }
                task::instr_task_map_t::iterator itOpMap = task::FirstInstrOpMap.find(static_cast<uint8_t>(op));
                if (UNLIKELY(itOpMap == task::FirstInstrOpMap.end())) {
                    //Not found
                    //Halt
                    TRACE_DEBUG_BLOCK{
                        boost::mutex::scoped_lock lk(Log::Mux::D);
                        Log::D("IFEngine") << "Op not found: " << (int)op << std::endl;
                    }
                    return;
                }
                next_task = itOpMap->second;
            }
        }

        auto &t = task::TasksTable[next_task];
        if(mContext->PcFlush.fetch_sub(1) > 0 && task_obj != nullptr){
            //Release previous
            delete task_obj;
        }
        if( !mContext->IFStall || task_obj == nullptr){
            task_obj = t.Get(mContext,
                             instr_ptr,
                             pc,
                             &clock);
        }

        TRACE_DEBUG_BLOCK{
            boost::mutex::scoped_lock lk(Log::Mux::D);
            Log::D("IFEngine") << "Next Task: " << task_obj->name << std::endl;
        };

        //TODO: Error handling
        /*auto ret = */task_obj->DoIF();
        //stall = (ret == Error::PIPELINE_STALL);

        bool ready_to_abort = false;
        try{
            FALLING_EDGE_FENCE();
        }catch(boost::thread_interrupted&){
            ready_to_abort = true;
        }
        
        bool ready_to_dead = (task_obj->task_id == task::OP_HALT) &&
                                ( ((mContext->IFStall)? halt_counter : ++halt_counter) >= 4 );
        if(ready_to_dead){
            ScopedReadWriteLock::WriteLock lk(mContext->DeadThreadMux);
            mContext->DeadThreadNum++;
        }

        if(instr_ptr != nullptr){
            std::stringstream ss;
            ss << std::setfill('0') << "0x" << std::setw(8) << std::hex << std::uppercase << instr_ptr->GetBitsInstruction();
            if(mContext->PcFlush.load() > 0){
                ss << " to_be_flushed";
            }else if(mContext->IFStall){
                ss << " to_be_stalled";
            }
            mContext->IFMessageQueue.Push(ss.str());
            
        }else if(task_obj->task_id == task::OP_NOP){
            std::stringstream ss;
            ss << "0x00000000";
            if(mContext->PcFlush.load() > 0){
                ss << " to_be_flushed";
            }else if(mContext->IFStall){
                ss << " to_be_stalled";
            }
            mContext->IFMessageQueue.Push(ss.str());
        }
        
        if (!(mContext->PcJump ||
              mContext->PcFlush.load() > 0 ||
              mContext->IFStall)) {
            mContext->AdvancePC();
        } else {
            mContext->PcJump = false;
        }

        if(ready_to_abort){
            mContext->IFMessageQueue.Push(Context::MSG_END);
            mContext->PcQueue.Push(mContext->GetPC());
            return;
        }

        if(ready_to_dead){
            mContext->IFMessageQueue.Push(Context::MSG_END);
            mContext->PcQueue.Push(mContext->GetPC());
            break;
        }
    }

    while(mContext->DeadThreadNum < THREAD_COUNT){
        //Mortuary zone

        clock.rising_edge.wait();

        try{
            FALLING_EDGE_FENCE();
        }catch(boost::thread_interrupted&){
            //mContext->IFMessageQueue.Push(Context::MSG_END);
            return;
        }
    }
}

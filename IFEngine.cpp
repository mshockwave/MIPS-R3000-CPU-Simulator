#include "IFEngine.h"

void IFEngine::Start(){

    //bool stall = false;
    TaskHandle* task_obj = nullptr;

    bool is_first_cycle = true;

    while(true) {

        if (!(  mContext->PcJump ||
                mContext->PcFlush ||
                mContext->IFStall.load() ||
                is_first_cycle )) {
            mContext->AdvancePC();
        } else {
            mContext->PcJump = false;
            is_first_cycle = false;
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
                        Log::D("IFEngine") << "Op not found!" << std::endl;
                    }
                    return;
                }
                next_task = itOpMap->second;
            }
        }

        auto &t = task::TasksTable[next_task];
        if(mContext->PcFlush && task_obj != nullptr){
            //Release previous
            delete task_obj;
        }
        if( (!mContext->IFStall.load()) || task_obj == nullptr){
            task_obj = t.Get(mContext, instr_ptr, &clock);
        }

        TRACE_DEBUG_BLOCK{
            boost::mutex::scoped_lock lk(Log::Mux::D);
            Log::D("IFEngine") << "Next Task: " << task_obj->name << std::endl;
        };

        mContext->PcFlush = false;

        //TODO: Error handling
        /*auto ret = */task_obj->DoIF();
        //stall = (ret == Error::PIPELINE_STALL);

        bool ready_to_dead = (task_obj->task_id == task::OP_HALT);
        if(ready_to_dead){
            ScopedReadWriteLock::WriteLock lk(mContext->DeadThreadMux);
            mContext->DeadThreadNum++;
        }

        FALLING_EDGE_FENCE();

        if(instr_ptr != nullptr){
            std::stringstream ss;
            ss << std::setfill('0') << "0x" << std::setw(8) << std::hex << std::uppercase << instr_ptr->GetBitsInstruction();
            if(mContext->PcFlush){
                ss << " to_be_flushed";
            }else if(mContext->IFStall.load()){
                ss << " to_be_stalled";
            }
            mContext->IFMessageQueue.Push(ss.str());
        }

        if(ready_to_dead){
            mContext->IFMessageQueue.Push(Context::MSG_END);
            break;
        }
    }

    while(mContext->DeadThreadNum < THREAD_COUNT){
        //Mortuary zone

        clock.rising_edge.wait();

        FALLING_EDGE_FENCE();
    }
}

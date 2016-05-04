#include "ExecutionEngine.h"

void ExecutionEngine::Start() {
    if(runnable){
        runnable(this);
    }
}

namespace engines{

    ExecutionEngine::engine_runnable_t IDEngineRunnable = ENGINE_RUNNABLE(){

        bool stall = false;
        TaskHandle* task_obj = nullptr;
        auto* ctx = self->GetContext();
        const auto& clock = self->GetClock();

        while(true){

            if(!stall){
                if(ctx->IF_ID.empty()){
                    //Insert NOP
                    task_obj = task::TasksTable[task::OP_NOP].Get(ctx, nullptr, U32_0, &clock);
                }else if(ctx->PcFlush.fetch_sub(1) > 0){
                    //Pop old task and insert NOP
                    ctx->IF_ID.erase(ctx->IF_ID.begin());
                    task_obj = task::TasksTable[task::OP_NOP].Get(ctx, nullptr, U32_0, &clock);
                }else{
                    //Get from IF_ID stage register
                    task_obj = ctx->IF_ID.front();
                    //Pop front
                    ctx->IF_ID.erase(ctx->IF_ID.begin());
                }
            }

            TRACE_DEBUG_BLOCK{
                boost::mutex::scoped_lock lk(Log::Mux::D);
                if(task_obj == nullptr){
                    Log::D("IDEngine") << "Next Task null!" << std::endl;
                }else{
                    Log::D("IDEngine") << "Next Task: " << task_obj->name << std::endl;
                }
            };

            if(task_obj == nullptr) continue;

            auto err = task_obj->DoID();
            stall = (err == Error::PIPELINE_STALL);
            ctx->IFStall = stall;

            //ID Forwarding stuff
            int rs_forward_reg_id = -1;
            int rt_forward_reg_id = -1;
            if(ctx->RegReserves[task_obj->RsIndex].IDForward){
                rs_forward_reg_id = task_obj->RsIndex;
            }
            if(ctx->RegReserves[task_obj->RtIndex].IDForward &&
               task_obj->ExportReg != TaskHandle::RegKind::kRt){
                rt_forward_reg_id = task_obj->RtIndex;
            }

            bool ready_to_dead = task_obj->task_id == task::OP_HALT;
            if(ready_to_dead){
                ScopedReadWriteLock::WriteLock lk(ctx->DeadThreadMux);
                ctx->DeadThreadNum++;
            }

            bool ready_to_abort = false;
            try{
                FALLING_EDGE_FENCE();
            }catch(boost::thread_interrupted&){
                ready_to_abort = true;
            }

            std::stringstream ss;
            ss << task_obj->name;
            if(stall){
                ss << " to_be_stalled";
            }else{
                if(rs_forward_reg_id > 0){
                    ss << " fwd_EX-DM_rs_$" << rs_forward_reg_id;
                }
                if(rt_forward_reg_id > 0){
                    ss << " fwd_EX-DM_rt_$" << rt_forward_reg_id;
                }
            }
            ctx->IDMessageQueue.Push(ss.str());

            if(ready_to_abort){
                ctx->IDMessageQueue.Push(Context::MSG_END);
                return;
            }

            if(ready_to_dead){
                ctx->IDMessageQueue.Push(Context::MSG_END);
                break;
            }
        }

        while(ctx->DeadThreadNum < THREAD_COUNT){
            //Mortuary zone

            clock.rising_edge.wait();

            try{
                FALLING_EDGE_FENCE();
            }catch(boost::thread_interrupted&){
                //ctx->IDMessageQueue.Push(Context::MSG_END);
                return;
            }
        }
    };

    ExecutionEngine::engine_runnable_t EXEngineRunnable = ENGINE_RUNNABLE(){

        auto* ctx = self->GetContext();
        const auto& clock = self->GetClock();
        
        int cycle = 0;

        while(true){

            TaskHandle *task_obj;
            if(ctx->ID_EX.empty()){
                //Insert NOP
                task_obj = task::TasksTable[task::OP_NOP].Get(ctx, nullptr, U32_0, &clock);
            }else{
                //Get from ID_EX stage register
                task_obj = ctx->ID_EX.front();
                //Pop front
                ctx->ID_EX.erase(ctx->ID_EX.begin());
            }

            TRACE_DEBUG_BLOCK{
                boost::mutex::scoped_lock lk(Log::Mux::D);
                if(task_obj == nullptr){
                    Log::D("EXEngine") << "Next Task null!" << std::endl;
                }else{
                    Log::D("EXEngine") << "Next Task: " << task_obj->name << std::endl;
                }
            };

            if(task_obj == nullptr) continue;

            //Forwarding message
            int rt_forward_reg_id = -1;
            int rs_forward_reg_id = -1;
            if(ctx->RegReserves[task_obj->RsIndex].EXForward){
                rs_forward_reg_id =  task_obj->RsIndex;
            }
            if(ctx->RegReserves[task_obj->RtIndex].EXForward &&
               task_obj->ExportReg != TaskHandle::RegKind::kRt){
                rt_forward_reg_id =  task_obj->RtIndex;
            }

            //TODO: Error handling
            auto err = task_obj->DoEX();
            bool stall = (err == Error::PIPELINE_STALL);
            
            if(err != Error::NONE && !stall){
                err.SetCycle(cycle+1);
                ctx->EXErrorQueue.Push(err);
                if(err.GetErrorLevel() >= Error::LEVEL_HALT){
                    //Halt
                    ctx->ThreadGroup->interrupt_all();
                }
            }

            bool ready_to_dead = task_obj->task_id == task::OP_HALT;
            if(ready_to_dead){
                ScopedReadWriteLock::WriteLock lk(ctx->DeadThreadMux);
                ctx->DeadThreadNum++;
            }

            bool ready_to_abort = false;
            try{
                FALLING_EDGE_FENCE();
            }catch(boost::thread_interrupted&){
                ready_to_abort = true;
            }
            
            //Clear forwarding register EX available flag
            if(task_obj->ExportReg != TaskHandle::RegKind::kNone){
                ctx->RegReserves[task_obj->ExportRegIndex].EXAvailable = false;
            }

            std::stringstream ss;
            ss << task_obj->name;
            if(stall){
                ss << " to_be_stalled";
            }else{
                if(rs_forward_reg_id > 0){
                    ss << " fwd_EX-DM_rs_$" << rs_forward_reg_id;
                }
                if(rt_forward_reg_id > 0){
                    ss << " fwd_EX-DM_rt_$" << rt_forward_reg_id;
                }
            }
            ctx->EXMessageQueue.Push(ss.str());

            if(ready_to_abort){
                ctx->EXMessageQueue.Push(Context::MSG_END);
                return;
            }

            if(ready_to_dead){
                ctx->EXMessageQueue.Push(Context::MSG_END);
                break;
            }
            
            cycle++;
        }

        while(ctx->DeadThreadNum < THREAD_COUNT){
            //Mortuary zone

            clock.rising_edge.wait();

            try{
                FALLING_EDGE_FENCE();
            }catch(boost::thread_interrupted&){
                //ctx->EXMessageQueue.Push(Context::MSG_END);
                return;
            }
        }
    };

    ExecutionEngine::engine_runnable_t DMEngineRunnable = ENGINE_RUNNABLE(){

        auto* ctx = self->GetContext();
        const auto& clock = self->GetClock();
        
        int cycle = 0;

        while(true){

            TaskHandle *task_obj;
            if(ctx->EX_DM.empty()){
                //Insert NOP
                task_obj = task::TasksTable[task::OP_NOP].Get(ctx, nullptr, U32_0, &clock);
            }else{
                //Get from ID_EX stage register
                task_obj = ctx->EX_DM.front();
                //Pop front
                ctx->EX_DM.erase(ctx->EX_DM.begin());
            }

            TRACE_DEBUG_BLOCK{
                boost::mutex::scoped_lock lk(Log::Mux::D);
                if(task_obj == nullptr){
                    Log::D("DMEngine") << "Next Task null!" << std::endl;
                }else{
                    Log::D("DMEngine") << "Next Task: " << task_obj->name << std::endl;
                }
            };

            if(task_obj == nullptr) continue;

            auto err = task_obj->DoDM();
            bool stall = (err == Error::PIPELINE_STALL);
            
            if(err != Error::NONE && !stall){
                err.SetCycle(cycle+1);
                ctx->DMErrorQueue.Push(err);
                if(err.GetErrorLevel() >= Error::LEVEL_HALT){
                    //Halt
                    ctx->ThreadGroup->interrupt_all();
                }
            }

            bool ready_to_dead = task_obj->task_id == task::OP_HALT;
            if(ready_to_dead){
                ScopedReadWriteLock::WriteLock lk(ctx->DeadThreadMux);
                ctx->DeadThreadNum++;
            }

            bool ready_to_abort = false;
            try{
                FALLING_EDGE_FENCE();
            }catch(boost::thread_interrupted&){
                ready_to_abort = true;
            }

            std::stringstream ss;
            ss << task_obj->name;
            if(stall) ss << " to_be_stalled";
            ctx->DMMessageQueue.Push(ss.str());

            if(ready_to_abort){
                ctx->DMMessageQueue.Push(Context::MSG_END);
                return;
            }

            if(ready_to_dead){
                ctx->DMMessageQueue.Push(Context::MSG_END);
                break;
            }
            
            cycle++;
        }

        while(ctx->DeadThreadNum < THREAD_COUNT){
            //Mortuary zone

            clock.rising_edge.wait();

            try{
                FALLING_EDGE_FENCE();
            }catch(boost::thread_interrupted&){
                //ctx->DMMessageQueue.Push(Context::MSG_END);
                return;
            }
        }
    };

    ExecutionEngine::engine_runnable_t WBEngineRunnable = ENGINE_RUNNABLE(){

        auto* ctx = self->GetContext();
        const auto& clock = self->GetClock();

        int cycle = 0;

        while(true){

            TaskHandle *task_obj;
            if(ctx->DM_WB.empty()){
                //Insert NOP
                task_obj = task::TasksTable[task::OP_NOP].Get(ctx, nullptr, U32_0, &clock);
            }else{
                //Get from ID_EX stage register
                task_obj = ctx->DM_WB.front();
                //Pop front
                ctx->DM_WB.erase(ctx->DM_WB.begin());
            }

            TRACE_DEBUG_BLOCK{
                boost::mutex::scoped_lock lk(Log::Mux::D);
                if(task_obj == nullptr){
                    Log::D("WBEngine") << "Next Task null!" << std::endl;
                }else{
                    Log::D("WBEngine") << "Next Task: " << task_obj->name << std::endl;
                }
            };

            if(task_obj == nullptr) continue;

            auto err = task_obj->DoWB();
            bool stall = (err == Error::PIPELINE_STALL);

            //Record modified registers
            RegsDiff regs_diff;
            uint8_t m_index = task_obj->ModifyRegIndex;
            if( m_index > 0 && m_index < REGISTER_COUNT){
                regs_diff.RegIndex = m_index;
                regs_diff.RegValue = ctx->Registers[m_index];
            }

            if(err != Error::NONE && !stall){
                err.SetCycle(cycle+1);
                ctx->WBErrorQueue.Push(err);
                if(err.GetErrorLevel() >= Error::LEVEL_HALT){
                    //Halt
                    ctx->ThreadGroup->interrupt_all();
                }
            }

            bool ready_to_dead = task_obj->task_id == task::OP_HALT;
            if(ready_to_dead){
                ScopedReadWriteLock::WriteLock lk(ctx->DeadThreadMux);
                ctx->DeadThreadNum++;
            }

            bool ready_to_abort = false;
            try{
                FALLING_EDGE_FENCE();
            }catch(boost::thread_interrupted&){
                ready_to_abort = true;
            }

            std::stringstream ss;
            ss << task_obj->name;
            if(stall) ss << " to_be_stalled";
            ctx->WBMessageQueue.Push(ss.str());

            delete task_obj;

            if(ready_to_abort){
                regs_diff.Abort = true;
                ctx->RegsQueue.Push(regs_diff);
                ctx->WBMessageQueue.Push(Context::MSG_END);
                return;
            }

            if(ready_to_dead){
                regs_diff.Terminated = true;
                ctx->RegsQueue.Push(regs_diff);
                ctx->WBMessageQueue.Push(Context::MSG_END);
                break;
            }
            
            ctx->RegsQueue.Push(regs_diff);

            cycle++;
        }

        while(ctx->DeadThreadNum < THREAD_COUNT){
            //Mortuary zone

            clock.rising_edge.wait();

            try{
                FALLING_EDGE_FENCE();
            }catch(boost::thread_interrupted&){
                //ctx->WBMessageQueue.Push(Context::MSG_END);
                return;
            }
        }

    };
} //namespace engines
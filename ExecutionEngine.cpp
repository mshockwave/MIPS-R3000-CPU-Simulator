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
                    task_obj = task::TasksTable[task::OP_NOP].Get(ctx, nullptr, &clock);
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
            ctx->IFStall.store(stall);

            bool ready_to_dead = task_obj->task_id == task::OP_HALT;
            if(ready_to_dead){
                ScopedReadWriteLock::WriteLock lk(ctx->DeadThreadMux);
                ctx->DeadThreadNum++;
            }

            FALLING_EDGE_FENCE();

            //TODO: Print here

            if(ready_to_dead) break;
        }

        while(ctx->DeadThreadNum < THREAD_COUNT){
            //Mortuary zone

            clock.rising_edge.wait();

            FALLING_EDGE_FENCE();
        }
    };

    ExecutionEngine::engine_runnable_t EXEngineRunnable = ENGINE_RUNNABLE(){

        auto* ctx = self->GetContext();
        const auto& clock = self->GetClock();

        while(true){

            TaskHandle *task_obj;
            if(ctx->ID_EX.empty()){
                //Insert NOP
                task_obj = task::TasksTable[task::OP_NOP].Get(ctx, nullptr, &clock);
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

            //TODO: Error handling
            /*auto err = */task_obj->DoEX();

            bool ready_to_dead = task_obj->task_id == task::OP_HALT;
            if(ready_to_dead){
                ScopedReadWriteLock::WriteLock lk(ctx->DeadThreadMux);
                ctx->DeadThreadNum++;
            }

            FALLING_EDGE_FENCE();

            //TODO: Print here

            if(ready_to_dead) break;
        }

        while(ctx->DeadThreadNum < THREAD_COUNT){
            //Mortuary zone

            clock.rising_edge.wait();

            FALLING_EDGE_FENCE();
        }
    };

    ExecutionEngine::engine_runnable_t DMEngineRunnable = ENGINE_RUNNABLE(){

        auto* ctx = self->GetContext();
        const auto& clock = self->GetClock();

        while(true){

            TaskHandle *task_obj;
            if(ctx->EX_DM.empty()){
                //Insert NOP
                task_obj = task::TasksTable[task::OP_NOP].Get(ctx, nullptr, &clock);
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

            //TODO: Error handling
            /*auto err = */task_obj->DoDM();

            bool ready_to_dead = task_obj->task_id == task::OP_HALT;
            if(ready_to_dead){
                ScopedReadWriteLock::WriteLock lk(ctx->DeadThreadMux);
                ctx->DeadThreadNum++;
            }

            FALLING_EDGE_FENCE();

            //TODO: Print here

            if(ready_to_dead) break;
        }

        while(ctx->DeadThreadNum < THREAD_COUNT){
            //Mortuary zone

            clock.rising_edge.wait();

            FALLING_EDGE_FENCE();
        }
    };

    ExecutionEngine::engine_runnable_t WBEngineRunnable = ENGINE_RUNNABLE(){

        auto* ctx = self->GetContext();
        const auto& clock = self->GetClock();

        while(true){

            TaskHandle *task_obj;
            if(ctx->DM_WB.empty()){
                //Insert NOP
                task_obj = task::TasksTable[task::OP_NOP].Get(ctx, nullptr, &clock);
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

            //TODO: Error handling
            /*auto err = */task_obj->DoWB();

            bool ready_to_dead = task_obj->task_id == task::OP_HALT;
            if(ready_to_dead){
                ScopedReadWriteLock::WriteLock lk(ctx->DeadThreadMux);
                ctx->DeadThreadNum++;
            }

            FALLING_EDGE_FENCE();

            //TODO: Print here

            if(ready_to_dead) break;
        }

        while(ctx->DeadThreadNum < THREAD_COUNT){
            //Mortuary zone

            clock.rising_edge.wait();

            FALLING_EDGE_FENCE();
        }

        //TODO: Release task memory!!
    };
} //namespace engines
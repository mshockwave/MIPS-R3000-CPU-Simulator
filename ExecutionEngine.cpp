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

        while(true){
            auto* ctx = self->GetContext();
            const auto& clock = self->GetClock();

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

            if(task_obj == nullptr) continue;
            //FIXME: Propagate HALT to next stage instead
            if(task_obj->task_id == task::OP_HALT) break;

            auto err = task_obj->DoID();
            stall = (err == Error::PIPELINE_STALL);
            ctx->IFStall.store(stall);

            FALLING_EDGE_FENCE();

            //TODO: Print here
        }
    };

    ExecutionEngine::engine_runnable_t EXEngineRunnable = ENGINE_RUNNABLE(){

        while(true){
            auto* ctx = self->GetContext();
            const auto& clock = self->GetClock();

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

            if(task_obj == nullptr) continue;
            //FIXME: Propagate HALT to next stage instead
            if(task_obj->task_id == task::OP_HALT) break;

            //TODO: Error handling
            /*auto err = */task_obj->DoEX();

            FALLING_EDGE_FENCE();

            //TODO: Print here
        }
    };

    ExecutionEngine::engine_runnable_t DMEngineRunnable = ENGINE_RUNNABLE(){

        while(true){
            auto* ctx = self->GetContext();
            const auto& clock = self->GetClock();

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

            if(task_obj == nullptr) continue;
            //FIXME: Propagate HALT to next stage instead
            if(task_obj->task_id == task::OP_HALT) break;

            //TODO: Error handling
            /*auto err = */task_obj->DoDM();

            FALLING_EDGE_FENCE();

            //TODO: Print here
        }
    };

    ExecutionEngine::engine_runnable_t WBEngineRunnable = ENGINE_RUNNABLE(){

        while(true){
            auto* ctx = self->GetContext();
            const auto& clock = self->GetClock();

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

            if(task_obj == nullptr) continue;
            //FIXME: Propagate HALT to next stage instead
            if(task_obj->task_id == task::OP_HALT) break;

            //TODO: Error handling
            /*auto err = */task_obj->DoWB();

            FALLING_EDGE_FENCE();

            //TODO: Print here
        }
    };
} //namespace engines
#include "IDEngine.h"

void IDEngine::Start() override {
    while(true){
        auto* ctx = GetContext();

        //Fetch raw instruction and decode
        auto* instruction = ctx->IF_ID.Instr.GetCurrent();

        auto op = static_cast<uint8_t>(extractInstrBits(instruction->GetBitsInstruction(),
                                                        31, 26));
        task::instr_task_map_t::iterator it_task;
        if( (it_task = task::FirstInstrOpMap.find(op)) == task::FirstInstrOpMap.end() ){
            //TODO: Not Found
        }
        auto task_id = it_task->second;

        if(task_id == task::OP_HALT){
            //TODO: Handle HALT
            break;
        }

        auto status = (task::TasksTable[task_id])(ctx, instruction);
        if(status != Error::NONE){
            //TODO: Handle Error
        }

        //Wait until next clock

    }
}

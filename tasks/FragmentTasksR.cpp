#include "FragmentTasksR.h"

namespace task {

    void InitFragmentTasksR(){
        TasksTable[DM_R_NOP] = TASK_HANDLER() {

            //Carry data to next stage
            context->DM_WB.WB = context->EXE_DM.WB;

            //TODO: Forwarding

            return Error::NONE;
        };

        TasksTable[WB_R_WRITE_REGS] = TASK_HANDLER() {

            //Write rd back
            auto regs = context->DM_WB.WB.Regs.GetCurrent();
            auto rd_index = regs.Rd_Index;
            auto rd_value = regs.Rd;

            //TODO: Assert rd_index != $zero
            context->Registers[rd_index] = rd_value;

            return Error::NONE;
        };
    }
}

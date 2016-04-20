#include "FirstClassTasks.h"

namespace task{
    void InitFirstClassTasks(){
        TasksTable[ID_R_TYPE] = TASK_HANDLER() {
            uint8_t func = static_cast<uint8_t>(extractInstrBits(instruction->GetBitsInstruction(),
                                                                 5, 0));
            instr_task_map_t::iterator itOp;
            if( (itOp = RtypeInstrFuncMap.find(func)) == RtypeInstrFuncMap.end() ){
                //TODO: Not found
            }

            //EXE
            auto next_task = itOp->second;
            context->ID_EXE.EXE.Task.WriteNext(next_task);

            if(func == 0x00/*sll*/ ||
               func == 0x02/*srl*/ ||
               func == 0x03/*sra*/){
                RInstr::loadIDRegs<false>(context, instruction);
            }else if(func == 0x08/*jr*/){
                RInstr::loadIDRegs<true,false,false>(context, instruction);
            }else{
                RInstr::loadIDRegs(context, instruction);
            }

            if(func != 0x08/*jr*/){
                //DM
                context->ID_EXE.DM.Task = DM_R_NOP;

                //WB
                context->ID_EXE.WB.Task = WB_R_WRITE_REGS;
            }else{
                //TODO: DM and WB of jr
            }

            return Error::NONE;
        };
    }
} //namespace task

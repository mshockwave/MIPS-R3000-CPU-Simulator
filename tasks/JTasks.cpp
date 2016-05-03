#include "JTasks.h"

namespace task{
    
    namespace JInstr{
        
        TaskHandle::stage_task_t EmptyIF = STAGE_TASK(){
            auto* ctx = self->context;
            
            RISING_EDGE_FENCE();
            
            return (ctx->pushTask(ctx->IF_ID, self))? Error::NONE : Error::PIPELINE_STALL;
        };
        
        TaskHandle::stage_task_t EmptyEX = STAGE_TASK(){
            auto* ctx = self->context;
            
            RISING_EDGE_FENCE();
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        };
        
        TaskHandle::stage_task_t DefaultDM = STAGE_TASK(){
            auto* ctx = self->context;
            
            RISING_EDGE_FENCE();
            //Clear forwarding register ID available flag
            if(self->ExportReg != TaskHandle::RegKind::kNone){
                ctx->RegReserves[self->ExportRegIndex].IDAvailable = false;
            }
            
            return (ctx->pushTask(ctx->DM_WB, self))? Error::NONE : Error::PIPELINE_STALL;
        };
        
        TaskHandle::stage_task_t EmptyWB = STAGE_TASK(){
            
            RISING_EDGE_FENCE();
            
            return Error::NONE;
        };
        
    }; //namespace JInstr
    
    void InitJTasks(){
        
        TasksTable[OP_J].Name("J", OP_J)
        .IF(JInstr::EmptyIF)
        .ID(STAGE_TASK(){
            auto* ctx = self->context;
            reg_t pc = self->instruction_address;
            
            RISING_EDGE_FENCE();
            
            uint32_t addr = JInstr::GetAddr(self->instruction->GetBitsInstruction());
            pc += WORD_WIDTH;
            uint32_t partPC = extractInstrBits(pc, 31, 28);
            pc = (partPC << 28);
            
            ctx->PcFlush = Context::PC_FLUSH_CONSUMER_COUNT;
            Error e = ctx->SetPC( pc | (addr << 2) );
            
            return (ctx->pushTask(ctx->ID_EX, self))? e : Error::PIPELINE_STALL;
        })
        .EX(JInstr::EmptyEX)
        .DM(JInstr::DefaultDM)
        .WB(JInstr::EmptyWB);
        
        TasksTable[OP_JAL].Name("JAL", OP_JAL)
        .ExportRegister(TaskHandle::RegKind::kRd)
        .IF(STAGE_TASK(){
            auto* ctx = self->context;
            
            RISING_EDGE_FENCE();
            self->ExportRegIndex = 31; //$ra
            
            return (ctx->pushTask(ctx->IF_ID, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .ID(STAGE_TASK(){
            auto* ctx = self->context;
            reg_t pc = self->instruction_address;
            
            RISING_EDGE_FENCE();
            
            uint32_t addr = JInstr::GetAddr(self->instruction->GetBitsInstruction());
            pc += WORD_WIDTH;
            //Store $ra value in self->Rd
            self->RdValue = pc;
            self->RdIndex = 31; //$ra
            
            uint32_t partPC = extractInstrBits(pc, 31, 28);
            pc = (partPC << 28);
            
            ctx->PcFlush = Context::PC_FLUSH_CONSUMER_COUNT;
            Error e = ctx->SetPC( pc | (addr << 2) );
            
            //Reserve $ra
            ctx->RegReserves[self->RdIndex].Reset(self);
            
            return (ctx->pushTask(ctx->ID_EX, self))? e : Error::PIPELINE_STALL;
        })
        .EX(STAGE_TASK(){
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            reg_reserves[self->RdIndex].EXAvailable = true;
            
            RISING_EDGE_FENCE();
            
            //Do the forwarding job
            reg_reserves[self->RdIndex].Value = self->RdValue;
            reg_reserves[self->RdIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(JInstr::DefaultDM)
        .WB(STAGE_TASK(){
            auto* ctx = self->context;
            
            ctx->Registers[self->RdIndex] = self->RdValue;
            self->ModifyRegIndex = self->RdIndex;
            
            //Clean destination register reservation
            if(ctx->RegReserves[self->RdIndex].Holder == self){
                ctx->RegReserves[self->RdIndex].Reset(nullptr);
            }
            
            RISING_EDGE_FENCE();
            
            return Error::NONE;
        });
        
    }
}

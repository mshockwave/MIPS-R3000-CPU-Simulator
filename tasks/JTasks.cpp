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
        
        TaskHandle::stage_task_t EmptyDM = STAGE_TASK(){
            auto* ctx = self->context;
            
            RISING_EDGE_FENCE();
            
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
            
            RISING_EDGE_FENCE();
            
            uint32_t addr = JInstr::GetAddr(self->instruction->GetBitsInstruction());
            reg_t pc = ctx->GetPC();
            pc += WORD_WIDTH;
            uint32_t partPC = extractInstrBits(pc, 31, 28);
            pc = (partPC << 28);
            
            ctx->PcFlush = Context::PC_FLUSH_CONSUMER_COUNT;
            Error e = ctx->SetPC( pc | (addr << 2) );
            
            return (ctx->pushTask(ctx->ID_EX, self))? e : Error::PIPELINE_STALL;
        })
        .EX(JInstr::EmptyEX)
        .DM(JInstr::EmptyDM)
        .WB(JInstr::EmptyWB);
        
        TasksTable[OP_JAL].Name("JAL", OP_JAL)
        .IF(JInstr::EmptyIF)
        .ID(STAGE_TASK(){
            auto* ctx = self->context;
            
            RISING_EDGE_FENCE();
            
            uint32_t addr = JInstr::GetAddr(self->instruction->GetBitsInstruction());
            reg_t pc = ctx->GetPC();
            pc += WORD_WIDTH;
            ctx->RA = pc;
            
            uint32_t partPC = extractInstrBits(pc, 31, 28);
            pc = (partPC << 28);
            
            ctx->PcFlush = Context::PC_FLUSH_CONSUMER_COUNT;
            Error e = ctx->SetPC( pc | (addr << 2) );
            
            return (ctx->pushTask(ctx->ID_EX, self))? e : Error::PIPELINE_STALL;
        })
        .EX(JInstr::EmptyEX)
        .DM(JInstr::EmptyDM)
        .WB(JInstr::EmptyWB);
        
    }
}

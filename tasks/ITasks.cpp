#include "ITasks.h"

namespace task{
    void InitITasks(){
        /*
         * Load rs, rt index
         * */
        TaskHandle::stage_task_t ResolveRegsIF = STAGE_TASK(){
            /*
             * This IF lambda is always executed in last half cycle
             * Since first half cycle is reserved for instruction loading
             * */
            RISING_EDGE_FENCE();

            //Decode registers index
            auto instr_bits = self->instruction->GetBitsInstruction();
            self->RtIndex = IInstr::GetRt(instr_bits);
            self->RsIndex = IInstr::GetRs(instr_bits);

            auto* ctx = self->context;
            return (ctx->pushTask(ctx->IF_ID, self))? Error::NONE : Error::PIPELINE_STALL;
        };

        TasksTable[OP_BEQ].Name("BEQ", OP_BEQ)
                .IF(ResolveRegsIF)
                .ID(STAGE_TASK(){
                    auto* ctx = self->context;
                    auto& reg_reserves = ctx->RegReserves;

                    auto rt_index = self->RtIndex;
                    auto rs_index = self->RsIndex;

                    bool rs_load = false, rt_load = false;
                    reg_t rs_value = 0, rt_value = 0;

                    // Load ID forwarding stuff here
                    // To avoiding race condition on falling edge
                    if(reg_reserves[rs_index].Holder != nullptr &&
                            reg_reserves[rs_index].IDAvailable){
                        rs_value = reg_reserves[rs_index].Value;
                        rs_load = true;
                        reg_reserves[rs_index].IDForward = true;
                    }
                    if(reg_reserves[rt_index].Holder != nullptr &&
                       reg_reserves[rt_index].IDAvailable){
                        rt_value = reg_reserves[rt_index].Value;
                        rt_load = true;
                        reg_reserves[rt_index].IDForward = true;
                    }

                    RISING_EDGE_FENCE();

                    bool need_wait = false;
                    if(reg_reserves[rs_index].Holder == nullptr){
                        rs_value = ctx->Registers[rs_index];
                    }else if(!rs_load){
                        need_wait = true;
                    }

                    if(reg_reserves[rt_index].Holder == nullptr){
                        rt_value = ctx->Registers[rt_index];
                    }else if(!rt_load){
                        need_wait = true;
                    }

                    bool stall = need_wait;
                    if(!need_wait){

                        if(static_cast<int32_t>(rs_value) == static_cast<int32_t>(rt_value)){
                            ctx->PcFlush = Context::PC_FLUSH_CONSUMER_COUNT;

                            auto imm = IInstr::GetImm(self->instruction->GetBitsInstruction());
                            reg_t offset = static_cast<reg_t>(WORD_WIDTH + (signExtend16(imm) << 2));
                            //TODO: Error handling
                            ctx->SetPC(ctx->GetPC() + offset);
                        }

                        //Push next task
                        stall = !ctx->pushTask(ctx->ID_EX, self);
                    }

                    return (stall)? Error::PIPELINE_STALL : Error::NONE;
                })
                .EX(STAGE_TASK(){

                    RISING_EDGE_FENCE();

                    self->context->pushTask(self->context->EX_DM, self);

                    return Error::NONE;
                })
                .DM(STAGE_TASK(){

                    RISING_EDGE_FENCE();

                    self->context->pushTask(self->context->DM_WB, self);

                    return Error::NONE;
                })
                .WB(STAGE_TASK(){

                    RISING_EDGE_FENCE();

                    return Error::NONE;
                });

        TasksTable[OP_BNE].Name("BNE", OP_BNE)
                .IF(ResolveRegsIF)
                .ID(STAGE_TASK(){
                    auto* ctx = self->context;
                    auto& reg_reserves = ctx->RegReserves;

                    auto rt_index = self->RtIndex;
                    auto rs_index = self->RsIndex;

                    bool rs_load = false, rt_load = false;
                    reg_t rs_value = 0, rt_value = 0;

                    // Load ID forwarding stuff here
                    // To avoiding race condition on falling edge
                    if(reg_reserves[rs_index].Holder != nullptr &&
                       reg_reserves[rs_index].IDAvailable){
                        rs_value = reg_reserves[rs_index].Value;
                        rs_load = true;
                        reg_reserves[rs_index].IDForward = true;
                    }
                    if(reg_reserves[rt_index].Holder != nullptr &&
                       reg_reserves[rt_index].IDAvailable){
                        rt_value = reg_reserves[rt_index].Value;
                        rt_load = true;
                        reg_reserves[rt_index].IDForward = true;
                    }

                    RISING_EDGE_FENCE();

                    bool need_wait = false;
                    if(reg_reserves[rs_index].Holder == nullptr){
                        rs_value = ctx->Registers[rs_index];
                    }else if(!rs_load){
                        need_wait = true;
                    }

                    if(reg_reserves[rt_index].Holder == nullptr){
                        rt_value = ctx->Registers[rt_index];
                    }else if(!rt_load){
                        need_wait = true;
                    }

                    bool stall = need_wait;
                    if(!need_wait){

                        if(static_cast<int32_t>(rs_value) != static_cast<int32_t>(rt_value)){
                            ctx->PcFlush = Context::PC_FLUSH_CONSUMER_COUNT;

                            auto imm = IInstr::GetImm(self->instruction->GetBitsInstruction());
                            reg_t offset = static_cast<reg_t>(WORD_WIDTH + (signExtend16(imm) << 2));
                            //TODO: Error handling
                            ctx->SetPC(ctx->GetPC() + offset);
                        }

                        //Push next task
                        stall = !ctx->pushTask(ctx->ID_EX, self);
                    }

                    return (stall)? Error::PIPELINE_STALL : Error::NONE;
                })
                .EX(STAGE_TASK(){

                    RISING_EDGE_FENCE();

                    self->context->pushTask(self->context->EX_DM, self);

                    return Error::NONE;
                })
                .DM(STAGE_TASK(){

                    RISING_EDGE_FENCE();

                    self->context->pushTask(self->context->DM_WB, self);

                    return Error::NONE;
                })
                .WB(STAGE_TASK(){

                    RISING_EDGE_FENCE();

                    return Error::NONE;
                });

        TasksTable[OP_BGTZ].Name("BGTZ", OP_BGTZ)
                .IF(STAGE_TASK(){
                    /*
                     * This IF lambda is always executed in last half cycle
                     * Since first half cycle is reserved for instruction loading
                     * */
                    RISING_EDGE_FENCE();

                    //Decode registers index
                    auto instr_bits = self->instruction->GetBitsInstruction();
                    self->RsIndex = IInstr::GetRs(instr_bits);

                    auto* ctx = self->context;
                    return (ctx->pushTask(ctx->IF_ID, self))? Error::NONE : Error::PIPELINE_STALL;
                })
                .ID(STAGE_TASK(){
                    auto* ctx = self->context;
                    auto& reg_reserves = ctx->RegReserves;

                    auto rs_index = self->RsIndex;

                    bool rs_load = false;
                    reg_t rs_value = 0;

                    // Load ID forwarding stuff here
                    // To avoiding race condition on falling edge
                    if(reg_reserves[rs_index].Holder != nullptr &&
                       reg_reserves[rs_index].IDAvailable){
                        rs_value = reg_reserves[rs_index].Value;
                        rs_load = true;
                        reg_reserves[rs_index].IDForward = true;
                    }

                    RISING_EDGE_FENCE();

                    bool need_wait = false;
                    if(reg_reserves[rs_index].Holder == nullptr){
                        rs_value = ctx->Registers[rs_index];
                    }else if(!rs_load){
                        need_wait = true;
                    }

                    bool stall = need_wait;
                    if(!need_wait){

                        if(static_cast<int32_t>(rs_value) > 0){
                            ctx->PcFlush = Context::PC_FLUSH_CONSUMER_COUNT;

                            auto imm = IInstr::GetImm(self->instruction->GetBitsInstruction());
                            reg_t offset = static_cast<reg_t>(WORD_WIDTH + (signExtend16(imm) << 2));
                            //TODO: Error handling
                            ctx->SetPC(ctx->GetPC() + offset);
                        }

                        //Push next task
                        stall = !ctx->pushTask(ctx->ID_EX, self);
                    }

                    return (stall)? Error::PIPELINE_STALL : Error::NONE;
                })
                .EX(STAGE_TASK(){

                    RISING_EDGE_FENCE();

                    self->context->pushTask(self->context->EX_DM, self);

                    return Error::NONE;
                })
                .DM(STAGE_TASK(){

                    RISING_EDGE_FENCE();

                    self->context->pushTask(self->context->DM_WB, self);

                    return Error::NONE;
                })
                .WB(STAGE_TASK(){

                    RISING_EDGE_FENCE();

                    return Error::NONE;
                });
    }
}

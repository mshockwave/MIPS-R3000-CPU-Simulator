#include "RTasks.h"

namespace task{
    void InitRTasks(){

        TasksTable[OP_ADD].Name("add")
                .IF(STAGE_TASK(){

                    /*
                     * This IF lambda is always executed in last half cycle
                     * Since first half cycle is reserved for instruction loading
                     * */
                    RISING_EDGE_FENCE();

                    //Decode registers index
                    auto instr_bits = self->instruction->GetBitsInstruction();
                    self->RdIndex = RInstr::GetRd(instr_bits);
                    self->RtIndex = RInstr::GetRt(instr_bits);
                    self->RsIndex = RInstr::GetRs(instr_bits);

                    auto* ctx = self->context;
                    return (ctx->pushTask(ctx->IF_ID, self))? Error::NONE : Error::PIPLINE_STALL;
                })
                .ID(STAGE_TASK(){

                    auto* ctx = self->context;
                    auto& reg_reserves = ctx->RegReserves;

                    // Do not load registers
                    // In first half cycle
                    RISING_EDGE_FENCE();

                    auto rt_index = self->RtIndex;
                    auto rs_index = self->RsIndex;

                    bool need_wait = false;
                    if(reg_reserves[rt_index].load() == nullptr){

                        //Load from main registers
                        self->RtValue = ctx->Registers[rt_index];
                    }else if(ctx->FWD_ID_EXE.Available.load() &&
                            ctx->FWD_ID_EXE.RegId == rt_index){

                        //Load from forward storage
                        self->RtValue = ctx->FWD_ID_EXE.RegValue;
                    }else{
                        need_wait = true;
                    }

                    if(reg_reserves[rs_index].load() == nullptr){

                        //Load from main registers
                        self->RtValue = ctx->Registers[rs_index];
                    }else if(ctx->FWD_ID_EXE.Available.load() &&
                             ctx->FWD_ID_EXE.RegId == rs_index){

                        //Load from forward storage
                        self->RtValue = ctx->FWD_ID_EXE.RegValue;
                    }else{
                        need_wait = true;
                    }

                    //Clean forwarding storage
                    ctx->FWD_ID_EXE.Available.store(false);

                    bool stall = false;
                    if(!need_wait){
                        //Push next task
                        stall = !ctx->pushTask(ctx->ID_EX, self);

                        //Reserve destination registers
                        ctx->RegReserves[self->RdIndex] = self;
                    }

                    return (stall)? Error::PIPLINE_STALL : Error::NONE;
                })
                .EX(STAGE_TASK(){

                    auto* ctx = self->context;

                    //TODO: Error detection
                    auto rt_value = self->RtValue;
                    auto rs_value = self->RsValue;

                    auto rd_value = rt_value + rs_value;

                    RISING_EDGE_FENCE();

                    self->RdValue = rd_value;

                    return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPLINE_STALL;
                })
                .DM(STAGE_TASK(){

                    auto* ctx = self->context;

                    //Write to forwarding storage
                    ctx->FWD_ID_EXE.RegId = self->RdIndex;
                    ctx->FWD_ID_EXE.RegValue = self->RdValue;
                    ctx->FWD_ID_EXE.Available.store(true);

                    RISING_EDGE_FENCE();

                    return (ctx->pushTask(ctx->DM_WB, self))? Error::NONE : Error::PIPLINE_STALL;
                })
                .WB(STAGE_TASK(){

                    auto* ctx = self->context;

                    //Write back to main registers
                    ctx->Registers[self->RdIndex] = self->RdValue;
                    //Clean destination register reservation
                    if(ctx->RegReserves[self->RdIndex].load() == self){
                        ctx->RegReserves[self->RdIndex] = nullptr;
                    }

                    RISING_EDGE_FENCE();

                    return Error::NONE;
                });
    }
}

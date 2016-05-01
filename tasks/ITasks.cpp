#include "ITasks.h"

namespace task{
    namespace IInstr{
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
        
        /*
         * Load rs value
         * And detect stalling
         * For non-branch operations
         * */
        TaskHandle::stage_task_t LoadRsRegID = STAGE_TASK(){
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            // Do not load registers
            // In first half cycle
            RISING_EDGE_FENCE();
            
            auto rt_index = self->RtIndex;
            auto rs_index = self->RsIndex;
            
            bool need_wait = false;
            
            if(reg_reserves[rs_index].Holder == nullptr){
                
                //Load from main registers
                self->RsValue = ctx->Registers[rs_index];
                
            }else if(reg_reserves[rs_index].EXAvailable){
                
                //Can EX Forward
                reg_reserves[rs_index].EXForward = true;
                
            }else{
                need_wait = true;
            }
            
            bool stall = need_wait;
            if(!need_wait){
                //Push next task
                stall = !ctx->pushTask(ctx->ID_EX, self);
                
                //Reserve destination registers
                ctx->RegReserves[rt_index].Reset(self);
            }
            
            return (stall)? Error::PIPELINE_STALL : Error::NONE;
        };
        
        /*
         * Load rs, rt value
         * And detect stalling
         * For non-branch operations
         * */
        TaskHandle::stage_task_t LoadRegsID = STAGE_TASK(){
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            // Do not load registers
            // In first half cycle
            RISING_EDGE_FENCE();
            
            auto rt_index = self->RtIndex;
            auto rs_index = self->RsIndex;
            
            bool need_wait = false;
            if(reg_reserves[rt_index].Holder == nullptr){
                
                //Load from main registers
                self->RtValue = ctx->Registers[rt_index];
                
            }else if(reg_reserves[rt_index].EXAvailable){
                
                //Can EX Forward
                reg_reserves[rt_index].EXForward = true;
                
            }else{
                need_wait = true;
            }
            
            if(reg_reserves[rs_index].Holder == nullptr){
                
                //Load from main registers
                self->RsValue = ctx->Registers[rs_index];
                
            }else if(reg_reserves[rs_index].EXAvailable){
                
                //Can EX Forward
                reg_reserves[rs_index].EXForward = true;
                
            }else{
                need_wait = true;
            }
            
            bool stall = need_wait;
            if(!need_wait){
                //Push next task
                stall = !ctx->pushTask(ctx->ID_EX, self);
            }
            
            return (stall)? Error::PIPELINE_STALL : Error::NONE;
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
        
        /*
         * Caculate offset: rs + imm(signed)
         * And store in self->RsValue
         */
        TaskHandle::stage_task_t CaculateMemOffsetEX = STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            auto rs_value = self->RsValue;
            if(reg_reserves[self->RsIndex].EXForward){
                rs_value = reg_reserves[self->RsIndex].Value;
            }
            auto imm = IInstr::GetImm(self->instruction->GetBitsInstruction());
            
            rs_value += static_cast<int32_t>(signExtend16(imm));
            
            RISING_EDGE_FENCE();
            
            self->RsValue = rs_value;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        };
        
        /*
         * Write rt back to main registers
         * */
        TaskHandle::stage_task_t WriteRegsWB = STAGE_TASK() {
            auto* ctx = self->context;
            
            //Write back to main registers
            //Check whether write to $0
            Error err = Error::NONE;
            if(self->RtIndex == 0){
                err = Error::WRITE_REG_ZERO;
            }else{
                ctx->Registers[self->RtIndex] = self->RtValue;
                self->ModifyRegIndex = self->RtIndex;
            }
            
            //Clean destination register reservation
            if(ctx->RegReserves[self->RtIndex].Holder == self){
                ctx->RegReserves[self->RtIndex].Reset(nullptr);
            }
            
            RISING_EDGE_FENCE();
            
            return err;
        };
        
    }; //namespace IInstr
    
    void InitITasks(){
        
        TasksTable[OP_ADDI].Name("ADDI", OP_ADDI)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRsRegID)
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            //TODO: Error detection
            auto rs_value = self->RsValue;
            if(reg_reserves[self->RsIndex].EXForward){
                rs_value = reg_reserves[self->RsIndex].Value;
            }
            auto imm = IInstr::GetImm(self->instruction->GetBitsInstruction());
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RtIndex].EXAvailable = true;
            
            auto rt_value = rs_value + static_cast<int32_t>(signExtend16(imm));
            
            RISING_EDGE_FENCE();
            
            self->RtValue = rt_value;
            
            reg_reserves[self->RtIndex].Value = rt_value;
            reg_reserves[self->RtIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(IInstr::EmptyDM)
        .WB(IInstr::WriteRegsWB);
        
        TasksTable[OP_ADDIU].Name("ADDIU", OP_ADDIU)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRsRegID)
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            //No overflow exception
            auto rs_value = self->RsValue;
            if(reg_reserves[self->RsIndex].EXForward){
                rs_value = reg_reserves[self->RsIndex].Value;
            }
            auto imm = IInstr::GetImm(self->instruction->GetBitsInstruction());
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RtIndex].EXAvailable = true;
            
            auto rt_value = rs_value + static_cast<reg_t>(imm);
            
            RISING_EDGE_FENCE();
            
            self->RtValue = rt_value;
            
            reg_reserves[self->RtIndex].Value = rt_value;
            reg_reserves[self->RtIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(IInstr::EmptyDM)
        .WB(IInstr::WriteRegsWB);
        
        TasksTable[OP_LW].Name("LW", OP_LW)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRsRegID)
        .EX(IInstr::CaculateMemOffsetEX)
        .DM(STAGE_TASK(){
            auto* ctx = self->context;
            
            //Do not read memory on the first cycle
            RISING_EDGE_FENCE();
            
            Error err = Error::NONE;
            try{
                word_t v = ctx->GetMemoryWord(self->RsValue);
                self->RtValue = v;
            }catch(const Error& e){
                err = e;
            }
            
            return (ctx->pushTask(ctx->DM_WB, self))? err : Error::PIPELINE_STALL;
        })
        .WB(IInstr::WriteRegsWB);
        
        TasksTable[OP_LH].Name("LH", OP_LH)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRsRegID)
        .EX(IInstr::CaculateMemOffsetEX)
        .DM(STAGE_TASK(){
            auto* ctx = self->context;
            
            //Do not read memory on the first cycle
            RISING_EDGE_FENCE();
            
            Error err = Error::NONE;
            try{
                half_w_t v = ctx->GetMemoryHalfWord(self->RsValue);
                self->RtValue = signExtend16(v);
            }catch(const Error& e){
                err = e;
            }
            
            return (ctx->pushTask(ctx->DM_WB, self))? err : Error::PIPELINE_STALL;
        })
        .WB(IInstr::WriteRegsWB);
        
        TasksTable[OP_LHU].Name("LHU", OP_LHU)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRsRegID)
        .EX(IInstr::CaculateMemOffsetEX)
        .DM(STAGE_TASK(){
            auto* ctx = self->context;
            
            //Do not read memory on the first cycle
            RISING_EDGE_FENCE();
            
            Error err = Error::NONE;
            try{
                half_w_t v = ctx->GetMemoryHalfWord(self->RsValue);
                self->RtValue = static_cast<reg_t>(v);
            }catch(const Error& e){
                err = e;
            }
            
            return (ctx->pushTask(ctx->DM_WB, self))? err : Error::PIPELINE_STALL;
        })
        .WB(IInstr::WriteRegsWB);
        
        TasksTable[OP_LB].Name("LB", OP_LB)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRsRegID)
        .EX(IInstr::CaculateMemOffsetEX)
        .DM(STAGE_TASK(){
            auto* ctx = self->context;
            
            //Do not read memory on the first cycle
            RISING_EDGE_FENCE();
            
            Error err = Error::NONE;
            try{
                byte_t v = ctx->GetMemoryByte(self->RsValue);
                self->RtValue = signExtend8(v);
            }catch(const Error& e){
                err = e;
            }
            
            return (ctx->pushTask(ctx->DM_WB, self))? err : Error::PIPELINE_STALL;
        })
        .WB(IInstr::WriteRegsWB);
        
        TasksTable[OP_LBU].Name("LBU", OP_LBU)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRsRegID)
        .EX(IInstr::CaculateMemOffsetEX)
        .DM(STAGE_TASK(){
            auto* ctx = self->context;
            
            //Do not read memory on the first cycle
            RISING_EDGE_FENCE();
            
            Error err = Error::NONE;
            try{
                byte_t v = ctx->GetMemoryByte(self->RsValue);
                self->RtValue = static_cast<reg_t>(v);
            }catch(const Error& e){
                err = e;
            }
            
            return (ctx->pushTask(ctx->DM_WB, self))? err : Error::PIPELINE_STALL;
        })
        .WB(IInstr::WriteRegsWB);
        
        TasksTable[OP_SW].Name("SW", OP_SW)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRegsID)
        .EX(IInstr::CaculateMemOffsetEX)
        .DM(STAGE_TASK(){
            auto* ctx = self->context;
            
            //Store memory on the first cycle
            Error err = Error::NONE;
            try{
                auto& v = ctx->GetMemoryWord(self->RsValue);
                v = self->RtValue;
            }catch(const Error& e){
                err = e;
            }
            
            RISING_EDGE_FENCE();
            
            return (ctx->pushTask(ctx->DM_WB, self))? err : Error::PIPELINE_STALL;
        })
        .WB(IInstr::EmptyWB);
        
        TasksTable[OP_SH].Name("SH", OP_SH)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRegsID)
        .EX(IInstr::CaculateMemOffsetEX)
        .DM(STAGE_TASK(){
            auto* ctx = self->context;
            
            //Store memory on the first cycle
            Error err = Error::NONE;
            try{
                auto& v = ctx->GetMemoryHalfWord(self->RsValue);
                v = static_cast<half_w_t>(self->RtValue & 0x0000FFFF);
            }catch(const Error& e){
                err = e;
            }
            
            RISING_EDGE_FENCE();
            
            return (ctx->pushTask(ctx->DM_WB, self))? err : Error::PIPELINE_STALL;
        })
        .WB(IInstr::EmptyWB);
        
        TasksTable[OP_SB].Name("SB", OP_SB)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRegsID)
        .EX(IInstr::CaculateMemOffsetEX)
        .DM(STAGE_TASK(){
            auto* ctx = self->context;
            
            //Store memory on the first cycle
            Error err = Error::NONE;
            try{
                auto& v = ctx->GetMemoryByte(self->RsValue);
                v = static_cast<byte_t>(self->RtValue & 0x000000FF);
            }catch(const Error& e){
                err = e;
            }
            
            RISING_EDGE_FENCE();
            
            return (ctx->pushTask(ctx->DM_WB, self))? err : Error::PIPELINE_STALL;
        })
        .WB(IInstr::EmptyWB);
        
        TasksTable[OP_LUI].Name("LUI", OP_LUI)
        .IF(STAGE_TASK(){
            /*
             * This IF lambda is always executed in last half cycle
             * Since first half cycle is reserved for instruction loading
             * */
            RISING_EDGE_FENCE();
            
            //Decode registers index
            auto instr_bits = self->instruction->GetBitsInstruction();
            self->RtIndex = IInstr::GetRt(instr_bits);
            
            auto* ctx = self->context;
            return (ctx->pushTask(ctx->IF_ID, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .ID(STAGE_TASK(){
            
            RISING_EDGE_FENCE();
            
            auto* ctx = self->context;
            //Reserve destination registers
            ctx->RegReserves[self->RtIndex].Reset(self);
            
            return (ctx->pushTask(ctx->ID_EX, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .EX(STAGE_TASK(){
            auto* ctx = self->context;
            
            RISING_EDGE_FENCE();
            auto imm = IInstr::GetImm(self->instruction->GetBitsInstruction());
            self->RtValue = static_cast<reg_t>(imm << 16);
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(IInstr::EmptyDM)
        .WB(IInstr::WriteRegsWB);
        
        TasksTable[OP_ANDI].Name("ANDI", OP_ANDI)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRsRegID)
        .EX(STAGE_TASK(){
            auto* ctx = self->context;
            
            RISING_EDGE_FENCE();
            auto imm = IInstr::GetImm(self->instruction->GetBitsInstruction());
            self->RtValue = (self->RsValue & static_cast<reg_t>(imm));
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(IInstr::EmptyDM)
        .WB(IInstr::WriteRegsWB);
        
        TasksTable[OP_ORI].Name("ORI", OP_ORI)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRsRegID)
        .EX(STAGE_TASK(){
            auto* ctx = self->context;
            
            RISING_EDGE_FENCE();
            auto imm = IInstr::GetImm(self->instruction->GetBitsInstruction());
            self->RtValue = (self->RsValue | static_cast<reg_t>(imm));
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(IInstr::EmptyDM)
        .WB(IInstr::WriteRegsWB);
        
        TasksTable[OP_NORI].Name("NORI", OP_NORI)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRsRegID)
        .EX(STAGE_TASK(){
            auto* ctx = self->context;
            
            RISING_EDGE_FENCE();
            auto imm = IInstr::GetImm(self->instruction->GetBitsInstruction());
            self->RtValue = ~(self->RsValue | static_cast<reg_t>(imm));
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(IInstr::EmptyDM)
        .WB(IInstr::WriteRegsWB);
        
        TasksTable[OP_SLTI].Name("SLTI", OP_SLTI)
        .IF(IInstr::ResolveRegsIF)
        .ID(IInstr::LoadRsRegID)
        .EX(STAGE_TASK(){
            auto* ctx = self->context;
            
            RISING_EDGE_FENCE();
            auto imm = IInstr::GetImm(self->instruction->GetBitsInstruction());
            int32_t s_rs = static_cast<int32_t>(self->RsValue);
            int32_t s_imm = static_cast<int32_t>(signExtend16(imm));
            self->RtValue = (s_rs < s_imm)? U32_1 : U32_0;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(IInstr::EmptyDM)
        .WB(IInstr::WriteRegsWB);

        TasksTable[OP_BEQ].Name("BEQ", OP_BEQ)
                .IF(IInstr::ResolveRegsIF)
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
                .WB(IInstr::EmptyWB);

        TasksTable[OP_BNE].Name("BNE", OP_BNE)
                .IF(IInstr::ResolveRegsIF)
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
                .WB(IInstr::EmptyWB);

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
                .WB(IInstr::EmptyWB);
    }
}

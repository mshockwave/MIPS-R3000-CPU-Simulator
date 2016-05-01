#include "RTasks.h"

namespace task{

    namespace RInstr{

        /*
         * Load rd, rs, rt index
         * */
        TaskHandle::stage_task_t ResolveRegsIF = STAGE_TASK(){
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
            return (ctx->pushTask(ctx->IF_ID, self))? Error::NONE : Error::PIPELINE_STALL;
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

                //Reserve destination registers
                //ctx->RegReserves[self->RdIndex] = self;
                ctx->RegReserves[self->RdIndex].Reset(self);
            }

            return (stall)? Error::PIPELINE_STALL : Error::NONE;
        };

        TaskHandle::stage_task_t EmptyDM = STAGE_TASK(){
            auto* ctx = self->context;

            RISING_EDGE_FENCE();

            return (ctx->pushTask(ctx->DM_WB, self))? Error::NONE : Error::PIPELINE_STALL;
        };

        /*
         * Write rd back to main registers
         * */
        TaskHandle::stage_task_t WriteRegsWB = STAGE_TASK() {
            auto* ctx = self->context;

            //Write back to main registers
            //Check whether write to $0
            Error err = Error::NONE;
            if(self->RdIndex == 0){
                err = Error::WRITE_REG_ZERO;
            }else{
                ctx->Registers[self->RdIndex] = self->RdValue;
                self->ModifyRegIndex = self->RdIndex;
            }

            //Clean destination register reservation
            if(ctx->RegReserves[self->RdIndex].Holder == self){
                ctx->RegReserves[self->RdIndex].Reset(nullptr);
            }

            RISING_EDGE_FENCE();

            return err;
        };

    }//namespace RInstr

    void InitRTasks(){

        TasksTable[OP_ADD].Name("ADD", OP_ADD)
                .IF(RInstr::ResolveRegsIF)
                .ID(RInstr::LoadRegsID)
                .EX(STAGE_TASK(){

                    auto* ctx = self->context;
                    auto& reg_reserves = ctx->RegReserves;

                    //TODO: Error detection
                    auto rt_value = self->RtValue;
                    auto rs_value = self->RsValue;
                    if(reg_reserves[self->RtIndex].EXForward){
                        rt_value = reg_reserves[self->RtIndex].Value;
                    }
                    if(reg_reserves[self->RsIndex].EXForward){
                        rs_value = reg_reserves[self->RsIndex].Value;
                    }

                    // Prepare forwarding info
                    // For result of this stage
                    reg_reserves[self->RdIndex].EXAvailable = true;

                    Error err = Error::NONE;
                    
                    auto rd_value = rt_value + rs_value;
                    auto overflow = isSumOverflow(rt_value, rs_value, rd_value);
                    if(overflow) err = err + Error::NUMBER_OVERFLOW;

                    RISING_EDGE_FENCE();

                    self->RdValue = rd_value;

                    reg_reserves[self->RdIndex].Value = rd_value;
                    reg_reserves[self->RdIndex].IDAvailable = true;

                    return (ctx->pushTask(ctx->EX_DM, self))? err : Error::PIPELINE_STALL;
                })
                .DM(RInstr::EmptyDM)
                .WB(RInstr::WriteRegsWB);
        
        TasksTable[OP_ADDU].Name("ADDU", OP_ADDU)
        .IF(RInstr::ResolveRegsIF)
        .ID(RInstr::LoadRegsID)
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            //TODO: Error detection
            auto rt_value = self->RtValue;
            auto rs_value = self->RsValue;
            if(reg_reserves[self->RtIndex].EXForward){
                rt_value = reg_reserves[self->RtIndex].Value;
            }
            if(reg_reserves[self->RsIndex].EXForward){
                rs_value = reg_reserves[self->RsIndex].Value;
            }
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RdIndex].EXAvailable = true;
            
            auto rd_value = rt_value + rs_value;
            
            RISING_EDGE_FENCE();
            
            self->RdValue = rd_value;
            
            reg_reserves[self->RdIndex].Value = rd_value;
            reg_reserves[self->RdIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(RInstr::EmptyDM)
        .WB(RInstr::WriteRegsWB);

        TasksTable[OP_SUB].Name("SUB", OP_SUB)
                .IF(RInstr::ResolveRegsIF)
                .ID(RInstr::LoadRegsID)
                .EX(STAGE_TASK(){

                    auto* ctx = self->context;
                    auto& reg_reserves = ctx->RegReserves;

                    //TODO: Error detection
                    auto rt_value = self->RtValue;
                    auto rs_value = self->RsValue;
                    if(reg_reserves[self->RtIndex].EXForward){
                        rt_value = reg_reserves[self->RtIndex].Value;
                    }
                    if(reg_reserves[self->RsIndex].EXForward){
                        rs_value = reg_reserves[self->RsIndex].Value;
                    }

                    // Prepare forwarding info
                    // For result of this stage
                    reg_reserves[self->RdIndex].EXAvailable = true;

                    Error err = Error::NONE;
                    
                    auto rd_value = rs_value - rt_value;
                    bool overflow = isSumOverflow(rs_value, twoComplement(rt_value), rd_value);
                    if(overflow) err = err + Error::NUMBER_OVERFLOW;

                    RISING_EDGE_FENCE();

                    self->RdValue = rd_value;

                    reg_reserves[self->RdIndex].IDAvailable = true;

                    return (ctx->pushTask(ctx->EX_DM, self))? err : Error::PIPELINE_STALL;
                })
                .DM(RInstr::EmptyDM)
                .WB(RInstr::WriteRegsWB);
        
        TasksTable[OP_AND].Name("AND", OP_AND)
        .IF(RInstr::ResolveRegsIF)
        .ID(RInstr::LoadRegsID)
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            //TODO: Error detection
            auto rt_value = self->RtValue;
            auto rs_value = self->RsValue;
            if(reg_reserves[self->RtIndex].EXForward){
                rt_value = reg_reserves[self->RtIndex].Value;
            }
            if(reg_reserves[self->RsIndex].EXForward){
                rs_value = reg_reserves[self->RsIndex].Value;
            }
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RdIndex].EXAvailable = true;
            
            auto rd_value = rt_value & rs_value;
            
            RISING_EDGE_FENCE();
            
            self->RdValue = rd_value;
            
            reg_reserves[self->RdIndex].Value = rd_value;
            reg_reserves[self->RdIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(RInstr::EmptyDM)
        .WB(RInstr::WriteRegsWB);
        
        TasksTable[OP_OR].Name("OR", OP_OR)
        .IF(RInstr::ResolveRegsIF)
        .ID(RInstr::LoadRegsID)
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            //TODO: Error detection
            auto rt_value = self->RtValue;
            auto rs_value = self->RsValue;
            if(reg_reserves[self->RtIndex].EXForward){
                rt_value = reg_reserves[self->RtIndex].Value;
            }
            if(reg_reserves[self->RsIndex].EXForward){
                rs_value = reg_reserves[self->RsIndex].Value;
            }
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RdIndex].EXAvailable = true;
            
            auto rd_value = rt_value | rs_value;
            
            RISING_EDGE_FENCE();
            
            self->RdValue = rd_value;
            
            reg_reserves[self->RdIndex].Value = rd_value;
            reg_reserves[self->RdIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(RInstr::EmptyDM)
        .WB(RInstr::WriteRegsWB);
        
        TasksTable[OP_XOR].Name("XOR", OP_XOR)
        .IF(RInstr::ResolveRegsIF)
        .ID(RInstr::LoadRegsID)
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            //TODO: Error detection
            auto rt_value = self->RtValue;
            auto rs_value = self->RsValue;
            if(reg_reserves[self->RtIndex].EXForward){
                rt_value = reg_reserves[self->RtIndex].Value;
            }
            if(reg_reserves[self->RsIndex].EXForward){
                rs_value = reg_reserves[self->RsIndex].Value;
            }
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RdIndex].EXAvailable = true;
            
            auto rd_value = rt_value ^ rs_value;
            
            RISING_EDGE_FENCE();
            
            self->RdValue = rd_value;
            
            reg_reserves[self->RdIndex].Value = rd_value;
            reg_reserves[self->RdIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(RInstr::EmptyDM)
        .WB(RInstr::WriteRegsWB);
        
        TasksTable[OP_NOR].Name("NOR", OP_NOR)
        .IF(RInstr::ResolveRegsIF)
        .ID(RInstr::LoadRegsID)
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            //TODO: Error detection
            auto rt_value = self->RtValue;
            auto rs_value = self->RsValue;
            if(reg_reserves[self->RtIndex].EXForward){
                rt_value = reg_reserves[self->RtIndex].Value;
            }
            if(reg_reserves[self->RsIndex].EXForward){
                rs_value = reg_reserves[self->RsIndex].Value;
            }
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RdIndex].EXAvailable = true;
            
            auto rd_value = ~(rt_value | rs_value);
            
            RISING_EDGE_FENCE();
            
            self->RdValue = rd_value;
            
            reg_reserves[self->RdIndex].Value = rd_value;
            reg_reserves[self->RdIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(RInstr::EmptyDM)
        .WB(RInstr::WriteRegsWB);
        
        TasksTable[OP_NAND].Name("NAND", OP_NAND)
        .IF(RInstr::ResolveRegsIF)
        .ID(RInstr::LoadRegsID)
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            //TODO: Error detection
            auto rt_value = self->RtValue;
            auto rs_value = self->RsValue;
            if(reg_reserves[self->RtIndex].EXForward){
                rt_value = reg_reserves[self->RtIndex].Value;
            }
            if(reg_reserves[self->RsIndex].EXForward){
                rs_value = reg_reserves[self->RsIndex].Value;
            }
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RdIndex].EXAvailable = true;
            
            auto rd_value = ~(rt_value & rs_value);
            
            RISING_EDGE_FENCE();
            
            self->RdValue = rd_value;
            
            reg_reserves[self->RdIndex].Value = rd_value;
            reg_reserves[self->RdIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(RInstr::EmptyDM)
        .WB(RInstr::WriteRegsWB);
        
        TasksTable[OP_SLT].Name("SLT", OP_SLT)
        .IF(RInstr::ResolveRegsIF)
        .ID(RInstr::LoadRegsID)
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            //TODO: Error detection
            auto rt_value = self->RtValue;
            auto rs_value = self->RsValue;
            if(reg_reserves[self->RtIndex].EXForward){
                rt_value = reg_reserves[self->RtIndex].Value;
            }
            if(reg_reserves[self->RsIndex].EXForward){
                rs_value = reg_reserves[self->RsIndex].Value;
            }
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RdIndex].EXAvailable = true;
            
            int32_t s_rt = static_cast<int32_t>(rt_value);
            int32_t s_rs = static_cast<int32_t>(rs_value);
            auto rd_value = (s_rs < s_rt)? U32_1 : U32_0;
            
            RISING_EDGE_FENCE();
            
            self->RdValue = rd_value;
            
            reg_reserves[self->RdIndex].Value = rd_value;
            reg_reserves[self->RdIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(RInstr::EmptyDM)
        .WB(RInstr::WriteRegsWB);
        
        TasksTable[OP_SLL].Name("SLL", OP_SLL)
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
            
            auto* ctx = self->context;
            return (ctx->pushTask(ctx->IF_ID, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .ID(STAGE_TASK(){
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            // Do not load registers
            // In first half cycle
            RISING_EDGE_FENCE();
            
            auto rt_index = self->RtIndex;
            
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
            
            bool stall = need_wait;
            if(!need_wait){
                //Push next task
                stall = !ctx->pushTask(ctx->ID_EX, self);
                
                //Reserve destination registers
                //ctx->RegReserves[self->RdIndex] = self;
                ctx->RegReserves[self->RdIndex].Reset(self);
            }
            
            return (stall)? Error::PIPELINE_STALL : Error::NONE;
        })
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            auto rt_value = self->RtValue;
            if(reg_reserves[self->RtIndex].EXForward){
                rt_value = reg_reserves[self->RtIndex].Value;
            }
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RdIndex].EXAvailable = true;
            
            uint8_t sh_amt = RInstr::GetShAmt(self->instruction->GetBitsInstruction());
            auto rd_value = (rt_value << sh_amt);
            
            RISING_EDGE_FENCE();
            
            self->RdValue = rd_value;
            
            reg_reserves[self->RdIndex].Value = rd_value;
            reg_reserves[self->RdIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(RInstr::EmptyDM)
        .WB(RInstr::WriteRegsWB);
        
        TasksTable[OP_SRL].Name("SRL", OP_SRL)
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
            
            auto* ctx = self->context;
            return (ctx->pushTask(ctx->IF_ID, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .ID(STAGE_TASK(){
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            // Do not load registers
            // In first half cycle
            RISING_EDGE_FENCE();
            
            auto rt_index = self->RtIndex;
            
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
            
            bool stall = need_wait;
            if(!need_wait){
                //Push next task
                stall = !ctx->pushTask(ctx->ID_EX, self);
                
                //Reserve destination registers
                //ctx->RegReserves[self->RdIndex] = self;
                ctx->RegReserves[self->RdIndex].Reset(self);
            }
            
            return (stall)? Error::PIPELINE_STALL : Error::NONE;
        })
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            reg_t rt_value = self->RtValue;
            if(reg_reserves[self->RtIndex].EXForward){
                rt_value = reg_reserves[self->RtIndex].Value;
            }
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RdIndex].EXAvailable = true;
            
            uint8_t sh_amt = RInstr::GetShAmt(self->instruction->GetBitsInstruction());
            reg_t rd_value = (rt_value >> sh_amt);
            
            RISING_EDGE_FENCE();
            
            self->RdValue = rd_value;
            
            reg_reserves[self->RdIndex].Value = rd_value;
            reg_reserves[self->RdIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(RInstr::EmptyDM)
        .WB(RInstr::WriteRegsWB);
        
        TasksTable[OP_SRA].Name("SRA", OP_SRA)
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
            
            auto* ctx = self->context;
            return (ctx->pushTask(ctx->IF_ID, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .ID(STAGE_TASK(){
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            // Do not load registers
            // In first half cycle
            RISING_EDGE_FENCE();
            
            auto rt_index = self->RtIndex;
            
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
            
            bool stall = need_wait;
            if(!need_wait){
                //Push next task
                stall = !ctx->pushTask(ctx->ID_EX, self);
                
                //Reserve destination registers
                //ctx->RegReserves[self->RdIndex] = self;
                ctx->RegReserves[self->RdIndex].Reset(self);
            }
            
            return (stall)? Error::PIPELINE_STALL : Error::NONE;
        })
        .EX(STAGE_TASK(){
            
            auto* ctx = self->context;
            auto& reg_reserves = ctx->RegReserves;
            
            reg_t rt_value = self->RtValue;
            if(reg_reserves[self->RtIndex].EXForward){
                rt_value = reg_reserves[self->RtIndex].Value;
            }
            
            // Prepare forwarding info
            // For result of this stage
            reg_reserves[self->RdIndex].EXAvailable = true;
            
            uint8_t sh_amt = RInstr::GetShAmt(self->instruction->GetBitsInstruction());
            auto rd_value = (static_cast<int32_t>(rt_value) >> sh_amt);
            
            RISING_EDGE_FENCE();
            
            self->RdValue = static_cast<reg_t>(rd_value);
            
            reg_reserves[self->RdIndex].Value = rd_value;
            reg_reserves[self->RdIndex].IDAvailable = true;
            
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(RInstr::EmptyDM)
        .WB(RInstr::WriteRegsWB);
        
        TasksTable[OP_JR].Name("JR", OP_JR)
        .IF(RInstr::ResolveRegsIF)
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
                
                ctx->PcFlush = Context::PC_FLUSH_CONSUMER_COUNT;
                ctx->SetPC(rs_value);
                
                //Push next task
                stall = !ctx->pushTask(ctx->ID_EX, self);
            }
            
            return (stall)? Error::PIPELINE_STALL : Error::NONE;
        })
        .EX(STAGE_TASK(){
            
            RISING_EDGE_FENCE();
            
            auto* ctx = self->context;
            return (ctx->pushTask(ctx->EX_DM, self))? Error::NONE : Error::PIPELINE_STALL;
        })
        .DM(RInstr::EmptyDM)
        .WB(STAGE_TASK(){
            
            RISING_EDGE_FENCE();
            
            return Error::NONE;
        });
    }
}

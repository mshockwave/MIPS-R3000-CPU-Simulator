#include "Task.h"

namespace task{
    task_t TasksTable[TASK_COUNT];
    instr_task_map_t FirstInstrOpMap;
    instr_task_map_t RtypeInstrFuncMap;

    void InitInstructionMap() {
        //Type R, delegate to another decoder
        FirstInstrOpMap[0x00] = TASK_R_TYPE_DECODER;

        //Type I
        FirstInstrOpMap[0x08] = OP_ADDI;
        FirstInstrOpMap[0x09] = OP_ADDIU;
        FirstInstrOpMap[0x23] = OP_LW;
        FirstInstrOpMap[0x21] = OP_LH;
        FirstInstrOpMap[0x25] = OP_LHU;
        FirstInstrOpMap[0x20] = OP_LB;
        FirstInstrOpMap[0x24] = OP_LBU;
        FirstInstrOpMap[0x2B] = OP_SW;
        FirstInstrOpMap[0x29] = OP_SH;
        FirstInstrOpMap[0x28] = OP_SB;
        FirstInstrOpMap[0x0F] = OP_LUI;
        FirstInstrOpMap[0x0C] = OP_ANDI;
        FirstInstrOpMap[0x0D] = OP_ORI;
        FirstInstrOpMap[0x0E] = OP_NORI;
        FirstInstrOpMap[0x0A] = OP_SLTI;
        FirstInstrOpMap[0x04] = OP_BEQ;
        FirstInstrOpMap[0x05] = OP_BNE;
        FirstInstrOpMap[0x07] = OP_BGTZ;

        //Type J
        FirstInstrOpMap[0x02] = OP_J;
        FirstInstrOpMap[0x03] = OP_JAL;

        FirstInstrOpMap[0x3F] = OP_HALT;

        /*Type R specific decoder*/
        RtypeInstrFuncMap[0x20] = OP_ADD;
        RtypeInstrFuncMap[0x21] = OP_ADDU;
        RtypeInstrFuncMap[0x22] = OP_SUB;
        RtypeInstrFuncMap[0x24] = OP_AND;
        RtypeInstrFuncMap[0x25] = OP_OR;
        RtypeInstrFuncMap[0x26] = OP_XOR;
        RtypeInstrFuncMap[0x27] = OP_NOR;
        RtypeInstrFuncMap[0x28] = OP_NAND;
        RtypeInstrFuncMap[0x2A] = OP_SLT;
        RtypeInstrFuncMap[0x00] = OP_SLL;
        RtypeInstrFuncMap[0x02] = OP_SRL;
        RtypeInstrFuncMap[0x03] = OP_SRA;
        RtypeInstrFuncMap[0x08] = OP_JR;

    }

    void InitTasks(){

        TasksTable[TASK_R_TYPE_DECODER] = TASK_HANDLER() {
            //Extract [5,0] bits
            uint8_t func = static_cast<uint8_t >(extractInstrBits(instruction->GetBitsInstruction(),
                                                                  5, 0));
            //TODO: Error handling: op code not found
            return RtypeInstrFuncMap[func];
        };

        /*R type instructions*/
        TasksTable[OP_ADD] = TASK_HANDLER() {
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("add")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            reg_t pend_rs = rs, pend_rt = rt;

            rd = rs + rt;
            if(isSumOverflow(pend_rs, pend_rt, rd)){
                context->putError(Error::NUMBER_OVERFLOW);
            }

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_ADDU] = TASK_HANDLER() {
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("addu")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            rd = rs + rt;

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_SUB] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("sub")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            reg_t pend_rs = rs, pend_rt = twoComplement(rt); //-rt

            rd = rs - rt;
            if(isSumOverflow(pend_rs, pend_rt, rd)){
                context->putError(Error::NUMBER_OVERFLOW);
            }

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_AND] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("and")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            rd = rs & rt;

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_OR] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("or")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            rd = rs | rt;

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_XOR] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("xor")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            rd = rs ^ rt;

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_NOR] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("nor")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            rd = ~(rs | rt);

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_NAND] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("nand")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            rd = ~(rs & rt);

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_SLT] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("slt")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            rd = (static_cast<int32_t>(rs) < static_cast<int32_t>(rt))? U32_1 : U32_0;

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_SLL] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("sll")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            uint8_t shamt = RInstr::GetShAmt(instruction->GetBitsInstruction());
            rd = rt << shamt;

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_SRA] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("sra")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            uint8_t shamt = RInstr::GetShAmt(instruction->GetBitsInstruction());
            int32_t signRt = rt;
            int32_t signRd = signRt >> shamt;
            rd = (reg_t)signRd;

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_SRL] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("srl")
            }

            ASSERT_DEST_REG_NOT_ZERO(rd)

            uint8_t shamt = RInstr::GetShAmt(instruction->GetBitsInstruction());
            rd = rt >> shamt;
            //uint32_t mask = createBitMask( ((uint8_t)32) - shamt );
            //rd &= mask;

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_JR] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            DEBUG_BLOCK {
                PRINT_R_INSTR_DEBUG("jr")
            }

            Error e = context->setPC(rs);
            if(e == Error::NONE){
                return TASK_END;
            }

            context->putError(e);
            return TASK_BAIL;
        };

        /*I type instructions*/
        TasksTable[OP_ADDI] = TASK_HANDLER(){
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("addi")
            }

            ASSERT_DEST_REG_NOT_ZERO(rt)

            reg_t pend_rs = rs;

            //rt = rs + signExtend16(imm);
            int32_t s_rs = static_cast<int32_t>(rs);
            int16_t s_imm = static_cast<int16_t>(imm);
            rt = static_cast<reg_t>(s_rs + s_imm);
            if(isSumOverflow(pend_rs, signExtend16(imm), rt)){
                context->putError(Error::NUMBER_OVERFLOW);
            }

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_ADDIU] = TASK_HANDLER(){
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("addiu")
            }

            ASSERT_DEST_REG_NOT_ZERO(rt)

            rt = rs + static_cast<reg_t>(signExtend16(imm));

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_LW] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("lw")
            }

            try{
                int16_t s_imm = static_cast<int16_t>(imm);
                int32_t s_rs = static_cast<int32_t>(rs);
                word_t v = reverse32ByteOrder(context->getMemoryWord(s_rs + s_imm));
                
                ASSERT_DEST_REG_NOT_ZERO(rt)
                
                rt = static_cast<reg_t>(v);

                context->AdvancePC();

                return TASK_END;
            }catch(Error e){
                context->putError(e);
                return TASK_BAIL;
            }
        };

        TasksTable[OP_LH] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("lh")
            }

            try{
                int16_t s_imm = static_cast<int16_t>(imm);
                int32_t s_rs = static_cast<int32_t>(rs);
                half_w_t v = reverse16ByteOrder(context->getMemoryHalfWord(s_rs + s_imm));
                
                ASSERT_DEST_REG_NOT_ZERO(rt)
                
                rt = static_cast<reg_t>(signExtend16(v));

                context->AdvancePC();

                return TASK_END;
            }catch(Error e){
                context->putError(e);
                return TASK_BAIL;
            }
        };

        TasksTable[OP_LHU] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("lhu")
            }

            try{
                int16_t s_imm = static_cast<int16_t>(imm);
                int32_t s_rs = static_cast<int32_t>(rs);
                half_w_t v = reverse16ByteOrder(context->getMemoryHalfWord(s_rs + s_imm));
                
                ASSERT_DEST_REG_NOT_ZERO(rt)
                
                rt = static_cast<reg_t>(v);

                context->AdvancePC();

                return TASK_END;
            }catch(Error e){
                context->putError(e);
                return TASK_BAIL;
            }
        };

        TasksTable[OP_LB] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("lb")
            }

            try{
                int16_t s_imm = static_cast<int16_t>(imm);
                int32_t s_rs = static_cast<int32_t>(rs);
                byte_t v = context->getMemoryByte(s_rs + s_imm);
                
                ASSERT_DEST_REG_NOT_ZERO(rt)
                
                rt = static_cast<reg_t>(signExtend8(v));

                context->AdvancePC();

                return TASK_END;
            }catch(Error e){
                context->putError(e);
                return TASK_BAIL;
            }
        };

        TasksTable[OP_LBU] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("lbu")
            }

            try{
                int16_t s_imm = static_cast<int16_t>(imm);
                int32_t s_rs = static_cast<int32_t>(rs);
                byte_t v = context->getMemoryByte(s_rs + s_imm);
                
                ASSERT_DEST_REG_NOT_ZERO(rt)
                
                rt = static_cast<reg_t>(v);

                context->AdvancePC();

                return TASK_END;
            }catch(Error e){
                context->putError(e);
                return TASK_BAIL;
            }
        };

        TasksTable[OP_SW] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("sw")
            }

            try{
                int16_t s_imm = static_cast<int16_t>(imm);
                int32_t s_rs = static_cast<int32_t>(rs);
                word_t& var = context->getMemoryWord(s_rs + s_imm);
                var = reverse32ByteOrder(rt);

                context->AdvancePC();

                return TASK_END;
            }catch(Error e){
                context->putError(e);
                return TASK_BAIL;
            }
        };

        TasksTable[OP_SH] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("sh")
            }

            try{
                int16_t s_imm = static_cast<int16_t>(imm);
                int32_t s_rs = static_cast<int32_t>(rs);
                half_w_t& var = context->getMemoryHalfWord(s_rs + s_imm);
                var = reverse16ByteOrder(static_cast<half_w_t>(rt & 0x0000FFFF));

                context->AdvancePC();

                return TASK_END;
            }catch(Error e){
                context->putError(e);
                return TASK_BAIL;
            }
        };

        TasksTable[OP_SB] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("sb")
            }

            try{
                int16_t s_imm = static_cast<int16_t>(imm);
                int32_t s_rs = static_cast<int32_t>(rs);
                byte_t& var = context->getMemoryByte(s_rs + s_imm);
                var = static_cast<byte_t>(rt & 0x000000FF);

                context->AdvancePC();

                return TASK_END;
            }catch(Error e){
                context->putError(e);
                return TASK_BAIL;
            }
        };

        TasksTable[OP_LUI] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("lui")
            }

            ASSERT_DEST_REG_NOT_ZERO(rt)

            rt = static_cast<reg_t>(imm << 16);

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_ANDI] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("andi")
            }

            ASSERT_DEST_REG_NOT_ZERO(rt)

            rt = rs & static_cast<reg_t>(imm);

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_ORI] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("ori")
            }

            ASSERT_DEST_REG_NOT_ZERO(rt)

            rt = rs | static_cast<reg_t>(imm);

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_NORI] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("nori")
            }

            ASSERT_DEST_REG_NOT_ZERO(rt)

            rt = ~(rs | static_cast<reg_t>(imm));

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_SLTI] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("slti")
            }

            ASSERT_DEST_REG_NOT_ZERO(rt)

            rt = (static_cast<int32_t>(rs) < static_cast<int32_t>(signExtend16(imm)))? U32_1 : U32_0;

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_BEQ] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("beq")
            }

            int32_t s_rs = static_cast<int32_t>(rs);
            int32_t s_rt = static_cast<int32_t>(rt);
            
            if(s_rs == s_rt){
                reg_t offset = static_cast<reg_t>(WORD_WIDTH + (signExtend16(imm) << 2));
                Error e = context->setPC(context->GetPC() + offset);
                if(e == Error::NONE){
                    return TASK_END;
                }
                context->putError(e);
                return TASK_BAIL;
            }

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_BNE] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("bne")
            }
            
            int32_t s_rs = static_cast<int32_t>(rs);
            int32_t s_rt = static_cast<int32_t>(rt);

            if(s_rs != s_rt){
                reg_t offset = static_cast<reg_t>(WORD_WIDTH + (signExtend16(imm) << 2));
                Error e = context->setPC(context->GetPC() + offset);
                if(e == Error::NONE){
                    return TASK_END;
                }
                context->putError(e);
                return TASK_BAIL;
            }

            context->AdvancePC();

            return TASK_END;
        };

        TasksTable[OP_BGTZ] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            DEBUG_BLOCK {
                PRINT_I_INSTR_DEBUG("bgtz")
            }

            if(static_cast<int32_t>(rs) > 0){
                reg_t offset = static_cast<reg_t>(WORD_WIDTH + (signExtend16(imm) << 2));
                Error e = context->setPC(context->GetPC() + offset);
                if(e == Error::NONE){
                    return TASK_END;
                }
                context->putError(e);
                return TASK_BAIL;
            }

            context->AdvancePC();

            return TASK_END;
        };

        /*J type instruction*/
        TasksTable[OP_J] = TASK_HANDLER() {
            J_INSTR_DEF_ADDR()

            DEBUG_BLOCK {
                PRINT_J_INSTR_DEBUG("j")
            }

            reg_t pc = context->GetPC();
            pc += WORD_WIDTH;
            uint32_t partPC = extractInstrBits(pc, 31, 28);
            pc = (partPC << 28);

            Error e = context->setPC( pc | (addr << 2) );
            if(e == Error::NONE){
                return TASK_END;
            }else{
                context->putError(e);
                return TASK_BAIL;
            }
        };

        TasksTable[OP_JAL] = TASK_HANDLER() {
            J_INSTR_DEF_ADDR()

            DEBUG_BLOCK {
                PRINT_J_INSTR_DEBUG("jal")
            }

            reg_t pc = context->GetPC();
            pc += WORD_WIDTH;
            context->RA = pc;

            uint32_t partPC = extractInstrBits(pc, 31, 28);
            pc = (partPC << 28);

            Error e = context->setPC( pc | (addr << 2) );
            if(e == Error::NONE){
                return TASK_END;
            }else{
                context->putError(e);
                return TASK_BAIL;
            }
        };

        TasksTable[OP_HALT] = TASK_HANDLER() {
            
            return TASK_END;
        };
    }

    namespace RInstr{
        /*[25, 21] bits*/
        inline uint8_t GetRs(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 25, 21));
        }
        /*[20, 16] bits*/
        inline uint8_t GetRt(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 20, 16));
        }
        /*[15, 11] bits*/
        inline uint8_t GetRd(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 15, 11));
        }
        /*[10, 6] bits*/
        inline uint8_t GetShAmt(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 10, 6));
        }
    } //namespace RInstr

    namespace IInstr {
        /*[25, 21] bits*/
        inline uint8_t GetRs(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 25, 21));
        }
        /*[20, 16] bits*/
        inline uint8_t GetRt(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 20, 16));
        }
        /*[15, 0] bits*/
        inline uint16_t GetImm(uint32_t instruction){
            return static_cast<uint16_t>(extractInstrBits(instruction, 15, 0));
        }
    } //namespace IInstr

    namespace JInstr {
        /*[25, 0] bits*/
        inline uint32_t GetAddr(uint32_t instruction){
            return extractInstrBits(instruction, 25, 0);
        }
    } //namespace JInstr
} //namespace task

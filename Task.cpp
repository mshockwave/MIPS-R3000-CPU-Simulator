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
        TasksTable[TASK_ROOT_DECODER] = TASK_HANDLER() {
            //Extract [31,26] bits
            uint8_t op = static_cast<uint8_t >(extractInstrBits(instruction->getBitsInstruction(),
                                                                31, 26));
            return FirstInstrOpMap[op];
        };
        TasksTable[TASK_R_TYPE_DECODER] = TASK_HANDLER() {
            //Extract [5,0] bits
            uint8_t func = static_cast<uint8_t >(extractInstrBits(instruction->getBitsInstruction(),
                                                                  5, 0));
            //TODO: Error handling: op code not found
            return RtypeInstrFuncMap[func];
        };

        /*R type instructions*/
        TasksTable[OP_ADD] = TASK_HANDLER() {
            R_INSTR_DEF_REGS()

            //TODO: Overflow detection
            rd = rs + rt;

            return TASK_END;
        };

        TasksTable[OP_ADDU] = TASK_HANDLER() {
            R_INSTR_DEF_REGS()

            //TODO: Unsigned check
            rd = rs + rt;

            return TASK_END;
        };

        TasksTable[OP_SUB] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            //TODO: Overflow detection
            rd = rs - rt;

            return TASK_END;
        };

        TasksTable[OP_AND] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            rd = rs & rt;

            return TASK_END;
        };

        TasksTable[OP_OR] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            rd = rs | rt;

            return TASK_END;
        };

        TasksTable[OP_XOR] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            rd = rs ^ rt;

            return TASK_END;
        };

        TasksTable[OP_NOR] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            rd = ~(rs | rt);

            return TASK_END;
        };

        TasksTable[OP_NAND] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            rd = ~(rs & rt);

            return TASK_END;
        };

        TasksTable[OP_SLT] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            rd = (rs < rt)? U32_1 : U32_0;

            return TASK_END;
        };

        TasksTable[OP_SLL] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            uint8_t shamt = RInstr::GetShAmt(instruction->getBitsInstruction());
            rd = rs << shamt;

            return TASK_END;
        };

        TasksTable[OP_SRA] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            uint8_t shamt = RInstr::GetShAmt(instruction->getBitsInstruction());
            rd = rs >> shamt;

            return TASK_END;
        };

        TasksTable[OP_SRL] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            uint8_t shamt = RInstr::GetShAmt(instruction->getBitsInstruction());
            rd = rs >> shamt;
            uint32_t mask = createBitMask( ((uint8_t)32) - shamt );
            rd &= mask;

            return TASK_END;
        };

        TasksTable[OP_JR] = TASK_HANDLER(){
            R_INSTR_DEF_REGS()

            //TODO: Check for misalignment error
            context->PC = rs;

            return TASK_END;
        };

        /*I type instructions*/
        TasksTable[OP_ADDI] = TASK_HANDLER(){
            I_INSTR_DEF_ARGS()

            //TODO: Check overflow
            rt = rs + signExtend16(imm);

            return TASK_END;
        };

        TasksTable[OP_ADDIU] = TASK_HANDLER(){
            I_INSTR_DEF_ARGS()

            //TODO: Check unsigned
            rt = rs + static_cast<reg_t>(imm);

            return TASK_END;
        };

        TasksTable[OP_LW] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            //TODO: Check memory overflow
            //TODO: Check alignment
            rt = *((reg_t*)( (context->Memory) + rs + imm ));

            return TASK_END;
        };

        TasksTable[OP_LH] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            //TODO: Check memory overflow
            //TODO: Check alignment
            rt = signExtend16(*((half_w_t*)( (context->Memory) + rs + imm )));

            return TASK_END;
        };

        TasksTable[OP_LHU] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            //TODO: Check memory overflow
            //TODO: Check alignment
            //TODO: Check unsigned
            rt = static_cast<reg_t>(*((half_w_t*)( (context->Memory) + rs + imm )));

            return TASK_END;
        };

        TasksTable[OP_LB] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            rt = signExtend8(*((byte_t*)( (context->Memory) + rs + imm )));

            return TASK_END;
        };

        TasksTable[OP_LBU] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            rt = static_cast<reg_t>(*((byte_t*)( (context->Memory) + rs + imm )));

            return TASK_END;
        };

        TasksTable[OP_SW] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            *((reg_t*)( (context->Memory) + rs + imm )) = rt;

            return TASK_END;
        };

        TasksTable[OP_SH] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            *((half_w_t*)( (context->Memory) + rs + imm )) = static_cast<half_w_t>(rt & 0x0000FFFF);

            return TASK_END;
        };

        TasksTable[OP_SB] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            *((byte_t*)( (context->Memory) + rs + imm )) = static_cast<byte_t>(rt & 0x000000FF);

            return TASK_END;
        };

        TasksTable[OP_LUI] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            rt = static_cast<reg_t>(imm << 16);

            return TASK_END;
        };

        TasksTable[OP_ANDI] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            rt = rs & static_cast<reg_t>(imm);

            return TASK_END;
        };

        TasksTable[OP_ORI] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            rt = rs | static_cast<reg_t>(imm);

            return TASK_END;
        };

        TasksTable[OP_NORI] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            rt = ~(rs | static_cast<reg_t>(imm));

            return TASK_END;
        };

        TasksTable[OP_SLTI] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            rt = (static_cast<int32_t>(rs) < static_cast<int32_t>(signExtend16(imm)))? U32_1 : U32_0;

            return TASK_END;
        };

        TasksTable[OP_BEQ] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            if(rs == rt){
                context->PC += (4 + (signExtend16(imm) << 2));
            }

            return TASK_END;
        };

        TasksTable[OP_BNE] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            if(rs != rt){
                context->PC += (4 + (signExtend16(imm) << 2));
            }

            return TASK_END;
        };

        TasksTable[OP_BGTZ] = TASK_HANDLER() {
            I_INSTR_DEF_ARGS()

            if(static_cast<uint32_t>(rs) > 0){
                context->PC += (4 + (signExtend16(imm) << 2));
            }

            return TASK_END;
        };

        /*J type instruction*/
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

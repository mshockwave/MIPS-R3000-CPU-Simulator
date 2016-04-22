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

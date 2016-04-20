
#include "Task.h"
#include "FirstClassTasks.h"

namespace task {

    instr_task_map_t FirstInstrOpMap;
    instr_task_map_t RtypeInstrFuncMap;

    void InitInstructionMap() {
        //Type R, delegate to another decoder
        FirstInstrOpMap[0x00] = ID_R_TYPE;

        //Type I
        FirstInstrOpMap[0x08] = ID_ADDI;
        FirstInstrOpMap[0x09] = ID_ADDIU;
        FirstInstrOpMap[0x23] = ID_LW;
        FirstInstrOpMap[0x21] = ID_LH;
        FirstInstrOpMap[0x25] = ID_LHU;
        FirstInstrOpMap[0x20] = ID_LB;
        FirstInstrOpMap[0x24] = ID_LBU;
        FirstInstrOpMap[0x2B] = ID_SW;
        FirstInstrOpMap[0x29] = ID_SH;
        FirstInstrOpMap[0x28] = ID_SB;
        FirstInstrOpMap[0x0F] = ID_LUI;
        FirstInstrOpMap[0x0C] = ID_ANDI;
        FirstInstrOpMap[0x0D] = ID_ORI;
        FirstInstrOpMap[0x0E] = ID_NORI;
        FirstInstrOpMap[0x0A] = ID_SLTI;
        FirstInstrOpMap[0x04] = ID_BEQ;
        FirstInstrOpMap[0x05] = ID_BNE;
        FirstInstrOpMap[0x07] = ID_BGTZ;

        //Type J
        FirstInstrOpMap[0x02] = ID_J;
        FirstInstrOpMap[0x03] = ID_JAL;

        FirstInstrOpMap[0x3F] = OP_HALT;

        /*Type R specific decoder*/
        RtypeInstrFuncMap[0x20] = EXE_ADD;
        RtypeInstrFuncMap[0x21] = EXE_ADDU;
        RtypeInstrFuncMap[0x22] = EXE_SUB;
        RtypeInstrFuncMap[0x24] = EXE_AND;
        RtypeInstrFuncMap[0x25] = EXE_OR;
        RtypeInstrFuncMap[0x26] = EXE_XOR;
        RtypeInstrFuncMap[0x27] = EXE_NOR;
        RtypeInstrFuncMap[0x28] = EXE_NAND;
        RtypeInstrFuncMap[0x2A] = EXE_SLT;
        RtypeInstrFuncMap[0x00] = EXE_SLL;
        RtypeInstrFuncMap[0x02] = EXE_SRL;
        RtypeInstrFuncMap[0x03] = EXE_SRA;
        RtypeInstrFuncMap[0x08] = EXE_JR;

    }
}//namespace task



#include "Task.h"

void task::InitInstructionMap() {
    //Type R, delegate to another decoder
    FirstInstrMap[0x00] = TASK_R_TYPE_DECODER;

    //Type I
    FirstInstrMap[0x08] = OP_ADDI;
    FirstInstrMap[0x09] = OP_ADDIU;
    FirstInstrMap[0x23] = OP_LW;
    FirstInstrMap[0x21] = OP_LH;
    FirstInstrMap[0x25] = OP_LHU;
    FirstInstrMap[0x20] = OP_LB;
    FirstInstrMap[0x24] = OP_LBU;
    FirstInstrMap[0x2B] = OP_SW;
    FirstInstrMap[0x29] = OP_SH;
    FirstInstrMap[0x28] = OP_SB;
    FirstInstrMap[0x0F] = OP_LUI;
    FirstInstrMap[0x0C] = OP_ANDI;
    FirstInstrMap[0x0D] = OP_ORI;
    FirstInstrMap[0x0E] = OP_NORI;
    FirstInstrMap[0x0A] = OP_SLTI;
    FirstInstrMap[0x04] = OP_BEQ;
    FirstInstrMap[0x05] = OP_BNE;
    FirstInstrMap[0x07] = OP_BGTZ;

    //Type J
    FirstInstrMap[0x02] = OP_J;
    FirstInstrMap[0x03] = OP_JAL;

    FirstInstrMap[0x3F] = OP_HALT;

    /*Type R specific decoder*/
    RtypeInstrMap[0x20] = OP_ADD;
    RtypeInstrMap[0x21] = OP_ADDU;
    RtypeInstrMap[0x22] = OP_SUB;
    RtypeInstrMap[0x24] = OP_AND;
    RtypeInstrMap[0x25] = OP_OR;
    RtypeInstrMap[0x26] = OP_XOR;
    RtypeInstrMap[0x27] = OP_NOR;
    RtypeInstrMap[0x28] = OP_NAND;
    RtypeInstrMap[0x2A] = OP_SLT;
    RtypeInstrMap[0x00] = OP_SLL;
    RtypeInstrMap[0x02] = OP_SRL;
    RtypeInstrMap[0x03] = OP_SRA;
    RtypeInstrMap[0x08] = OP_JR;

}

void task::InitTasks(){
    TasksTable[TASK_ROOT_DECODER] = TASK_HANDLER() {
        //Extract [31,26] bits
        uint8_t op = static_cast<uint8_t >(extractInstrBits(instruction->getBitsInstruction(),
                                                            31, 26));
        return FirstInstrMap[op];
    };
    TasksTable[TASK_R_TYPE_DECODER] = TASK_HANDLER() {
        //Extract [5,0] bits
        uint8_t func = static_cast<uint8_t >(extractInstrBits(instruction->getBitsInstruction(),
                                                              5, 0));
        return RtypeInstrMap[func];
    };

    /*R type instructions*/
    TasksTable[OP_ADD] = TASK_HANDLER() {

        return TASK_END;
    };

    TasksTable[OP_ADDU] = TASK_HANDLER() {

        return TASK_END;
    };
}

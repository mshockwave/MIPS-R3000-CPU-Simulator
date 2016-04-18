
#ifndef ARCHIHW1_TASKS_FIRSTCLASSTASKS_H
#define ARCHIHW1_TASKS_FIRSTCLASSTASKS_H

#include "Task.h"

namespace task{

    //Reserve 0
    DEF_TASK(TASK_END, 36)
    DEF_TASK(TASK_BAIL, 38) //Exit with error

    //DEF_TASK(TASK_ROOT_DECODER, 1) //Legacy Root Level Decoder
    DEF_TASK(TASK_R_TYPE_DECODER, 2)

    /*First class tasks*/
    //Type R Instructions
    DEF_TASK(OP_ADD, 3)
    DEF_TASK(OP_ADDU, 4)
    DEF_TASK(OP_SUB, 5)
    DEF_TASK(OP_AND, 6)
    DEF_TASK(OP_OR, 7)
    DEF_TASK(OP_XOR, 8)
    DEF_TASK(OP_NOR, 9)
    DEF_TASK(OP_NAND, 10)
    DEF_TASK(OP_SLT, 11)
    DEF_TASK(OP_SLL, 12)
    DEF_TASK(OP_SRL, 13)
    DEF_TASK(OP_SRA, 14)
    DEF_TASK(OP_JR, 15)

    //Type I Instructions
    DEF_TASK(OP_ADDI, 16)
    DEF_TASK(OP_ADDIU, 17)
    DEF_TASK(OP_LW, 18)
    DEF_TASK(OP_LH, 19)
    DEF_TASK(OP_LHU, 20)
    DEF_TASK(OP_LB, 21)
    DEF_TASK(OP_LBU, 22)
    DEF_TASK(OP_SW, 23)
    DEF_TASK(OP_SH, 24)
    DEF_TASK(OP_SB, 25)
    DEF_TASK(OP_LUI, 26)
    DEF_TASK(OP_ANDI, 27)
    DEF_TASK(OP_ORI, 28)
    DEF_TASK(OP_NORI, 29)
    DEF_TASK(OP_SLTI, 30)
    DEF_TASK(OP_BEQ, 31)
    DEF_TASK(OP_BNE, 32)
    DEF_TASK(OP_BGTZ, 33)

    //Type J Instructions
    DEF_TASK(OP_J, 34)
    DEF_TASK(OP_JAL, 35)

    DEF_TASK(OP_HALT, 37)
}//namespace task

#endif //ARCHIHW1_TASKS_FIRSTCLASSTASKS_H


#ifndef ARCHIHW1_TASKS_FIRSTCLASSTASKS_H
#define ARCHIHW1_TASKS_FIRSTCLASSTASKS_H

#include "FragmentTasksR.h"
#include "FragmentTasksI.h"
#include "FragmentTasksJ.h"

namespace task{

    //Reserve 0
    DEF_TASK(TASK_END, 36)
    DEF_TASK(TASK_BAIL, 38) //Exit with error

    DEF_TASK(ID_R_TYPE, 2)

    /*First class tasks*/
    //Type R Instructions
    DEF_TASK(EXE_ADD, 3)
    DEF_TASK(EXE_ADDU, 4)
    DEF_TASK(EXE_SUB, 5)
    DEF_TASK(EXE_AND, 6)
    DEF_TASK(EXE_OR, 7)
    DEF_TASK(EXE_XOR, 8)
    DEF_TASK(EXE_NOR, 9)
    DEF_TASK(EXE_NAND, 10)
    DEF_TASK(EXE_SLT, 11)
    DEF_TASK(EXE_SLL, 12)
    DEF_TASK(EXE_SRL, 13)
    DEF_TASK(EXE_SRA, 14)
    DEF_TASK(EXE_JR, 15)

    //Type I Instructions
    DEF_TASK(ID_ADDI, 16)
    DEF_TASK(ID_ADDIU, 17)
    DEF_TASK(ID_LW, 18)
    DEF_TASK(ID_LH, 19)
    DEF_TASK(ID_LHU, 20)
    DEF_TASK(ID_LB, 21)
    DEF_TASK(ID_LBU, 22)
    DEF_TASK(ID_SW, 23)
    DEF_TASK(ID_SH, 24)
    DEF_TASK(ID_SB, 25)
    DEF_TASK(ID_LUI, 26)
    DEF_TASK(ID_ANDI, 27)
    DEF_TASK(ID_ORI, 28)
    DEF_TASK(ID_NORI, 29)
    DEF_TASK(ID_SLTI, 30)
    DEF_TASK(ID_BEQ, 31)
    DEF_TASK(ID_BNE, 32)
    DEF_TASK(ID_BGTZ, 33)

    //Type J Instructions
    DEF_TASK(ID_J, 34)
    DEF_TASK(ID_JAL, 35)

    DEF_TASK(OP_HALT, 37)
}//namespace task

#endif //ARCHIHW1_TASKS_FIRSTCLASSTASKS_H

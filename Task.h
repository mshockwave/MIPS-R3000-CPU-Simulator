
#ifndef ARCHIHW1_TASK_H
#define ARCHIHW1_TASK_H

#include "Types.h"
#include "Instruction.h"

typedef std::function<task_id_t(Context *, Instruction*)> task_t;

namespace Task {

#define DEF_TASK(name, id) \
    const task_id_t name = (id);

    const uint32_t TASK_COUNT = 40;

    task_t TasksTable[TASK_COUNT];

    void InitTasks();

    //Task ids
    DEF_TASK(TASK_END, 0)       //Reserve
    DEF_TASK(TASK_DECODE, 1)    //Reserve

    //Type I Instructions
    DEF_TASK(OP_ADD, 2)
    DEF_TASK(OP_ADDU, 3)
    DEF_TASK(OP_SUB, 4)
    DEF_TASK(OP_AND, 5)
    DEF_TASK(OP_OR, 6)
    DEF_TASK(OP_XOR, 7)
    DEF_TASK(OP_NOR, 8)
    DEF_TASK(OP_NAND, 9)
}

#endif //ARCHIHW1_TASK_H

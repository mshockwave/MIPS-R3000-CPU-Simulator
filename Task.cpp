
#include "Task.h"

void Task::InitTasks(){
    TasksTable[OP_ADD] = [](Context* ctx, Instruction* instruction) -> task_id_t {

        return TASK_END;
    };

    TasksTable[OP_ADDU] = [](Context* ctx, Instruction* instruction) -> task_id_t {

        return TASK_END;
    };
}

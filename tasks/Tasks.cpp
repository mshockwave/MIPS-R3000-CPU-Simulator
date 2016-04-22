#include "../Task.h"
#include "RTasks.h"
#include "ITasks.h"
#include "JTasks.h"

namespace task{
    void InitTasks(){

        /*Init different types of instructions*/
        InitRTasks();

        InitITasks();

        InitJTasks();
    }
}



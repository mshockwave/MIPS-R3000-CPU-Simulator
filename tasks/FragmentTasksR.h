
#ifndef ARCHIHW1_FRAGMENTTASKSR_H
#define ARCHIHW1_FRAGMENTTASKSR_H

#include <cstdint>
#include "Task.h"

namespace task {

#define DEF_R_FRAG(name, id) \
    DEF_TASK(name, FRAG_R_BASE_INDEX + (id))

    DEF_R_FRAG(DM_R_NOP, 0)
    DEF_R_FRAG(WB_R_WRITE_REGS, 1)

}//namespace task

#endif //ARCHIHW1_FRAGMENTTASKSR_H

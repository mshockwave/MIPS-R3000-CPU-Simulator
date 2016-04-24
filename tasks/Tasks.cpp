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


        /*Init special task*/
        TasksTable[OP_NOP].Name("nop", OP_NOP)
                .IF(STAGE_TASK(){

                    auto* ctx = self->context;

                    RISING_EDGE_FENCE();

                    ctx->pushTask(ctx->IF_ID, self);

                    return Error::NONE;
                })
                .ID(STAGE_TASK(){

                    auto* ctx = self->context;

                    RISING_EDGE_FENCE();

                    ctx->pushTask(ctx->ID_EX, self);

                    return Error::NONE;
                })
                .EX(STAGE_TASK(){

                    auto* ctx = self->context;

                    RISING_EDGE_FENCE();

                    ctx->pushTask(ctx->EX_DM, self);

                    return Error::NONE;
                })
                .DM(STAGE_TASK(){

                    auto* ctx = self->context;

                    RISING_EDGE_FENCE();

                    ctx->pushTask(ctx->DM_WB, self);

                    return Error::NONE;
                }).WB(STAGE_TASK(){

                    RISING_EDGE_FENCE();

                    return Error::NONE;
                });

        TasksTable[OP_HALT].Name("halt", OP_HALT)
                .IF(STAGE_TASK(){

                    auto* ctx = self->context;

                    RISING_EDGE_FENCE();

                    ctx->pushTask(ctx->IF_ID, self);

                    return Error::NONE;
                })
                .ID(STAGE_TASK(){

                    auto* ctx = self->context;

                    RISING_EDGE_FENCE();

                    ctx->pushTask(ctx->ID_EX, self);

                    return Error::NONE;
                })
                .EX(STAGE_TASK(){

                    auto* ctx = self->context;

                    RISING_EDGE_FENCE();

                    ctx->pushTask(ctx->EX_DM, self);

                    return Error::NONE;
                })
                .DM(STAGE_TASK(){

                    auto* ctx = self->context;

                    RISING_EDGE_FENCE();

                    ctx->pushTask(ctx->DM_WB, self);

                    return Error::NONE;
                }).WB(STAGE_TASK(){

                    RISING_EDGE_FENCE();

                    return Error::NONE;
                });
    }
}



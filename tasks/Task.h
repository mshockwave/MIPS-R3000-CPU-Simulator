
#ifndef ARCHIHW1_TASKS_TASK_H
#define ARCHIHW1_TASKS_TASK_H

#include "../Types.h"
#include "../Utils.h"

#include <cstdint>
#include <unordered_map>

namespace task {

#define ASSERT_DEST_REG_NOT_ZERO(dest) \
    if( &(dest) == &(context->ZERO) ){ \
        context->putError(Error::WRITE_REG_ZERO); \
        context->AdvancePC(); \
        return TASK_END; \
    }

#define DEF_TASK(name, id) \
    const task_id_t name = (id);

    const uint32_t TASK_COUNT = 39;

    extern task_t TasksTable[TASK_COUNT];

    void InitTasks();
    void InitFirstClassTasks();
    void InitFragmentTasksR();
    void InitFragmentTasksI();
    void InitFragmentTasksJ();

#define TASK_HANDLER() \
    [](Context* context, Instruction* instruction) -> task_id_t

    /*Instruction map*/
    /*Map instruction op code -> task_id_t*/
    //Use typedef for future extension. e.g. Replace it with sparse_vector
    typedef std::unordered_map<uint8_t, task_id_t > instr_task_map_t;
    //First level
    extern instr_task_map_t FirstInstrOpMap;
    //R type's second level
    extern instr_task_map_t RtypeInstrFuncMap;
    void  InitInstructionMap();

    namespace RInstr{
        /*Routines for R type instructions*/

        inline uint8_t GetRs(uint32_t instruction);

        inline uint8_t GetRt(uint32_t instruction);

        inline uint8_t GetRd(uint32_t instruction);

        inline uint8_t GetShAmt(uint32_t instruction);
    }//namespace RInstr

    namespace IInstr{
        /*Routines for I type instructions*/

        inline uint8_t GetRs(uint32_t instruction);

        inline uint8_t GetRt(uint32_t instruction);

        inline uint16_t GetImm(uint32_t instruction);
    }//namespace IInstr

    namespace JInstr{
        /*Routines for J type instructions*/

        inline uint32_t GetAddr(uint32_t instruction);
    }//namespace JInstr


}//namespace task

#endif //ARCHIHW1_TASKS_TASK_H

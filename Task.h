
#ifndef ARCHIHW1_TASK_H
#define ARCHIHW1_TASK_H

#include <unordered_map>

#include "Types.h"
#include "Instruction.h"
#include "Utils.h"

typedef std::function<task_id_t(Context *, Instruction*)> task_t;

namespace task {

#define DEF_TASK(name, id) \
    const task_id_t name = (id);

    const uint32_t TASK_COUNT = 38;

    extern task_t TasksTable[TASK_COUNT];

    void InitTasks();
#define TASK_HANDLER() \
    [](Context* context, Instruction* instruction) -> task_id_t

    /*Task ids*/
    //Reserve 0
    DEF_TASK(TASK_END, 37)
    DEF_TASK(TASK_ROOT_DECODER, 1)
    DEF_TASK(TASK_R_TYPE_DECODER, 2)

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

    DEF_TASK(OP_HALT, 36)

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

} //namespace task

#endif //ARCHIHW1_TASK_H

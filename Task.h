
#ifndef ARCHIHW1_TASK_H
#define ARCHIHW1_TASK_H

#include <unordered_map>

#include "Types.h"
#include "Context.h"
#include "Instruction.h"
#include "Utils.h"
#include "tasks/TaskHandle.h"

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
#define TASK_HANDLER() \
    [](Context* context, Instruction* instruction) -> task_id_t

    /*Task ids*/
    //Reserve 0
    DEF_TASK(TASK_END, 36)
    DEF_TASK(TASK_BAIL, 38) //Exit with error

    //DEF_TASK(TASK_ROOT_DECODER, 1) //Legacy Root Level Decoder
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

    DEF_TASK(OP_HALT, 37)

    /*Instruction map*/
    /*Map instruction op code -> task_id_t*/
    //Use typedef for future extension. e.g. Replace it with sparse_vector
    typedef std::unordered_map<uint8_t, task_id_t > instr_task_map_t;
    //First level
    extern instr_task_map_t FirstInstrOpMap;
    //R type's second level
    extern instr_task_map_t RtypeInstrFuncMap;
    void  InitInstructionMap();

#define R_INSTR_DEF_REGS() \
    reg_t& rs = context->Registers[RInstr::GetRs(instruction->GetBitsInstruction())]; \
    reg_t& rt = context->Registers[RInstr::GetRt(instruction->GetBitsInstruction())]; \
    reg_t& rd = context->Registers[RInstr::GetRd(instruction->GetBitsInstruction())];

#define PRINT_R_INSTR_DEBUG(op_name) \
    Log::D("Task R Type") << "TimeStamp: " << getCurrentTimeMs() << std::endl; \
    Log::D("Task R Type") << "Cycle: " << context->GetCycleCounter() << std::endl; \
    Log::D("Task R Type") << "Op Name: " << (op_name) << std::endl; \
    Log::D("Task R Type") << "Rs: " << \
                            static_cast<unsigned int>(RInstr::GetRs(instruction->GetBitsInstruction())) << \
                            ". Value: 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << rs << std::endl;\
    Log::D("Task R Type") << "Rt: " << \
                            static_cast<unsigned int>(RInstr::GetRt(instruction->GetBitsInstruction())) << \
                            ". Value: 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << rt << std::endl;\
    Log::D("Task R Type") << "Rd: " << \
                            static_cast<unsigned int>(RInstr::GetRd(instruction->GetBitsInstruction())) << \
                            ". Value: 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << rd << std::endl; \
    Log::D("Task R Type") << "------------------------------------" << std::endl;

#define I_INSTR_DEF_ARGS() \
    reg_t& rs = context->Registers[IInstr::GetRs(instruction->GetBitsInstruction())]; \
    reg_t& rt = context->Registers[IInstr::GetRt(instruction->GetBitsInstruction())]; \
    uint16_t imm = IInstr::GetImm(instruction->GetBitsInstruction());

#define PRINT_I_INSTR_DEBUG(op_name) \
    Log::D("Task I Type") << "TimeStamp: " << getCurrentTimeMs() << std::endl; \
    Log::D("Task I Type") << "Cycle: " << context->GetCycleCounter() << std::endl; \
    Log::D("Task I Type") << "Op Name: " << (op_name) << std::endl; \
    Log::D("Task I Type") << "Rs: " << \
                            static_cast<unsigned int>(RInstr::GetRs(instruction->GetBitsInstruction())) << \
                            ". Value: 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << rs << std::endl;\
    Log::D("Task I Type") << "Rt: " << \
                            static_cast<unsigned int>(RInstr::GetRt(instruction->GetBitsInstruction())) << \
                            ". Value: 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << rt << std::endl;\
    Log::D("Task I Type") << "Imm: 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << \
                            static_cast<unsigned int>(IInstr::GetImm(instruction->GetBitsInstruction())) << std::endl; \
    Log::D("Task I Type") << "------------------------------------" << std::endl;

#define J_INSTR_DEF_ADDR() \
    uint32_t addr = JInstr::GetAddr(instruction->GetBitsInstruction());

#define PRINT_J_INSTR_DEBUG(op_name) \
    Log::D("Task J Type") << "TimeStamp: " << getCurrentTimeMs() << std::endl; \
    Log::D("Task J Type") << "Cycle: " << context->GetCycleCounter() << std::endl; \
    Log::D("Task J Type") << "Op Name: " << (op_name) << std::endl; \
    Log::D("Task J Type") << "Addr: 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << \
                            static_cast<unsigned int>(JInstr::GetAddr(instruction->GetBitsInstruction())) << std::endl; \
    Log::D("Task J Type") << "------------------------------------" << std::endl;

    namespace RInstr{
        /*[25, 21] bits*/
        inline uint8_t GetRs(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 25, 21));
        }
        /*[20, 16] bits*/
        inline uint8_t GetRt(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 20, 16));
        }
        /*[15, 11] bits*/
        inline uint8_t GetRd(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 15, 11));
        }
        /*[10, 6] bits*/
        inline uint8_t GetShAmt(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 10, 6));
        }
    } //namespace RInstr

    namespace IInstr {
        /*[25, 21] bits*/
        inline uint8_t GetRs(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 25, 21));
        }
        /*[20, 16] bits*/
        inline uint8_t GetRt(uint32_t instruction){
            return static_cast<uint8_t>(extractInstrBits(instruction, 20, 16));
        }
        /*[15, 0] bits*/
        inline uint16_t GetImm(uint32_t instruction){
            return static_cast<uint16_t>(extractInstrBits(instruction, 15, 0));
        }
    } //namespace IInstr

    namespace JInstr {
        /*[25, 0] bits*/
        inline uint32_t GetAddr(uint32_t instruction){
            return extractInstrBits(instruction, 25, 0);
        }
    } //namespace JInstr

} //namespace task

#endif //ARCHIHW1_TASK_H

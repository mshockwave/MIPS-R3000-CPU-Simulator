
#include "Task.h"

namespace task {

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

        template <
                bool load_rs = true,
                bool load_rt = true,
                bool load_rd = true
        >
        inline void loadIDRegs(Context* ctx, Instruction* instruction){
            RegisterTuple regs;
            if(load_rs){
                regs.Rs_Index = GetRs(instruction->GetBitsInstruction());
                regs.Rs = ctx->Registers[regs.Rs_Index].GetCurrent();
            }

            if(load_rt){
                regs.Rt_Index = GetRt(instruction->GetBitsInstruction());
                regs.Rt = ctx->Registers[regs.Rt_Index].GetCurrent();
            }

            if(load_rd){
                regs.Rd_Index = GetRs(instruction->GetBitsInstruction());
            }

            ctx->ID_EXE.EXE.Regs = regs;
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

    task_t TasksTable[TASK_COUNT];

    void InitTasks(){
        InitFirstClassTasks();
        InitFragmentTasksR();
        InitFragmentTasksI();
        InitFragmentTasksJ();
    }

}//namespace task


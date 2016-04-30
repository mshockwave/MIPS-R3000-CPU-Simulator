
#ifndef ARCHIHW1_TESTTASKSERROR_H
#define ARCHIHW1_TESTTASKSERROR_H

#include "TestTasks.h"
#include "../Context.h"
#include "../Task.h"
#include "../tasks/TaskHandle.h"
#include "../ExecutionEngine.h"
#include "../IFEngine.h"

#include <boost/thread/thread.hpp>

class TestTasksError : public TestCase {

public:
    TestTasksError() :
            TestCase("TestTasksError") {}

private:

    bool verifyWrite2ZeroRegister(){
#ifndef NDEBUG

        Log::D(mName) << "Verifying: write to zero register error" << std::endl;

        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();

        ctx.mInstrStartAddr = 0x00;
        //ctx.Registers[1] = 0xA;
        //ctx.Registers[4] = 0xA;

        Instructions instructions;
        /* add $0, $1, $2 */
        const byte_t raw_add_instr[4] = { 0x00, 0x22, 0x00, 0x20 };
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        /* here: halt x5 */
        const byte_t raw_halt_instr[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        Instruction halt_instr(raw_halt_instr);
        for(int i = 0; i < 5; i++){
            instructions.mInstructions.push_back(halt_instr);
        }

        ctx.setInstructionCount((uint32_t)instructions.length());

        TaskHandle::ClockHandle::Barrier rising_barrier(5);
        TaskHandle::ClockHandle::Barrier falling_barrier(5);
        TaskHandle::ClockHandle clock_handle(rising_barrier, falling_barrier);

        /*IF*/
        auto* if_thread = group.create_thread([&]()->void{
            IFEngine engine(ctx, instructions, clock_handle);
            engine.Start();
        });
        /*ID*/
        auto* id_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::IDEngineRunnable);
            engine.Start();
        });
        /*EX*/
        auto* ex_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::EXEngineRunnable);
            engine.Start();
        });
        /*DM*/
        auto* dm_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::DMEngineRunnable);
            engine.Start();
        });
        /*WB*/
        auto* wb_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::WBEngineRunnable);
            engine.Start();
        });

        ctx.StartPrinterLoop(if_thread,
                             id_thread,
                             ex_thread,
                             dm_thread,
                             wb_thread);

        //Log::V(mName) << "$4 Value: " << (int)ctx.Registers[4] << std::endl;
        //Log::V(mName) << "$3 Value: " << (int)ctx.Registers[3] << std::endl;
        //Log::V(mName) << "$5 Value: " << (int)ctx.Registers[5] << std::endl;

        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif
    }
    
    bool verifyDataMisAlignment(){
#ifndef NDEBUG
        
        Log::D(mName) << "Verifying: data misalignment" << std::endl;
        
        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();
        
        ctx.mInstrStartAddr = 0x00;
        
        Instructions instructions;
        
        /*lw $1, 3($0)*/
        const byte_t raw_lw_instr[4] = {0x8C, 0x01, 0x00, 0x03};
        Instruction lw_instr(raw_lw_instr);
        instructions.mInstructions.push_back(lw_instr);
        
        /*Test if forwarding mechanism would cause dead lock*/
        /*add $2, $1, $3*/
        const byte_t raw_add_instr[4] = {0x00, 0x23, 0x10, 0x20};
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        
        /* halt x5 */
        const byte_t raw_halt_instr[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        Instruction halt_instr(raw_halt_instr);
        for(int i = 0; i < 5; i++){
            instructions.mInstructions.push_back(halt_instr);
        }
        
        ctx.setInstructionCount((uint32_t)instructions.length());
        
        TaskHandle::ClockHandle::Barrier rising_barrier(5);
        TaskHandle::ClockHandle::Barrier falling_barrier(5);
        TaskHandle::ClockHandle clock_handle(rising_barrier, falling_barrier);
        
        /*IF*/
        auto* if_thread = group.create_thread([&]()->void{
            IFEngine engine(ctx, instructions, clock_handle);
            engine.Start();
        });
        /*ID*/
        auto* id_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::IDEngineRunnable);
            engine.Start();
        });
        /*EX*/
        auto* ex_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::EXEngineRunnable);
            engine.Start();
        });
        /*DM*/
        auto* dm_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::DMEngineRunnable);
            engine.Start();
        });
        /*WB*/
        auto* wb_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::WBEngineRunnable);
            engine.Start();
        });
        
        ctx.StartPrinterLoop(if_thread,
                             id_thread,
                             ex_thread,
                             dm_thread,
                             wb_thread);
        
        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif
    }
    
    bool verifyNumberOverflow(){
#ifndef NDEBUG
        
        Log::D(mName) << "Verifying: number overflow error" << std::endl;
        
        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();
        
        ctx.mInstrStartAddr = 0x00;
        ctx.Registers[1] = 0x80000000;
        ctx.Registers[3] = 0x80000000;
        
        Instructions instructions;
        
        /* add $2, $1, $3 */
        const byte_t raw_add_instr[4] = { 0x00, 0x23, 0x10, 0x20 };
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        
        /* halt x5 */
        const byte_t raw_halt_instr[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        Instruction halt_instr(raw_halt_instr);
        for(int i = 0; i < 5; i++){
            instructions.mInstructions.push_back(halt_instr);
        }
        
        ctx.setInstructionCount((uint32_t)instructions.length());
        
        TaskHandle::ClockHandle::Barrier rising_barrier(5);
        TaskHandle::ClockHandle::Barrier falling_barrier(5);
        TaskHandle::ClockHandle clock_handle(rising_barrier, falling_barrier);
        
        /*IF*/
        auto* if_thread = group.create_thread([&]()->void{
            IFEngine engine(ctx, instructions, clock_handle);
            engine.Start();
        });
        /*ID*/
        auto* id_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::IDEngineRunnable);
            engine.Start();
        });
        /*EX*/
        auto* ex_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::EXEngineRunnable);
            engine.Start();
        });
        /*DM*/
        auto* dm_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::DMEngineRunnable);
            engine.Start();
        });
        /*WB*/
        auto* wb_thread = group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::WBEngineRunnable);
            engine.Start();
        });
        
        ctx.StartPrinterLoop(if_thread,
                             id_thread,
                             ex_thread,
                             dm_thread,
                             wb_thread);
        
        //Log::V(mName) << "$4 Value: " << (int)ctx.Registers[4] << std::endl;
        //Log::V(mName) << "$3 Value: " << (int)ctx.Registers[3] << std::endl;
        //Log::V(mName) << "$5 Value: " << (int)ctx.Registers[5] << std::endl;
        
        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif

    }

    bool doTest(){

        bool result = true;
        result &= verifyWrite2ZeroRegister();
        result &= verifyNumberOverflow();
        result &= verifyDataMisAlignment();

        return result;
    }
};

#endif //ARCHIHW1_TESTTASKSERROR_H

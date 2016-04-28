
#ifndef ARCHIHW1_TESTTASKS_H
#define ARCHIHW1_TESTTASKS_H

#include "TestCase.h"
#include "../Context.h"
#include "../Task.h"
#include "../tasks/TaskHandle.h"
#include "../ExecutionEngine.h"
#include "../IFEngine.h"

#include <boost/thread/thread.hpp>

class TestTasks : public TestCase {

public:
    TestTasks() :
            TestCase("TestTasks") {}

private:

    bool testSingleThread(){
#ifndef NDEBUG
        Log::D(mName) << "Testing single task on single thread..." << std::endl;

        Context ctx(std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();

        ctx.Registers[1] = 0xA;
        ctx.Registers[2] = 0xB;

        //Instructions instructions;
        /* add $4, $1, $2 */
        const byte_t raw_add_instr[4] = { 0x00, 0x22, 0x20, 0x20 };
        Instruction add_instr(raw_add_instr);
        //instructions.mInstructions.push_back(add_instr);
        /* sub $3, $2, $1 */
        //const byte_t sub_instr[4] = { 0x00, 0x41, 0x18, 0x22 };

        /* halt */
        //const byte_t raw_halt_instr[4] = {0x3F, 0x00, 0x00, 0x00};
        //Instruction halt_instr(raw_halt_instr);
        //instructions.mInstructions.push_back(halt_instr);

        TaskHandle::ClockHandle::Barrier rising_barrier(1);
        TaskHandle::ClockHandle::Barrier falling_barrier(1);
        TaskHandle::ClockHandle clock_handle(rising_barrier, falling_barrier);
        auto* add_task = task::TasksTable[task::OP_ADD]
                .Get(&ctx, &add_instr, &clock_handle);

        AssertEqual(add_task->name, std::string("add"), "Test task");

        auto err = add_task->DoIF();
        //Log::V(mName) << "IF error: " << err << std::endl;
        AssertEqual(err, Error::NONE, "IF return result");
        AssertEqual((int)add_task->RdIndex, 4, "Rd Index");
        AssertEqual((int)add_task->RsIndex, 1, "Rs Index");
        AssertEqual((int)add_task->RtIndex, 2, "Rt Index");

        err = add_task->DoID();
        //Log::V(mName) << "ID error: " << err << std::endl;
        AssertEqual(err, Error::NONE, "ID return result");
        AssertEqual((int)add_task->RsValue, 0xA, "Rs Value");
        AssertEqual((int)add_task->RtValue, 0xB, "Rt Value");
        //Log::V(mName) << "Had reserve Rd: " << (ctx.RegReserves[add_task->RdIndex].load() == add_task) << std::endl;

        err = add_task->DoEX();
        //Log::V(mName) << "EX error: " << err << std::endl;
        AssertEqual(err, Error::NONE, "EX return result");
        AssertEqual((int)add_task->RdValue, 0x15, "Rd Value");

        err = add_task->DoDM();
        //Log::V(mName) << "DM error: " << err << std::endl;
        AssertEqual(err, Error::NONE, "DM return result");
        AssertEqual(ctx.FWD_ID_EXE.Available.load(), true, "Forward storage available");
        AssertEqual((int)ctx.FWD_ID_EXE.RegId, 4, "Forward register index");

        delete add_task;

        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif
    }

    bool verifyValMultiThreadNoStall(){
#ifndef NDEBUG

        Log::D(mName) << "Verifying value: two tasks with no stalling..." << std::endl;

        Context ctx(std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();

        ctx.mInstrStartAddr = 0x00;
        ctx.Registers[1] = 0xA;
        ctx.Registers[2] = 0xC;

        Instructions instructions;
        /* add $4, $1, $2 */
        const byte_t raw_add_instr[4] = { 0x00, 0x22, 0x20, 0x20 };
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        /* sub $3, $2, $1 */
        const byte_t raw_sub_instr[4] = { 0x00, 0x41, 0x18, 0x22 };
        Instruction sub_instr(raw_sub_instr);
        instructions.mInstructions.push_back(sub_instr);
        /* halt */
        const byte_t raw_halt_instr[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        Instruction halt_instr(raw_halt_instr);
        instructions.mInstructions.push_back(halt_instr);

        ctx.setInstructionCount((uint32_t)instructions.length());

        TaskHandle::ClockHandle::Barrier rising_barrier(5);
        TaskHandle::ClockHandle::Barrier falling_barrier(5);
        TaskHandle::ClockHandle clock_handle(rising_barrier, falling_barrier);

        boost::thread_group group;
        /*IF*/
        group.create_thread([&]()->void{
            IFEngine engine(ctx, instructions, clock_handle);
            engine.Start();
        });
        /*ID*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::IDEngineRunnable);
            engine.Start();
        });
        /*EX*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::EXEngineRunnable);
            engine.Start();
        });
        /*DM*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::DMEngineRunnable);
            engine.Start();
        });
        /*WB*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::WBEngineRunnable);
            engine.Start();
        });

        group.join_all();

        AssertEqual((int)ctx.Registers[4], 22, "$4 Value");
        AssertEqual((int)ctx.Registers[3], 2, "$3 Value");

        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif
    }

    /*
     * Two instructions
     * One inserted NOP
     * */
    bool verifyValMultiThreadStall1(){
#ifndef NDEBUG

        Log::D(mName) << "Verifying value: two tasks with stalling..." << std::endl;

        Context ctx(std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();

        ctx.mInstrStartAddr = 0x00;
        ctx.Registers[1] = 0xA;
        ctx.Registers[2] = 0xC;

        Instructions instructions;
        /* add $4, $1, $2 */
        const byte_t raw_add_instr[4] = { 0x00, 0x22, 0x20, 0x20 };
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        /* sub $3, $4, $1 */
        const byte_t raw_sub_instr[4] = { 0x00, 0x81, 0x18, 0x22 };
        Instruction sub_instr(raw_sub_instr);
        instructions.mInstructions.push_back(sub_instr);
        /* halt */
        const byte_t raw_halt_instr[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        Instruction halt_instr(raw_halt_instr);
        instructions.mInstructions.push_back(halt_instr);

        ctx.setInstructionCount((uint32_t)instructions.length());

        TaskHandle::ClockHandle::Barrier rising_barrier(5);
        TaskHandle::ClockHandle::Barrier falling_barrier(5);
        TaskHandle::ClockHandle clock_handle(rising_barrier, falling_barrier);

        boost::thread_group group;
        /*IF*/
        group.create_thread([&]()->void{
            IFEngine engine(ctx, instructions, clock_handle);
            engine.Start();
        });
        /*ID*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::IDEngineRunnable);
            engine.Start();
        });
        /*EX*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::EXEngineRunnable);
            engine.Start();
        });
        /*DM*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::DMEngineRunnable);
            engine.Start();
        });
        /*WB*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::WBEngineRunnable);
            engine.Start();
        });

        group.join_all();

        //Log::V(mName) << "$4 Value: " << (int)ctx.Registers[4] << std::endl;
        //Log::V(mName) << "$3 Value: " << (int)ctx.Registers[3] << std::endl;
        AssertEqual((int)ctx.Registers[4], 22, "$4 Value");
        AssertEqual((int)ctx.Registers[3], 0xC, "$3 Value");

        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif
    }

    /*
     * Three instructions
     * One inserted NOP
     * */
    bool verifyValMultiThreadStall2(){
#ifndef NDEBUG

        Log::D(mName) << "Verifying value: three tasks with stalling..." << std::endl;

        Context ctx(std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();

        ctx.mInstrStartAddr = 0x00;
        ctx.Registers[1] = 0xA;
        ctx.Registers[2] = 0xC;

        Instructions instructions;
        /* add $4, $1, $2 */
        const byte_t raw_add_instr[4] = { 0x00, 0x22, 0x20, 0x20 };
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        /* sub $3, $4, $1 */
        const byte_t raw_sub_instr[4] = { 0x00, 0x81, 0x18, 0x22 };
        Instruction sub_instr(raw_sub_instr);
        instructions.mInstructions.push_back(sub_instr);
        /* add $5, $1, $2 */
        const byte_t raw_add_instr2[4] = { 0x00, 0x22, 0x28, 0x20 };
        Instruction add_instr2(raw_add_instr2);
        instructions.mInstructions.push_back(add_instr2);
        /* halt */
        const byte_t raw_halt_instr[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        Instruction halt_instr(raw_halt_instr);
        instructions.mInstructions.push_back(halt_instr);

        ctx.setInstructionCount((uint32_t)instructions.length());

        TaskHandle::ClockHandle::Barrier rising_barrier(5);
        TaskHandle::ClockHandle::Barrier falling_barrier(5);
        TaskHandle::ClockHandle clock_handle(rising_barrier, falling_barrier);

        boost::thread_group group;
        /*IF*/
        group.create_thread([&]()->void{
            IFEngine engine(ctx, instructions, clock_handle);
            engine.Start();
        });
        /*ID*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::IDEngineRunnable);
            engine.Start();
        });
        /*EX*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::EXEngineRunnable);
            engine.Start();
        });
        /*DM*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::DMEngineRunnable);
            engine.Start();
        });
        /*WB*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::WBEngineRunnable);
            engine.Start();
        });

        group.join_all();

        //Log::V(mName) << "$4 Value: " << (int)ctx.Registers[4] << std::endl;
        //Log::V(mName) << "$3 Value: " << (int)ctx.Registers[3] << std::endl;
        //Log::V(mName) << "$5 Value: " << (int)ctx.Registers[5] << std::endl;
        AssertEqual((int)ctx.Registers[4], 22, "$4 Value");
        AssertEqual((int)ctx.Registers[3], 0xC, "$3 Value");
        AssertEqual((int)ctx.Registers[5], 22, "$5 Value");

        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif
    }

    bool verifyPrinterMultiThreadNoStall(){
#ifndef NDEBUG

        Log::D(mName) << "Verifying printer: two tasks with no stalling..." << std::endl;

        Context ctx(std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();

        ctx.mInstrStartAddr = 0x00;
        ctx.Registers[1] = 0xA;
        ctx.Registers[2] = 0xC;

        Instructions instructions;
        /* add $4, $1, $2 */
        const byte_t raw_add_instr[4] = { 0x00, 0x22, 0x20, 0x20 };
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        /* sub $3, $2, $1 */
        const byte_t raw_sub_instr[4] = { 0x00, 0x41, 0x18, 0x22 };
        Instruction sub_instr(raw_sub_instr);
        instructions.mInstructions.push_back(sub_instr);
        /* halt */
        const byte_t raw_halt_instr[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        Instruction halt_instr(raw_halt_instr);
        instructions.mInstructions.push_back(halt_instr);

        ctx.setInstructionCount((uint32_t)instructions.length());

        TaskHandle::ClockHandle::Barrier rising_barrier(5);
        TaskHandle::ClockHandle::Barrier falling_barrier(5);
        TaskHandle::ClockHandle clock_handle(rising_barrier, falling_barrier);

        boost::thread_group group;
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

        AssertEqual((int)ctx.Registers[4], 22, "$4 Value");
        AssertEqual((int)ctx.Registers[3], 2, "$3 Value");

        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif
    }
    /*
     * Three instructions
     * One inserted NOP
     * */
    bool verifyPrinterMultiThreadStall(){
#ifndef NDEBUG

        Log::D(mName) << "Verifying printer: three tasks with stalling..." << std::endl;

        Context ctx(std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();

        ctx.mInstrStartAddr = 0x00;
        ctx.Registers[1] = 0xA;
        ctx.Registers[2] = 0xC;

        Instructions instructions;
        /* add $4, $1, $2 */
        const byte_t raw_add_instr[4] = { 0x00, 0x22, 0x20, 0x20 };
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        /* sub $3, $4, $1 */
        const byte_t raw_sub_instr[4] = { 0x00, 0x81, 0x18, 0x22 };
        Instruction sub_instr(raw_sub_instr);
        instructions.mInstructions.push_back(sub_instr);
        /* add $5, $1, $2 */
        const byte_t raw_add_instr2[4] = { 0x00, 0x22, 0x28, 0x20 };
        Instruction add_instr2(raw_add_instr2);
        instructions.mInstructions.push_back(add_instr2);
        /* halt */
        const byte_t raw_halt_instr[4] = {0xFF, 0xFF, 0xFF, 0xFF};
        Instruction halt_instr(raw_halt_instr);
        instructions.mInstructions.push_back(halt_instr);

        ctx.setInstructionCount((uint32_t)instructions.length());

        TaskHandle::ClockHandle::Barrier rising_barrier(5);
        TaskHandle::ClockHandle::Barrier falling_barrier(5);
        TaskHandle::ClockHandle clock_handle(rising_barrier, falling_barrier);

        boost::thread_group group;
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
        AssertEqual((int)ctx.Registers[4], 22, "$4 Value");
        AssertEqual((int)ctx.Registers[3], 0xC, "$3 Value");
        AssertEqual((int)ctx.Registers[5], 22, "$5 Value");

        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif
    }

    bool doTest(){

        bool result = true;

        result &= testSingleThread();
        result &= verifyValMultiThreadNoStall();
        result &= verifyValMultiThreadStall1();
        result &= verifyValMultiThreadStall2();
        result &= verifyPrinterMultiThreadNoStall();
        result &= verifyPrinterMultiThreadStall();

        return result;
    }
};

#endif //ARCHIHW1_TESTTASKS_H

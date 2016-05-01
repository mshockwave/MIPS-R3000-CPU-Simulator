
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

        Context ctx(nullptr, std::cout, std::cout);
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

        AssertEqual(add_task->name, std::string("ADD"), "Test task");

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

        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
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

        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
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

        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
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

        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
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

        ctx.StartPrinterLoop(&group);

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

        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
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

        ctx.StartPrinterLoop(&group);

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

    /*
     * Branch instruction
     * Flush but no stall
     * */
    bool verifyPrinterBranchFlushNoStall(){
#ifndef NDEBUG

        Log::D(mName) << "Verifying printer: branch instruction flush without stalling..." << std::endl;

        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();

        ctx.mInstrStartAddr = 0x00;
        ctx.Registers[1] = 0xA;
        ctx.Registers[4] = 0xA;

        Instructions instructions;
        /* add $2, $1, $4 */
        const byte_t raw_add_instr[4] = { 0x00, 0x24, 0x10, 0x20 };
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        /* beq $1, $4, here */
        const byte_t raw_beq_instr[4] = { 0x10, 0x24, 0x00, 0x01 };
        Instruction beq_instr(raw_beq_instr);
        instructions.mInstructions.push_back(beq_instr);
        /* add $3, $4, $1 */
        const byte_t raw_add_instr2[4] = { 0x00, 0x81, 0x18, 0x20 };
        Instruction add_instr2(raw_add_instr2);
        instructions.mInstructions.push_back(add_instr2);
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

        ctx.StartPrinterLoop(&group);

        //Log::V(mName) << "$4 Value: " << (int)ctx.Registers[4] << std::endl;
        //Log::V(mName) << "$3 Value: " << (int)ctx.Registers[3] << std::endl;
        //Log::V(mName) << "$5 Value: " << (int)ctx.Registers[5] << std::endl;

        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif
    }

    /*
     * Branch instruction
     * With stalling
     * */
    bool verifyPrinterBranchFlushStall1(){
#ifndef NDEBUG

        Log::D(mName) << "Verifying printer: branch instruction flush and stall..." << std::endl;

        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();

        ctx.mInstrStartAddr = 0x00;
        //ctx.Registers[1] = 0xA;
        //ctx.Registers[4] = 0xA;

        Instructions instructions;
        /* add $2, $1, $4 */
        const byte_t raw_add_instr[4] = { 0x00, 0x24, 0x10, 0x20 };
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        /* beq $1, $2, here */
        const byte_t raw_beq_instr[4] = { 0x10, 0x22, 0x00, 0x01 };
        Instruction beq_instr(raw_beq_instr);
        instructions.mInstructions.push_back(beq_instr);
        /* add $3, $4, $1 */
        const byte_t raw_add_instr2[4] = { 0x00, 0x81, 0x18, 0x20 };
        Instruction add_instr2(raw_add_instr2);
        instructions.mInstructions.push_back(add_instr2);
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

        ctx.StartPrinterLoop(&group);

        //Log::V(mName) << "$4 Value: " << (int)ctx.Registers[4] << std::endl;
        //Log::V(mName) << "$3 Value: " << (int)ctx.Registers[3] << std::endl;
        //Log::V(mName) << "$5 Value: " << (int)ctx.Registers[5] << std::endl;

        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif
    }
    
    bool verifyValPrinterITasks(){
#ifndef NDEBUG
        
        Log::D(mName) << "Verifying values and printer: I type instructions" << std::endl;
        
        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();
        
        ctx.mInstrStartAddr = 0x00;
        ctx.mMemory[0x02] = 0x03;
        ctx.mMemory[0x03] = 0x04;
        ctx.mMemory[0x06] = 0x01;
        ctx.mMemory[0x07] = 0x02;
        
        Instructions instructions;
        /* addi $1, $2, 2 */
        const byte_t raw_add_instr[4] = { 0x20, 0x41, 0x00, 0x02 };
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        
        /* addi $1, $4, -1 */
        const byte_t raw_add_instr2[4] = { 0x20, 0x81, 0xFF, 0xFF };
        Instruction add_instr2(raw_add_instr2);
        instructions.mInstructions.push_back(add_instr2);
        
        /* addiu $3, $1, 4 */
        const byte_t raw_add_instr3[4] = { 0x24, 0x23, 0x00, 0x04 };
        Instruction add_instr3(raw_add_instr3);
        instructions.mInstructions.push_back(add_instr3);
        
        /* lw $5, 0($0) */
        const byte_t raw_lw_instr[4] = { 0x8C, 0x05, 0x00, 0x00 };
        Instruction lw_instr(raw_lw_instr);
        instructions.mInstructions.push_back(lw_instr);
        
        /* sw $5, 4($0) */
        const byte_t raw_sw_instr[4] = { 0xAC, 0x05, 0x00, 0x04 };
        Instruction sw_instr(raw_sw_instr);
        instructions.mInstructions.push_back(sw_instr);
        
        /* lw $6, 4($0) */
        const byte_t raw_lw_instr2[4] = { 0x8C, 0x06, 0x00, 0x04 };
        Instruction lw_instr2(raw_lw_instr2);
        instructions.mInstructions.push_back(lw_instr2);
        
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
        
        ctx.StartPrinterLoop(&group);
        //group.join_all();
        
        //AssertEqual((int)ctx.Registers[1], 2, "$1 Value");
        //AssertEqual((int)ctx.Registers[3], 1, "$3 Value");
        
        return true;
#else
        Log::E(mName) << "Need to test under Debug configuration" << std::endl;
        return false;
#endif

    }
    
    bool verifyValPrinterJTasks(){
#ifndef NDEBUG
        
        Log::D(mName) << "Verifying values and printer: J type instructions" << std::endl;
        
        boost::thread_group group;
        Context ctx(&group, std::cout, std::cout);
        task::InitInstructionMap();
        task::InitTasks();
        
        ctx.mInstrStartAddr = 0x00;
        
        
        Instructions instructions;
        /* addi $1, $2, 2 */
        const byte_t raw_add_instr[4] = { 0x20, 0x41, 0x00, 0x02 };
        Instruction add_instr(raw_add_instr);
        instructions.mInstructions.push_back(add_instr);
        
        /* j here */
        const byte_t raw_j_instr[4] = { 0x08, 0x00, 0x00, 0x03 };
        Instruction j_instr(raw_j_instr);
        instructions.mInstructions.push_back(j_instr);
        
        /* addi $2, $1, 2 */
        const byte_t raw_add_instr2[4] = { 0x20, 0x22, 0x00, 0x02 };
        Instruction add_instr2(raw_add_instr2);
        instructions.mInstructions.push_back(add_instr2);
        
        /* here: addi $3, $2, 3 */
        const byte_t raw_add_instr3[4] = { 0x20, 0x43, 0x00, 0x03 };
        Instruction add_instr3(raw_add_instr3);
        instructions.mInstructions.push_back(add_instr3);
        
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
        
        ctx.StartPrinterLoop(&group);
        //group.join_all();
        
        //AssertEqual((int)ctx.Registers[1], 2, "$1 Value");
        //AssertEqual((int)ctx.Registers[3], 1, "$3 Value");
        
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
        result &= verifyValPrinterITasks();
        result &= verifyValPrinterJTasks();
        result &= verifyPrinterMultiThreadNoStall();
        result &= verifyPrinterMultiThreadStall();
        result &= verifyPrinterBranchFlushNoStall();
        result &= verifyPrinterBranchFlushStall1();
        

        return result;
    }
};

#endif //ARCHIHW1_TESTTASKS_H

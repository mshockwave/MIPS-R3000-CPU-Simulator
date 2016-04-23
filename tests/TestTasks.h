
#ifndef ARCHIHW1_TESTTASKS_H
#define ARCHIHW1_TESTTASKS_H

#include "TestCase.h"
#include "../Context.h"
#include "../Task.h"
#include "../tasks/TaskHandle.h"

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

    bool doTest(){

        bool result = true;

        result &= testSingleThread();

        return result;
    }
};

#endif //ARCHIHW1_TESTTASKS_H

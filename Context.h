
#ifndef ARCHIHW1_CONTEXT_H
#define ARCHIHW1_CONTEXT_H

#include <iomanip>

#include <queue>
#include <thread>
#include <mutex>
#include <string>

#include "Types.h"
#include "RawBinary.h"
#include "Utils.h"
#include "Instruction.h"

#include "adts/FlipFlop.h"

struct RegisterTuple {
    reg_t Rd, Rs, Rt;
    uint8_t Rd_Index, Rs_Index, Rt_Index;

    RegisterTuple() :
            Rd_Index(REGISTER_DONT_CARE), Rd(U32_0),
            Rs_Index(REGISTER_DONT_CARE), Rs(U32_0),
            Rt_Index(REGISTER_DONT_CARE), Rt(U32_0){}
};
struct StageStorage {
    FlipFlop<RegisterTuple> Regs;
    FlipFlop<task_id_t> Task;

    void operator=(const StageStorage& rhs){
        Regs = rhs.Regs.GetCurrent();
        Task = rhs.Task.GetCurrent();
    }
};
struct StageRegisters {
    StageStorage EXE, DM, WB;
    FlipFlop<Instruction*> Instr;
    bool Visited;

    StageRegisters() :
            Visited(false),
            Instr(nullptr){}
};

class Context {

#define CHECK_MEMORY_BOUND(offset) \
    if((offset) > mDataSize && (offset) < SP.GetCurrent())

public:
    typedef unsigned long CounterType;
    typedef std::ostream OutputStream;

private:

    uint32_t mInstrCount;
    addr_t mInstrStartAddr;
    addr_t mInstrEndAddr;

    //Program counter
    reg_t PC;

    //Memory
    addr_t mDataSize;
    byte_t mMemory[MEMORY_LENGTH];

    CounterType mCycleCounter;

    //Output streams
    OutputStream &mSnapShotStream, &mErrorStream;

    void loadMemory(RawBinary& rawBinary);

public:

    addr_t getInstrEndAddr(){ return mInstrEndAddr; }

    //Global Registers
    FlipFlop<reg_t> Registers[REGISTER_COUNT];
    //Global Special registers
    FlipFlop<reg_t> &ZERO, &AT, &SP, &FP, &RA;

    void tickAllRegisters();

    //(Pipeline)Stage Registers
    StageRegisters IF_ID, ID_EXE, EXE_DM, DM_WB;

    Context(RawBinary& rawBinary,
            OutputStream& snapshotStream, OutputStream& errorStream) :
            /*Registers*/
            PC(0),
            ZERO(Registers[0]),
            AT(Registers[1]),
            SP(Registers[29]),
            FP(Registers[30]),
            RA(Registers[31]),
            /*Memory*/
            mDataSize(0),
            /*Cycle counter*/
            mCycleCounter(0),
            /*Streams*/
            mSnapShotStream(snapshotStream), mErrorStream(errorStream),
            mInstrCount(0), mInstrEndAddr(0){
        //Zero registers
        for(int i = 0; i < REGISTER_COUNT; i++){
            Registers[i] = (byte_t)0;
        }

        //Load PC from rawBinary
        FlipFlop<reg_t> pc_stub(0);
        load2Register(rawBinary.getInstructions(), pc_stub);
        pc_stub.tick();
        PC = pc_stub.GetCurrent();
        mInstrStartAddr = static_cast<addr_t>(PC);

        //Zero memory
        for(int i = 0; i < MEMORY_LENGTH; i++){
            mMemory[i] = (byte_t)0;
        }
        loadMemory(rawBinary);

        tickAllRegisters();
    }

    addr_t GetInstrStartAddress() const{ return mInstrStartAddr; }

    void setInstructionCount(uint32_t num){
        mInstrCount = num;

        //Evaluate end instruction address
        mInstrEndAddr = mInstrStartAddr + (mInstrCount - 1) * WORD_WIDTH;
    }

    //PC operations
    const reg_t& GetPC(){ return PC; }
    Error setPC(reg_t pc){
        Error e = Error::NONE;

        if(pc % WORD_WIDTH != 0)
            e = e + Error::DATA_MISALIGNED;

        if(pc > getInstrEndAddr())
            e = e + Error::MEMORY_ADDR_OVERFLOW;

        if( !(e == Error::NONE) ) return e;

        PC = pc;

        return Error::NONE;
    }
    inline void AdvancePC(){
        //TODO: Check bound
        PC += WORD_WIDTH;
    }

    //mMemory operations
    //const byte_t* getMemoryR() { return const_cast<const byte_t*>(mMemory); }
    word_t& getMemoryWord(addr_t offset){
        Error e = Error::NONE;

        //Check alignment
        if(offset % WORD_WIDTH != 0) e = e + Error::DATA_MISALIGNED;
        //Check boundary
        CHECK_MEMORY_BOUND(offset) e = e + Error::MEMORY_ADDR_OVERFLOW;

        if(!(e == Error::NONE)) throw e;

        return *((word_t*)(mMemory + offset));
    }
    half_w_t& getMemoryHalfWord(addr_t offset){
        Error e = Error::NONE;

        //Check alignment
        if(offset % (WORD_WIDTH >> 1) != 0) e = e + Error::DATA_MISALIGNED;
        //Check boundary
        CHECK_MEMORY_BOUND(offset) e = e + Error::MEMORY_ADDR_OVERFLOW;

        if(!(e == Error::NONE)) throw e;

        return *((half_w_t*)(mMemory + offset));
    }
    byte_t& getMemoryByte(addr_t offset){
        //Check boundary
        CHECK_MEMORY_BOUND(offset) throw Error::MEMORY_ADDR_OVERFLOW;

        return *((byte_t*)(mMemory + offset));
    }

    CounterType IncCycleCounter(){ return ++mCycleCounter; }
    CounterType GetCycleCounter(){ return mCycleCounter; }

    /*
     * Append current cycle's snapshot
     * */
    void DumpSnapshot();

    /*
     * Append error
     * */
    void PutError(Error &error);

};

#endif //ARCHIHW1_CONTEXT_H

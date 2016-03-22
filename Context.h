
#ifndef ARCHIHW1_CONTEXT_H
#define ARCHIHW1_CONTEXT_H

#include <iomanip>

#include "Types.h"
#include "RawBinary.h"
#include "Utils.h"

class Context {

public:
    typedef unsigned long CounterType;
    typedef std::ostream OutputStream;

private:

    uint32_t mInstrCount;
    addr_t mInstrEndAddr;

    //Program counter
    reg_t PC;

    //mMemory
    byte_t mMemory[MEMORY_LENGTH];

    CounterType mCycleCounter;

    //Output streams
    OutputStream &mSnapShotStream, &mErrorStream;

    void loadMemory(RawBinary& rawBinary);

public:

    static const addr_t INSTR_START_ADDR;
    addr_t getInstrEndAddr(){ return mInstrEndAddr; }

    //Registers
    reg_t Registers[REGISTER_COUNT];
    //Special registers
    reg_t &ZERO, &AT, &SP, &FP, &RA;

    Context(RawBinary& rawBinary,
            OutputStream& snapshotStream, OutputStream& errorStream) :
            /*Registers*/
            ZERO(Registers[0]),
            AT(Registers[1]),
            SP(Registers[29]),
            FP(Registers[30]),
            RA(Registers[31]),
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
        load2Register(rawBinary.getInstructions(), PC);

        //Zero memory
        for(int i = 0; i < MEMORY_LENGTH; i++){
            mMemory[i] = (byte_t)0;
        }
        loadMemory(rawBinary);
    }

    void setInstructionCount(uint32_t num){
        mInstrCount = num;

        //Evaluate end instruction address
        mInstrEndAddr = INSTR_START_ADDR + (mInstrCount - 1) * WORD_WIDTH;
    }

    //PC operations
    const reg_t& getPC(){ return PC; }
    Error& setPC(reg_t pc){

        if(pc % WORD_WIDTH != 0) return Error::DATA_MISALIGNED;
        if(pc > getInstrEndAddr()) return Error::MEMORY_ADDR_OVERFLOW;

        PC = pc;

        return Error::NONE;
    }
    inline void advancePC(){
        //TODO: Check bound
        PC += WORD_WIDTH;
    }

    //mMemory operations
    //const byte_t* getMemoryR() { return const_cast<const byte_t*>(mMemory); }
    word_t& getMemoryWord(addr_t offset){
        //Check alignment
        if(offset % WORD_WIDTH != 0) throw Error::DATA_MISALIGNED;
        //Check boundary
        if(offset > MEMORY_LENGTH) throw Error::MEMORY_ADDR_OVERFLOW;

        return *((word_t*)(mMemory + offset));
    }
    half_w_t& getMemoryHalfWord(addr_t offset){
        //Check alignment
        if(offset % WORD_WIDTH != 0) throw Error::DATA_MISALIGNED;
        //Check boundary
        if(offset > MEMORY_LENGTH) throw Error::MEMORY_ADDR_OVERFLOW;

        return *((half_w_t*)(mMemory + offset));
    }
    byte_t& getMemoryByte(addr_t offset){
        //Check alignment
        if(offset % WORD_WIDTH != 0) throw Error::DATA_MISALIGNED;
        //Check boundary
        if(offset > MEMORY_LENGTH) throw Error::MEMORY_ADDR_OVERFLOW;

        return *((byte_t*)(mMemory + offset));
    }

    CounterType incCycleCounter(){ return ++mCycleCounter; }
    CounterType getCycleCounter(){ return mCycleCounter; }

    /*
     * Append current cycle's snapshot
     * */
    void dumpSnapshot();

    /*
     * Append error
     * */
    void putError(Error& error);

};

#endif //ARCHIHW1_CONTEXT_H

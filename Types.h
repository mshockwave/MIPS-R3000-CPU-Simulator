
#ifndef ARCHIHW1_TYPES_H
#define ARCHIHW1_TYPES_H

#include <cstdint>
#include <functional>
#include <string>
#include <ostream>

typedef uint8_t byte_t;

const uint8_t REGISTER_COUNT = 32;
typedef uint32_t reg_t;
typedef uint32_t word_t;
typedef uint16_t half_w_t;

typedef uint32_t addr_t;

const uint32_t WORD_WIDTH = 4;
const uint32_t MEMORY_LENGTH = (1 << 10);

class Error {
private:
    uint8_t mErrorId;
    std::string mDescription;

    //0 for the default level, would continue
    //1 for the severe level, halt
    uint8_t mErrorLevel;

protected:
    Error(uint8_t id, uint8_t level, std::string description) :
            mErrorId(id),
            mDescription(description),
            mErrorLevel(level) {}
    Error(uint8_t id, std::string description) :
            Error(id, LEVEL_CONTINUE, description){}

public:
    static const uint8_t LEVEL_CONTINUE;
    static const uint8_t LEVEL_HALT;

    static Error NONE; //Place holder
    static Error WRITE_REG_ZERO;
    static Error NUMBER_OVERFLOW;
    static Error MEMORY_ADDR_OVERFLOW;
    static Error DATA_MISALIGNED;

    bool operator==(const Error& rhs){
        return this->mErrorId == rhs.mErrorId;
    }
    bool operator==(Error& rhs){
        return this->mErrorId == rhs.mErrorId;
    }

    friend std::ostream& operator<<(std::ostream& os, const Error& error);
};

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

public:

    static const addr_t INSTR_START_ADDR;
    addr_t getInstrEndAddr(){ return mInstrEndAddr; }

    //Registers
    reg_t Registers[REGISTER_COUNT];
    //Special registers
    reg_t &ZERO, &AT, &SP, &FP, &RA;

    Context(reg_t pc, OutputStream& snapshotStream, OutputStream& errorStream) :
            /*Registers*/
            PC(pc),
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

        //Zero memory
        for(int i = 0; i < MEMORY_LENGTH; i++){
            mMemory[i] = (byte_t)0;
        }
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

typedef uint32_t task_id_t;

const uint8_t U8_1 = (uint8_t)1;
const uint32_t U32_1 = (uint32_t)1;
const uint32_t U32_0 = (uint32_t)0;

#endif //ARCHIHW1_TYPES_H

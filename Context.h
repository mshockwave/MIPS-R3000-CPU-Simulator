
#ifndef ARCHIHW1_CONTEXT_H
#define ARCHIHW1_CONTEXT_H

extern "C"{
#include <unistd.h>
}

#include <iomanip>
#include <memory>

#include "Types.h"
#include "RawBinary.h"
#include "Utils.h"
#include "CMP.hpp"

class Context {

#define CHECK_MEMORY_BOUND(offset, size) \
    if((offset) + (size) > MEMORY_LENGTH)

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
    //std::shared_ptr<cmp::CMP> MemoryCMP;
    std::unique_ptr<cmp::CMP> MemoryCMP;

    CounterType mCycleCounter;

    //Output streams
    OutputStream &mSnapShotStream, &mErrorStream;

    void loadMemory(RawBinary& rawBinary);

public:

    addr_t getInstrEndAddr(){ return mInstrEndAddr; }

    //Registers
    reg_t Registers[REGISTER_COUNT];
    //Special registers
    reg_t &ZERO, &AT, &SP, &FP, &RA;

    Context(RawBinary& rawBinary,
            cmp::CMP::cmp_config_t mem_cmp_config,
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
        load2Register(rawBinary.getInstructions(), PC);
        mInstrStartAddr = static_cast<addr_t>(PC);

        //Zero memory
        for(int i = 0; i < MEMORY_LENGTH; i++){
            mMemory[i] = (byte_t)0;
        }
        loadMemory(rawBinary);
                
        // Init CMP
        /*
        MemoryCMP = std::make_shared<cmp::CMP>(mem_cmp_config,
                                               0,
                                               mMemory,
                                               MEMORY_LENGTH);
         */
        std::unique_ptr<cmp::CMP> mem_cmp(new cmp::CMP(mem_cmp_config,
                                                       0,
                                                       mMemory,
                                                       MEMORY_LENGTH));
        MemoryCMP = std::move(mem_cmp);
        
    }

    //addr_t GetInstrStartAddress() const{ return mInstrStartAddr; }
    addr_t GetInstrStartAddress() const{ return 0; }

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
        //Error e = Error::NONE;

        //Check alignment
        //if(offset % WORD_WIDTH != 0) e = e + Error::DATA_MISALIGNED;
        //Check boundary
        //CHECK_MEMORY_BOUND(offset, 4) e = e + Error::MEMORY_ADDR_OVERFLOW;

        //if(!(e == Error::NONE)) throw e;

        byte_t* mem_ptr = MemoryCMP->Access(offset);
        
        return *((word_t*)mem_ptr);
    }
    half_w_t& getMemoryHalfWord(addr_t offset){
        //Error e = Error::NONE;

        //Check alignment
        //if(offset % (WORD_WIDTH >> 1) != 0) e = e + Error::DATA_MISALIGNED;
        //Check boundary
        //CHECK_MEMORY_BOUND(offset, 2) e = e + Error::MEMORY_ADDR_OVERFLOW;

        //if(!(e == Error::NONE)) throw e;
        
        byte_t* mem_ptr = MemoryCMP->Access(offset);

        return *((half_w_t*)mem_ptr);
    }
    byte_t& getMemoryByte(addr_t offset){
        //Check boundary
        //CHECK_MEMORY_BOUND(offset, 1) throw Error::MEMORY_ADDR_OVERFLOW;
        
        byte_t* mem_ptr = MemoryCMP->Access(offset);

        return *((byte_t*)mem_ptr);
    }

    CounterType IncCycleCounter(){
        if((mCycleCounter++) >= 500000){
            Log::E("Context") << "Over 500000 cycle, abort" << std::endl;
            ::exit(1);
        }
        
        return mCycleCounter;
    }
    CounterType GetCycleCounter(){ return mCycleCounter; }

    /*
     * Append current cycle's snapshot
     * */
    void DumpSnapshot();

    /*
     * Append error
     * */
    void putError(Error& error);
    
    inline cmp::CMP::profile_result_t GetCacheProfileData() const{
        return MemoryCMP->GetCacheProfileData();
    }
    inline cmp::CMP::profile_result_t GetTLBProfileData() const{
        return MemoryCMP->GetTLBProfileData();
    }
    inline cmp::CMP::profile_result_t GetPageProfileData() const{
        return MemoryCMP->GetPageProfileData();
    }

};

#endif //ARCHIHW1_CONTEXT_H

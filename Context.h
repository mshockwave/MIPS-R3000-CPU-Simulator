
#ifndef ARCHIHW1_CONTEXT_H
#define ARCHIHW1_CONTEXT_H

#include <iomanip>

#include <queue>
#include <vector>
#include <thread>
#include <mutex>

#include <boost/atomic/atomic.hpp>

#include "Types.h"
#include "RawBinary.h"
#include "Utils.h"
#include "adts/BlockingQueue.h"

class Context {

#ifndef NDEBUG
    friend class TestTasks;
#endif

#define CHECK_MEMORY_BOUND(offset) \
    if((offset) > mDataSize && (offset) < SP)

public:
    typedef unsigned long CounterType;
    typedef std::ostream OutputStream;

    typedef std::vector<TaskHandle*> StageRegister;

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
    reg_t Registers[REGISTER_COUNT];
    //Global Special registers
    reg_t &ZERO, &AT, &SP, &FP, &RA;

    typedef boost::atomic<TaskHandle*> reg_reserve_t;
    reg_reserve_t RegReserves[REGISTER_COUNT];

    const unsigned int STAGE_REG_BUF_SIZE = 2;
    StageRegister IF_ID, ID_EX, EX_DM, DM_WB;
    inline bool pushTask(StageRegister& stage, TaskHandle* task){
        if(stage.size() < STAGE_REG_BUF_SIZE){
            stage.push_back(task);
            return true;
        }else{
            return false;
        }
    }

    struct ForwardStorage{
        uint8_t RegId;
        reg_t RegValue;
        boost::atomic<bool> Available;

        ForwardStorage() :
                RegId(0),
                RegValue(0),
                Available(false) {}
    };
    ForwardStorage FWD_ID_EXE;

#ifndef NDEBUG
    /*
     * For Unit Test Only!
     * */
    Context(OutputStream& snapshotStream, OutputStream& errorStream) :
            /*Registers*/
            PC(0),
            PcJump(false),
            PcFlush(false),
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
            RegReserves[i] = nullptr;
        }

        //Zero memory
        for(int i = 0; i < MEMORY_LENGTH; i++){
            mMemory[i] = (byte_t)0;
        }
    }
#endif

    Context(RawBinary& rawBinary,
            OutputStream& snapshotStream, OutputStream& errorStream) :
            /*Registers*/
            PC(0),
            PcJump(false),
            PcFlush(false),
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
            RegReserves[i] = nullptr;
        }

        //Load PC from rawBinary
        load2Register(rawBinary.getInstructions(), PC);
        mInstrStartAddr = static_cast<addr_t>(PC);

        //Zero memory
        for(int i = 0; i < MEMORY_LENGTH; i++){
            mMemory[i] = (byte_t)0;
        }
        loadMemory(rawBinary);
    }

    addr_t GetInstrStartAddress() const{ return mInstrStartAddr; }

    void setInstructionCount(uint32_t num){
        mInstrCount = num;

        //Evaluate end instruction address
        mInstrEndAddr = mInstrStartAddr + (mInstrCount - 1) * WORD_WIDTH;
    }

    //PC operations
    bool PcJump;
    bool PcFlush;
    const reg_t& GetPC(){ return PC; }
    Error SetPC(reg_t pc){
        Error e = Error::NONE;

        if(pc % WORD_WIDTH != 0)
            e = e + Error::DATA_MISALIGNED;

        if(pc > getInstrEndAddr())
            e = e + Error::MEMORY_ADDR_OVERFLOW;

        if( !(e == Error::NONE) ) return e;

        PC = pc;
        PcJump = true;

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
     * Output streams for stages
     * */
    BlockingQueue<std::string> IFMessageQueue,
            IDMessageQueue,
            EXMessageQueue,
            DMMessageQueue,
            WBMessageQueue;

    /*
     * Append error
     * */
    void putError(Error& error);

};

#endif //ARCHIHW1_CONTEXT_H


#ifndef ARCHIHW1_CONTEXT_H
#define ARCHIHW1_CONTEXT_H

#include <iomanip>

#include <queue>
#include <vector>
#include <thread>
#include <mutex>

#include <boost/atomic/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono/chrono.hpp>
#include <boost/thread/mutex.hpp>

#include "Types.h"
#include "RawBinary.h"
#include "Utils.h"
#include "adts/BlockingQueue.h"
#include "adts/ScopedReadWriteLock.h"

class Context {

#ifndef NDEBUG
    friend class TestTasks;
    friend class TestTasksError;
#endif

#define CHECK_MEMORY_BOUND(offset) \
    if((offset) > MEMORY_LENGTH)

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

    addr_t GetInstrEndAddr(){ return mInstrEndAddr; }

    //Global Registers
    reg_t Registers[REGISTER_COUNT];
    //Global Special registers
    reg_t &ZERO, &AT, &SP, &FP, &RA;

    struct RegReserve {
        TaskHandle* Holder;
        reg_t Value;

        bool ForbitReservation;
        bool IDAvailable, EXAvailable;
        bool EXForward, IDForward;

        RegReserve() :
                Holder(nullptr),
                Value(0),
                ForbitReservation(false),
                IDAvailable(false), EXAvailable(false),
                EXForward(false), IDForward(false){ }
        void Reset(TaskHandle* h){
            
            if(!ForbitReservation) Holder = h;
            
            Value = 0;
            IDAvailable = EXAvailable = false;
            EXForward = IDForward = false;
        }
    };
    
    RegReserve RegReserves[REGISTER_COUNT];

    const unsigned int STAGE_REG_BUF_SIZE = 1;
    StageRegister IF_ID, ID_EX, EX_DM, DM_WB;
    inline bool pushTask(StageRegister& stage, TaskHandle* task){
        if(stage.size() < STAGE_REG_BUF_SIZE){
            stage.push_back(task);
            return true;
        }else{
            return false;
        }
    }


#ifndef NDEBUG
    /*
     * For Unit Test Only!
     * */
    Context(boost::thread_group* threads,
            OutputStream& snapshotStream, OutputStream& errorStream) :
            /*Registers*/
            PC(0),
            PcJump(false),
            PcFlush(0),
            ZERO(Registers[0]),
            AT(Registers[1]),
            SP(Registers[29]),
            FP(Registers[30]),
            RA(Registers[31]),
            /*Memory*/
            mDataSize(0),
            /*Pipeline*/
            IFStall(false),
            /*Thread*/
            DeadThreadMux(),
            DeadThreadNum(0),
            ThreadGroup(threads),
            /*Cycle counter*/
            mCycleCounter(0),
            /*Streams*/
            mSnapShotStream(snapshotStream), mErrorStream(errorStream),
            mInstrCount(0), mInstrEndAddr(0){

        //Zero registers
        for(int i = 0; i < REGISTER_COUNT; i++){
            Registers[i] = (byte_t)0;
            RegReserves[i].Reset(nullptr);
        }
        RegReserves[0].ForbitReservation = true;

        //Zero memory
        for(int i = 0; i < MEMORY_LENGTH; i++){
            mMemory[i] = (byte_t)0;
        }
    }
#endif

    Context(RawBinary& rawBinary,
            boost::thread_group *threads,
            OutputStream& snapshotStream, OutputStream& errorStream) :
            /*Registers*/
            PC(0),
            PcJump(false),
            PcFlush(0),
            ZERO(Registers[0]),
            AT(Registers[1]),
            SP(Registers[29]),
            FP(Registers[30]),
            RA(Registers[31]),
            /*Memory*/
            mDataSize(0),
            /*Pipeline*/
            IFStall(false),
            /*Thread*/
            DeadThreadMux(),
            DeadThreadNum(0),
            ThreadGroup(threads),
            /*Cycle counter*/
            mCycleCounter(0),
            /*Streams*/
            mSnapShotStream(snapshotStream), mErrorStream(errorStream),
            mInstrCount(0), mInstrEndAddr(0){
        //Zero registers
        for(int i = 0; i < REGISTER_COUNT; i++){
            Registers[i] = (byte_t)0;
            RegReserves[i].Reset(nullptr);
        }
        RegReserves[0].ForbitReservation = true;

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

    //Pipeline operations
    bool IFStall;

    //Thread operations
    ScopedReadWriteLock::mutex_type DeadThreadMux;
    int DeadThreadNum;
    boost::thread_group* ThreadGroup;

    //PC operations
    bool PcJump;
    boost::atomic_int PcFlush;
    static const int PC_FLUSH_CONSUMER_COUNT;
    const reg_t& GetPC(){ return PC; }
    Error SetPC(reg_t pc){
        Error e = Error::NONE;

        if(pc % WORD_WIDTH != 0)
            e = e + Error::DATA_MISALIGNED;

        if(pc > GetInstrEndAddr())
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
    word_t& GetMemoryWord(addr_t offset){
        Error e = Error::NONE;

        //Check alignment
        if(offset % WORD_WIDTH != 0) e = e + Error::DATA_MISALIGNED;
        //Check boundary
        CHECK_MEMORY_BOUND(offset) e = e + Error::MEMORY_ADDR_OVERFLOW;

        if(!(e == Error::NONE)) throw e;

        return *((word_t*)(mMemory + offset));
    }
    half_w_t& GetMemoryHalfWord(addr_t offset){
        Error e = Error::NONE;

        //Check alignment
        if(offset % (WORD_WIDTH >> 1) != 0) e = e + Error::DATA_MISALIGNED;
        //Check boundary
        CHECK_MEMORY_BOUND(offset) e = e + Error::MEMORY_ADDR_OVERFLOW;

        if(!(e == Error::NONE)) throw e;

        return *((half_w_t*)(mMemory + offset));
    }
    byte_t& GetMemoryByte(addr_t offset){
        //Check boundary
        CHECK_MEMORY_BOUND(offset) throw Error::MEMORY_ADDR_OVERFLOW;

        return *((byte_t*)(mMemory + offset));
    }

    CounterType IncCycleCounter(){ return ++mCycleCounter; }
    CounterType GetCycleCounter(){ return mCycleCounter; }

    /*
     * Output queues for stages
     * */
    static const std::string MSG_END;
    typedef BlockingQueue<std::string> msg_queue_t;
    msg_queue_t IFMessageQueue,
            IDMessageQueue,
            EXMessageQueue,
            DMMessageQueue,
            WBMessageQueue;

    typedef BlockingQueue<Error> err_queue_t;
    err_queue_t EXErrorQueue,
            DMErrorQueue,
            WBErrorQueue;

    typedef BlockingQueue<RegsDiff> reg_queue_t;
    reg_queue_t RegsQueue;
    
    typedef BlockingQueue<reg_t> pc_queue_t;
    pc_queue_t PcQueue;

    void StartPrinterLoop(boost::thread_group*);

    /*
     * Append error
     * */
    bool PrintError();

};

#endif //ARCHIHW1_CONTEXT_H

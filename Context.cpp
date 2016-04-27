
#include "Context.h"

/*Context*/
const std::string Context::MSG_END("#END#");

void Context::loadMemory(RawBinary& rawBinary) {
    RawBinary::raw_container_t& dataImg = rawBinary.getDataImg();

    /*The first eight bytes are SP and data size, skip*/
    SP = U32_0;
    load2Register(dataImg, SP);

    uint32_t dataLength = 0;
    load2Register<4>(dataImg, dataLength);
    mDataSize = dataLength * WORD_WIDTH;

    int i, j, k;
    for(i = 0, k = 8; i < dataLength; i++, k += WORD_WIDTH){
        for(j = 0; j < WORD_WIDTH; j++){
            mMemory[k - 8 + j] = dataImg[k + j];
        }
    }
}

void Context::DumpRegisters() {

    for(int i = 0; i < REGISTER_COUNT; i++){
        mSnapShotStream << '$' << std::setfill('0') << std::setw(2) << std::dec << i;
        mSnapShotStream << ": 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << Registers[i] << std::endl;
    }
    mSnapShotStream << "PC: 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << PC << std::endl;

    //Epilogue
    //mSnapShotStream << std::endl << std::endl;
}

void Context::putError(Error &error) {

    //Check errors
    if(error.contains(Error::WRITE_REG_ZERO)){
        mErrorStream << "In cycle " << std::dec << mCycleCounter << ": ";
        mErrorStream << Error::WRITE_REG_ZERO << std::endl;
    }

    if(error.contains(Error::NUMBER_OVERFLOW)){
        mErrorStream << "In cycle " << std::dec << mCycleCounter << ": ";
        mErrorStream << Error::NUMBER_OVERFLOW << std::endl;
    }

    if(error.contains(Error::MEMORY_ADDR_OVERFLOW)){
        mErrorStream << "In cycle " << std::dec << mCycleCounter << ": ";
        mErrorStream << Error::MEMORY_ADDR_OVERFLOW << std::endl;
    }

    if(error.contains(Error::DATA_MISALIGNED)){
        mErrorStream << "In cycle " << std::dec << mCycleCounter << ": ";
        mErrorStream << Error::DATA_MISALIGNED << std::endl;
    }
}

void Context::StartPrinterLoop(boost::thread* if_thread,
                               boost::thread* id_thread,
                               boost::thread* ex_thread,
                               boost::thread* dm_thread,
                               boost::thread* wb_thread) {
    bool if_dead = false, id_dead = false,
            ex_dead = false, dm_dead = false, wb_dead = false;

    std::string if_msg, id_msg, ex_msg, dm_msg, wb_msg;

    while(!(if_dead && id_dead &&
            ex_dead && dm_dead && wb_dead)){

        std::string tmp;

        if(!if_dead){
            tmp = IFMessageQueue.Pop();
            if(tmp != MSG_END){
                if_msg = tmp;
            }else{
                if_dead |= true;
            }
        }
        {
#ifndef NDEBUG
            boost::mutex::scoped_lock lk(Log::Mux::D);
#endif
            mSnapShotStream << "IF: " << if_msg << std::endl;
        }

        if(!id_dead){
            tmp = IDMessageQueue.Pop();
            if(tmp != MSG_END){
                id_msg = tmp;
            }else{
                id_dead |= true;
            }
        }
        {
#ifndef NDEBUG
            boost::mutex::scoped_lock lk(Log::Mux::D);
#endif
            mSnapShotStream << "ID: " << id_msg << std::endl;
        }

        if(!ex_dead){
            tmp = EXMessageQueue.Pop();
            if(tmp != MSG_END){
                ex_msg = tmp;
            }else{
                ex_dead |= true;
            }
        }
        {
#ifndef NDEBUG
            boost::mutex::scoped_lock lk(Log::Mux::D);
#endif
            mSnapShotStream << "EX: " << ex_msg << std::endl;
        }

        if(!dm_dead){
            tmp = DMMessageQueue.Pop();
            if(tmp != MSG_END){
                dm_msg = tmp;
            }else{
                dm_dead |= true;
            }
        }
        {
#ifndef NDEBUG
            boost::mutex::scoped_lock lk(Log::Mux::D);
#endif
            mSnapShotStream << "DM: " << dm_msg << std::endl;
        }

        if(!wb_dead){
            tmp = WBMessageQueue.Pop();
            if(tmp != MSG_END){
                wb_msg = tmp;
            }else{
                wb_dead |= true;
            }
        }
        {
#ifndef NDEBUG
            boost::mutex::scoped_lock lk(Log::Mux::D);
#endif
            mSnapShotStream << "WB: " << wb_msg << std::endl << std::endl;
        }

        const boost::chrono::microseconds dur(0);
        if_dead |= if_thread->try_join_for(dur);
        id_dead |= id_thread->try_join_for(dur);
        ex_dead |= ex_thread->try_join_for(dur);
        dm_dead |= dm_thread->try_join_for(dur);
        wb_dead |= wb_thread->try_join_for(dur);
    }
}

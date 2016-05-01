
#include "Context.h"

/*Context*/
const std::string Context::MSG_END("#END#");
const int Context::PC_FLUSH_CONSUMER_COUNT = 2;

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

bool Context::PrintError() {

    //Check errors
    bool empty = true;
    Error error = Error::NONE;
    if(!EXErrorQueue.IsEmpty()){
        error = (error + EXErrorQueue.Pop());
        empty = false;
    }
    if(!DMErrorQueue.IsEmpty()){
        error = (error + DMErrorQueue.Pop());
        empty = false;
    }
    if(!WBErrorQueue.IsEmpty()){
        error = (error + WBErrorQueue.Pop());
        empty = false;
    }

    if(empty) return true;

    if(error.contains(Error::WRITE_REG_ZERO)){
        mErrorStream << "In cycle " << std::dec << error.GetCycle() << ": ";
        mErrorStream << Error::WRITE_REG_ZERO << std::endl;
    }

    if(error.contains(Error::MEMORY_ADDR_OVERFLOW)){
        mErrorStream << "In cycle " << std::dec << error.GetCycle() << ": ";
        mErrorStream << Error::MEMORY_ADDR_OVERFLOW << std::endl;
    }

    if(error.contains(Error::DATA_MISALIGNED)){
        mErrorStream << "In cycle " << std::dec << error.GetCycle() << ": ";
        mErrorStream << Error::DATA_MISALIGNED << std::endl;
    }

    if(error.contains(Error::NUMBER_OVERFLOW)){
        mErrorStream << "In cycle " << std::dec << error.GetCycle() << ": ";
        mErrorStream << Error::NUMBER_OVERFLOW << std::endl;
    }

    return false;
}

static void trimOutput(std::string& str){
    //Find first space
    auto index = str.find(' ');
    if(index != std::string::npos){
        str = str.substr(0, index);
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

    //Zero cycle
    mSnapShotStream << "cycle " << std::dec << mCycleCounter << std::endl;
    
    reg_t registers_clone[REGISTER_COUNT];
    int i;
    for(i = 0; i < REGISTER_COUNT; i++){
        registers_clone[i] = Registers[i];
        mSnapShotStream << '$' << std::setfill('0') << std::setw(2) << std::dec << i;
        mSnapShotStream << ": 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << registers_clone[i] << std::endl;
    }
    reg_t pc_;

    while(true){

        //Print errors
        PrintError();

        auto regs_diff = RegsQueue.Pop();

        //Update register differents
        if(regs_diff.RegIndex > 0){
            registers_clone[regs_diff.RegIndex] = regs_diff.RegValue;
        }
        pc_ = PcQueue.Pop();
        mSnapShotStream << "PC: 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << pc_ << std::endl;

        if(!if_dead){
            if_msg = IFMessageQueue.PopAndCheck(Context::MSG_END, &if_dead);
            if_dead |= (if_msg == Context::MSG_END);
        }else{
            trimOutput(if_msg);
        }
        {
#ifndef NDEBUG
            boost::mutex::scoped_lock lk(Log::Mux::D);
#endif
            if(if_msg != Context::MSG_END){
                mSnapShotStream << "IF: " << if_msg << std::endl;
            }
        }

        if(!id_dead){
            id_msg = IDMessageQueue.PopAndCheck(Context::MSG_END, &id_dead);
            id_dead |= (id_msg == Context::MSG_END);
        }else{
            trimOutput(id_msg);
        }
        {
#ifndef NDEBUG
            boost::mutex::scoped_lock lk(Log::Mux::D);
#endif
            if(id_msg != Context::MSG_END){
                mSnapShotStream << "ID: " << id_msg << std::endl;
            }
        }

        if(!ex_dead){
            ex_msg = EXMessageQueue.PopAndCheck(Context::MSG_END, &ex_dead);
            ex_dead |= (ex_msg == Context::MSG_END);
        }else{
            trimOutput(ex_msg);
        }
        {
#ifndef NDEBUG
            boost::mutex::scoped_lock lk(Log::Mux::D);
#endif
            if(ex_msg != Context::MSG_END){
                mSnapShotStream << "EX: " << ex_msg << std::endl;
            }
        }

        if(!dm_dead){
            dm_msg = DMMessageQueue.PopAndCheck(Context::MSG_END, &dm_dead);
            dm_dead |= (dm_msg == Context::MSG_END);
        }else{
            trimOutput(dm_msg);
        }
        {
#ifndef NDEBUG
            boost::mutex::scoped_lock lk(Log::Mux::D);
#endif
            if(dm_msg != Context::MSG_END){
                mSnapShotStream << "DM: " << dm_msg << std::endl;
            }
        }

        if(!wb_dead){
            wb_msg = WBMessageQueue.PopAndCheck(Context::MSG_END, &wb_dead);
            wb_dead |= (wb_msg == Context::MSG_END);
        }else{
            trimOutput(wb_msg);
        }
        {
#ifndef NDEBUG
            boost::mutex::scoped_lock lk(Log::Mux::D);
#endif
            if(wb_msg != Context::MSG_END){
                mSnapShotStream << "WB: " << wb_msg << std::endl << std::endl;
            }
        }

        if(regs_diff.Abort || (if_dead && id_dead &&
                               ex_dead && dm_dead && wb_dead)) break;
        
        IncCycleCounter();
        
        mSnapShotStream << "cycle " << std::dec << mCycleCounter << std::endl;
        
        for(i = 0; i < REGISTER_COUNT; i++){
            mSnapShotStream << '$' << std::setfill('0') << std::setw(2) << std::dec << i;
            mSnapShotStream << ": 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << registers_clone[i] << std::endl;
        }

    }

    //Clean rest of the errors
    while(!PrintError());

    /*
     * Remember to join before exit!!
     * */
    //FIXME: Use thread group instead
    if_thread->join();
    id_thread->join();
    ex_thread->join();
    dm_thread->join();
    wb_thread->join();
}


#include "Context.h"

/*Context*/
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

void Context::DumpSnapshot() {
    mSnapShotStream << "cycle " << std::dec << mCycleCounter << std::endl;

    for(int i = 0; i < REGISTER_COUNT; i++){
        mSnapShotStream << '$' << std::setfill('0') << std::setw(2) << std::dec << i;
        mSnapShotStream << ": 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << Registers[i] << std::endl;
    }
    mSnapShotStream << "PC: 0x" << OSTREAM_HEX_OUTPUT_FMT(8) << PC << std::endl;

    //Epilogue
    mSnapShotStream << std::endl << std::endl;
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

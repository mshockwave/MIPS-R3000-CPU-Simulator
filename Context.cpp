
#include "Context.h"

/*Context*/
void Context::loadMemory(RawBinary& rawBinary) {
    std::vector<byte_t>& dataImg = rawBinary.getDataImg();

    /*The first eight bytes are SP and data size, skip*/
    SP = U32_0;
    load2Register(dataImg, SP);

    uint32_t dataSize = 0;
    load2Register<4>(dataImg, dataSize);

    int i, j, k;
    for(i = 0, k = 8; i < dataSize; i ++, k += WORD_WIDTH){
        for(j = 0; j < WORD_WIDTH; j++){
            mMemory[k - 8 + j] = dataImg[k + (WORD_WIDTH - 1) - j];
        }
    }
}

void Context::dumpSnapshot() {
    mSnapShotStream << "cycle " << mCycleCounter << std::endl;

    for(int i = 0; i < REGISTER_COUNT; i++){
        mSnapShotStream << '$' << std::setfill('0') << std::setw(2) << i;
        mSnapShotStream << ": 0x" << std::setfill('0') << std::setw(8) << std::hex << Registers[i] << std::endl;
    }
    mSnapShotStream << "PC: 0x" << std::setfill('0') << std::setw(8) << std::hex << PC << std::endl;

    //Epilogue
    mSnapShotStream << std::endl << std::endl;
}

void Context::putError(Error &error) {
    mErrorStream << "In cycle " << mCycleCounter << ": ";
    mErrorStream << error << std::endl;
}

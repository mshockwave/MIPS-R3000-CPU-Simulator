
#include "Context.h"

/*Context*/
void Context::loadMemory(RawBinary& rawBinary) {
    std::vector<byte_t>& dataImg = rawBinary.getDataImg();

    /*The first four bytes are SP, skip*/
    SP = U32_0;
    load2Register(dataImg, SP);

    for(int i = 4; i < dataImg.size(); i += WORD_WIDTH){
        for(int j = 0; j < WORD_WIDTH; j++){
            mMemory[i + j] = dataImg[i + (WORD_WIDTH - 1) - j];
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

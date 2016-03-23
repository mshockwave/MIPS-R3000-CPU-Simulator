
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
    for(i = 0, k = 8; i < dataSize; i++, k += WORD_WIDTH){
        for(j = 0; j < WORD_WIDTH; j++){
            mMemory[k - 8 + j] = dataImg[k + j];
        }
    }
}

void Context::dumpSnapshot() {
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
    mErrorStream << "In cycle " << mCycleCounter << ": ";
    mErrorStream << error << std::endl;
}

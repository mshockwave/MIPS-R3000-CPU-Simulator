
#include "Types.h"

#include <iomanip>

/*Context*/
const addr_t Context::INSTR_START_ADDR = U32_0;

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

const uint8_t Error::LEVEL_CONTINUE = 0;
const uint8_t Error::LEVEL_HALT = 1;

Error Error::NONE(0, "");
Error Error::WRITE_REG_ZERO(1, "Write $0 error");
Error Error::NUMBER_OVERFLOW(2, "Number Overflow");
Error Error::MEMORY_ADDR_OVERFLOW(3, LEVEL_HALT, "Address Overflow");
Error Error::DATA_MISALIGNED(4, LEVEL_HALT, "Misalignment Error");

std::ostream& operator<<(std::ostream& os, const Error& error){
    os << error.mDescription;
    return os;
}


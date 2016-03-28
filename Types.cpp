
#include "Types.h"

#include <iomanip>

const uint8_t Error::LEVEL_CONTINUE = 0;
const uint8_t Error::LEVEL_HALT = 1;

Error Error::NONE(0, "");
Error Error::WRITE_REG_ZERO(1, "Write $0 Error");
Error Error::NUMBER_OVERFLOW(2, "Number Overflow");
Error Error::MEMORY_ADDR_OVERFLOW(4, LEVEL_HALT, "Address Overflow");
Error Error::DATA_MISALIGNED(8, LEVEL_HALT, "Misalignment Error");

std::ostream& operator<<(std::ostream& os, const Error& error){
    os << error.mDescription;
    return os;
}


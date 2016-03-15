
#include "Types.h"

static const uint8_t Error::LEVEL_CONTINUE = 0;
static const uint8_t Error::LEVEL_HALT = 1;

static Error Error::NONE(0, "");
static Error Error::WRITE_REG_ZERO(1, "Write $0 error");
static Error Error::NUMBER_OVERFLOW(2, "Number Overflow");
static Error Error::MEMORY_ADDR_OVERFLOW(3, LEVEL_HALT, "Address Overflow");
static Error Error::DATA_MISALIGNED(4, LEVEL_HALT, "Misalignment Error");


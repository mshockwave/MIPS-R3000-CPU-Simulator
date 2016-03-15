
#include "Instruction.h"
#include <cstring>

inline void Instruction::convert() {
    //To Little endian
    for(int i = 0; i < INSTRUCTION_BYTE_WIDTH; i++){
        this->mInstruction[i] = this->mRawInstruction[INSTRUCTION_BYTE_WIDTH - 1 - i];
    }

    //Convert to 32-bits uint for bits operations
    for(int i = 0; i < INSTRUCTION_BYTE_WIDTH; i++){
        uint32_t b = static_cast<uint32_t>(this->mInstruction[i]);
        this->mBitsInstruction |= (b << ((INSTRUCTION_BYTE_WIDTH - 1 - i) << 3));
    }
}

Instruction::Instruction(const byte_t *rawInstruction) :
        mBitsInstruction(0),
        mError(Error::NONE) {
    memcpy(this->mRawInstruction, rawInstruction, sizeof(byte_t) * INSTRUCTION_BYTE_WIDTH);
    convert();
}

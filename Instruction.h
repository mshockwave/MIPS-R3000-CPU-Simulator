
#ifndef ARCHIHW1_INSTRUCTION_H
#define ARCHIHW1_INSTRUCTION_H

#define INSTRUCTION_BYTE_WIDTH 4

#include "Types.h"

class Instruction {
private:
    byte_t mRawInstruction[INSTRUCTION_BYTE_WIDTH];
    byte_t mInstruction[INSTRUCTION_BYTE_WIDTH]; //little endian
    uint32_t mBitsInstruction;

    Error &mError;

    inline void convert();

public:

    Instruction(const byte_t* rawInstruction);

    const byte_t* getInstruction(){ return const_cast<const byte_t*>(mInstruction); }
    const uint32_t getBitsInstruction(){ return mBitsInstruction; }

    void setError(Error& error){ mError = error; }
    Error& getError(){ return mError; }
};


#endif //ARCHIHW1_INSTRUCTION_H


#ifndef ARCHIHW1_INSTRUCTION_H
#define ARCHIHW1_INSTRUCTION_H

#define INSTRUCTION_BYTE_WIDTH 4

#include "Types.h"
#include "RawBinary.h"
#include "Utils.h"

#include <vector>

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

struct Instructions{
private:
    std::vector<Instruction> mInstructions;

public:
    Instructions(RawBinary& binary);

    typedef std::vector<Instruction>::iterator iterator;

    uint32_t length(){ return static_cast<uint32_t>(mInstructions.size()); }

    iterator begin(){ return mInstructions.begin(); }
    iterator end(){ return mInstructions.end(); }
};

#endif //ARCHIHW1_INSTRUCTION_H

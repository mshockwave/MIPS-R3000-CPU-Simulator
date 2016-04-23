
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

    inline void convert();

public:

    Instruction(const byte_t* rawInstruction);

    const byte_t* getInstruction(){ return const_cast<const byte_t*>(mInstruction); }
    const uint32_t GetBitsInstruction(){ return mBitsInstruction; }

};

struct Instructions{

#ifndef NDEBUG
    /*
     * For unit tests
     * */
    friend class TestTasks;
#endif

private:
    std::vector<Instruction> mInstructions;

public:
#ifndef NDEBUG
    /*
     * For unit tests
     * */
    Instructions(){}
#endif
    Instructions(RawBinary& binary);

    typedef std::vector<Instruction>::iterator iterator;

    unsigned long length(){ return mInstructions.size(); }

    iterator begin(){ return mInstructions.begin(); }
    iterator end(){ return mInstructions.end(); }
};

#endif //ARCHIHW1_INSTRUCTION_H

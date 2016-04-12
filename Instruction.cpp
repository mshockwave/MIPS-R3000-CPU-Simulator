
#include "Instruction.h"

#include <cstring>

extern "C"{
#include <sys/time.h>
}

inline void Instruction::convert() {
    //To Little endian
    for(int i = 0; i < INSTRUCTION_BYTE_WIDTH; i++){
        this->mInstruction[i] = this->mRawInstruction[INSTRUCTION_BYTE_WIDTH - 1 - i];
    }

    //Convert to 32-bits uint for bits operations
    for(int i = 0; i < INSTRUCTION_BYTE_WIDTH; i++){
        uint32_t b = static_cast<uint32_t>(this->mRawInstruction[i]);
        this->mBitsInstruction |= (b << ((INSTRUCTION_BYTE_WIDTH - 1 - i) << 3));
    }
}

Instruction::Instruction(const byte_t *rawInstruction) :
        mBitsInstruction(0){
    memcpy(this->mRawInstruction, rawInstruction, sizeof(byte_t) * INSTRUCTION_BYTE_WIDTH);
    convert();
}

/*struct Instructions*/
Instructions::Instructions(RawBinary &binary) {

    auto bytes = binary.getInstructions();

    //TODO: assert binary.size() % 4 == 0
    //Load instruction length
    uint32_t instructionLength = U32_0;
    load2Register<4>(bytes, instructionLength);

    const byte_t* bytesArray = bytes.data();

    /*The first eight bytes are PC address and instruction size, skip*/
    uint32_t i, j;
    for(i = 0, j = 8; i < instructionLength; i++, j += INSTRUCTION_BYTE_WIDTH){
        Instruction instruction(bytesArray + j);
        mInstructions.push_back(instruction);
    }

    DEBUG_BLOCK {
        Log::D("Instructions Read") << "End Time(ms): " << getCurrentTimeMs() << std::endl;
    };
}
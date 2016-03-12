#include "TestCase.h"

#ifndef ARCHIHW1_TESTINSTRUCTION_H
#define ARCHIHW1_TESTINSTRUCTION_H

#include <cstdio>
#include "../Types.h"
#include "../Instruction.h"

class TestInstructionTransform : public TestCase {
public:
    TestInstructionTransform() :
            TestCase("InstructionTransform") {}

private:
    bool doTest();
};
bool TestInstructionTransform::doTest() {

    const byte_t rawInstruction[4] = { 0x00, 0x12, 0x34, 0x56 };
    Instruction instruction(rawInstruction);

    const byte_t* inst = instruction.getInstruction();
    for(int i = 0; i < INSTRUCTION_BYTE_WIDTH; i++){
        //printf("0x%x ", inst[i]);
        Log::D(EMPTY_STRING) << "0x" << std::hex << inst[i] << " ";
    }
    Log::D(EMPTY_STRING) << std::endl;

    //printf("0x%x\n", instruction.getBitsInstruction());
    Log::D(EMPTY_STRING) << "0x" << std::hex << instruction.getBitsInstruction() << std::endl;

    return true;
}

#endif //ARCHIHW1_TESTINSTRUCTION_H

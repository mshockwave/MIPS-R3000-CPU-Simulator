
#ifndef ARCHIHW1_TESTUTILS_H
#define ARCHIHW1_TESTUTILS_H

#include <vector>

#include "TestCase.h"
#include "../Utils.h"

class TestUtils : public TestCase {

public:
    TestUtils() :
            TestCase("Utils"){}

private:
    bool doTest();
};
bool TestUtils::doTest() {

    //Test extracting bits
    Log::V(mName) << "Testing bits slice..." << std::endl;
    const uint32_t value = 0x12345; //0000 0000 0000 0001 0010 0011 0100 0101

    uint32_t slice = extractInstrBits(value, 11, 5);
    std::stringstream ss;
    ss << "Expect 26, " << "get " << slice << std::endl;
    assert(slice == (uint32_t)26, ss.str());
    ss.clear();

    //Test loading data to register
    Log::V(mName) << "Testing register value loader..." << std::endl;
    const byte_t dataArray[] = {0x12, 0x34, 0x56, 0x78};
    std::vector<byte_t> dataVector(dataArray, std::end(dataArray));
    reg_t reg1 = U32_0;
    load2Register(dataVector, reg1);
    //Log::D(mName) << "Register value: 0x" << std::hex << reg1 << std::endl;
    ss << "Expect 0x12345678, " << "get 0x" << std::hex << reg1 << std::endl;
    assert(reg1 == static_cast<reg_t>(0x12345678), ss.str());
    ss.clear();

    return true;
}

#endif //ARCHIHW1_TESTUTILS_H

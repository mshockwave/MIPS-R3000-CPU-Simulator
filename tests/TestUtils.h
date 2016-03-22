
#ifndef ARCHIHW1_TESTUTILS_H
#define ARCHIHW1_TESTUTILS_H

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

    Log::D(mName) << "Testing bits slice..." << std::endl;
    const uint32_t value = 0x12345; //0000 0000 0000 0001 0010 0011 0100 0101

    uint32_t slice = extractInstrBits(value, 11, 5);
    std::stringstream ss;
    ss << "Expect 26, " << "get " << slice << std::endl;
    assert(slice == (uint32_t)26, ss.str());

    return true;
}

#endif //ARCHIHW1_TESTUTILS_H

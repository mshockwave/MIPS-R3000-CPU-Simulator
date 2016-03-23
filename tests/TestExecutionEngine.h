
#ifndef ARCHIHW1_TESTEXECUTIONENGINE_H
#define ARCHIHW1_TESTEXECUTIONENGINE_H

#include <iostream>

#include "TestCase.h"
#include "../Types.h"
#include "../ExecutionEngine.h"
#include "../Context.h"

class TestExecutionEngine : public TestCase {
public:
    TestExecutionEngine() :
            TestCase("ExecutionEngine"){}

private:
    bool doTest(){

        RawBinary data("test_dataset/func/iimage.bin", "test_dataset/func/dimage.bin");

        Instructions instructions(data);

        Context context(data, std::cout, Log::E(mName));
        Log::D("") << std::endl;

        ExecutionEngine engine(context, instructions);

        engine.start();

        return true;
    }
};

#endif //ARCHIHW1_TESTEXECUTIONENGINE_H

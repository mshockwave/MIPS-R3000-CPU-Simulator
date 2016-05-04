#ifndef _SIMULATOR_TEST_TYPES_H_
#define _SIMULATOR_TEST_TYPES_H_

#include "TestCase.h"
#include "../Types.h"

class TestTypes : public TestCase{

public:
    TestTypes() :
        TestCase("TestTypes"){}
    
private:
    
    bool testErrorCopyable(){
        
        Log::D(mName) << "Testing Error class copyable..." << std::endl;
        
        //Test copy operator
        Error e = Error::NUMBER_OVERFLOW;
        AssertEqual(e.GetId(), (Error::NUMBER_OVERFLOW).GetId(), "Error id");
        AssertEqual(e.GetDescription(), (Error::NUMBER_OVERFLOW).GetDescription(), "Error description");
        AssertEqual(e.GetErrorLevel(), (Error::NUMBER_OVERFLOW).GetErrorLevel(), "Error level");
        
        e.SetCycle(3);
        
        //Test copy constructor
        Error e2(e);
        AssertEqual(e2.GetId(), e.GetId(), "Error id");
        AssertEqual(e2.GetDescription(), e.GetDescription(), "Error description");
        AssertEqual(e2.GetErrorLevel(), e.GetErrorLevel(), "Error level");
        AssertEqual(e2.GetCycle(), 3, "Error cycle");
        
        return true;
    }
    
    bool testRegsDiffCopyable(){
        
        Log::D(mName) << "Testing RegsDiff class copyable..." << std::endl;
        
        //Test copy operator
        RegsDiff regs_diff;
        regs_diff.RegIndex = 1;
        regs_diff.RegValue = 2;
        regs_diff.Terminated = true;
        regs_diff.Abort = true;
        
        RegsDiff regs_diff2;
        regs_diff2 = regs_diff;
        AssertEqual(regs_diff2.RegIndex, (uint8_t)1, "RegsDiff RegIndex");
        AssertEqual(regs_diff2.RegValue, (reg_t)2, "RegsDiff RegIndex");
        AssertEqual(regs_diff2.Abort, true, "RegsDiff Abort");
        AssertEqual(regs_diff2.Terminated, true, "RegsDiff Terminated");
        
        
        //Test copy constructor
        RegsDiff regs_diff3(regs_diff);
        AssertEqual(regs_diff3.RegIndex, (uint8_t)1, "RegsDiff RegIndex");
        AssertEqual(regs_diff3.RegValue, (reg_t)2, "RegsDiff RegIndex");
        AssertEqual(regs_diff3.Abort, true, "RegsDiff Abort");
        AssertEqual(regs_diff3.Terminated, true, "RegsDiff Terminated");
        
        return true;
    }
    
    bool doTest(){
        
        bool result = true;
        result &= testErrorCopyable();
        result &= testRegsDiffCopyable();
        
        return result;
    }
};

#endif
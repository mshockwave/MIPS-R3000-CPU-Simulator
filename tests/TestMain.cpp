
#include "TestInstruction.h"
#include "TestUtils.h"
#include "TestAdts.h"
#include "TestTasks.h"
#include "TestTasksError.h"

int main(){

    //Add test cases
    std::vector<TestCase*> testCases;
    testCases.push_back(new TestUtils());
    testCases.push_back(new TestAdts());
    testCases.push_back(new TestInstructionTransform());
    testCases.push_back(new TestInstructionList());
    testCases.push_back(new TestTasks());
    testCases.push_back(new TestTasksError());

    std::vector<TestCase*>::iterator it = testCases.begin();
    for(; it != testCases.end(); ++it){
        bool fail = false;
        if( !((*it)->test()) ){
            Log::E("TestMain") << "Test failed on test case " << (*it)->getName() << std::endl;
            fail = true;
        }

        if((*it) != nullptr){
            TestCase* ptr = *it;
            delete ptr;
        }
        if(fail) break;
    }

    return 0;
}


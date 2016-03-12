
#ifndef ARCHIHW1_TESTCASE_H
#define ARCHIHW1_TESTCASE_H

#include <string>
#include <vector>

#include "../Utils.h"

class TestCase {

protected:
    std::string mName;

public:

    TestCase(std::string name) :
            mName(name) {}
    TestCase(const char* name) :
            TestCase(std::string(name)) {}

    std::string& getName() { return mName;}

    void init(){}

    bool test();

    void destroy(){}

    //Utils
    static void assert(bool v, std::string msg){
        if(!v){
            auto prefix = std::string("Assert failed: ");
            throw (prefix + msg);
        }
    }

protected:
    virtual bool doTest() = 0;
};
bool TestCase::test() {
    try{
        return doTest();
    }catch(std::string msg){
        Log::E(mName) << msg;
        return false;
    }
}

#endif //ARCHIHW1_TESTCASE_H

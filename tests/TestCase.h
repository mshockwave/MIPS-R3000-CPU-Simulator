
#ifndef ARCHIHW1_TESTCASE_H
#define ARCHIHW1_TESTCASE_H

#include <string>
#include <vector>
#include <sstream>

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
    static void Assert(bool v, std::string msg){
        if(!v){
            auto prefix = std::string("Assert failed: ");
            throw (prefix + msg);
        }
    }
    template <typename T>
    static void AssertEqual(const T& out, const T& expect, std::string prefix){
        bool result = (out == expect);
        if(!result){
            std::stringstream ss;
            ss << prefix << std::endl;
            ss << "\t\tExpect: " << expect << std::endl;
            ss << "\t\tGet: " << out << std::endl;
            throw ss.str();
        }
    }

protected:
    virtual bool doTest() = 0;
};
bool TestCase::test() {
    try{
        return doTest();
    }catch(std::string msg){
        Log::E(mName) << msg << std::endl;
        return false;
    }catch(const char* msg){
        Log::E(mName) << msg << std::endl;
        return false;
    }
}

#endif //ARCHIHW1_TESTCASE_H

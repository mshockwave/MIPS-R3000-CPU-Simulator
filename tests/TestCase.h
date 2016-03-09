
#ifndef ARCHIHW1_TESTCASE_H
#define ARCHIHW1_TESTCASE_H

#include <string>

class TestCase {
public:
    void init(){}

    bool test();

    void destroy(){}

    //Utils
    static void assert(bool v, std::string msg){

    }

protected:
    virtual bool doTest() = 0;
};
bool TestCase::test() {
    try{
        return doTest();
    }catch(const char*){
        
        return false;
    }catch(char *){

        return false;
    }
}

#endif //ARCHIHW1_TESTCASE_H


#ifndef ARCHIHW1_TESTADTS_H
#define ARCHIHW1_TESTADTS_H

#include <boost/chrono/chrono.hpp>
#include <boost/thread/thread.hpp>

#include "TestCase.h"
#include "../adts/BlockingQueue.h"
#include <string>

class TestAdts : public TestCase {

public:
    TestAdts() :
            TestCase("TestAdts"){}

private:

    bool testBlockingQueue(){

        BlockingQueue<std::string> msg_queue;

        boost::thread t1([&]()->void{
            boost::this_thread::sleep_for(boost::chrono::seconds(3));
            std::string str1("String1");
            msg_queue.Push(str1);
        });

        Log::V(mName) << "Try to Pop..." << std::endl;
        Log::V(mName) << "Get value: " << msg_queue.Pop() << std::endl;

        t1.join();

        return true;
    }

    bool doTest(){

        bool pass = true;

        pass &= testBlockingQueue();

        return pass;
    }
};

#endif //ARCHIHW1_TESTADTS_H

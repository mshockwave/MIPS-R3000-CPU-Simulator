//
// Created by 許民易 on 2016/3/9.
//

#ifndef ARCHIHW1_LOG_H
#define ARCHIHW1_LOG_H

#include <iostream>

class Log {
private:
    static std::ostream* sOstream;

public:
    static void setStream(std::ostream& ostr){
        sOstream = &ostr;
    }

    static void E();
    static void W();
    static void V();
    static void D();
};
std::ostream* Log::sOstream = &(std::cout);

void Log::E(){
    *(sOstream) << "Test";
}
void Log::W(){

}
void Log::V(){

}
void Log::D() {

}

#endif //ARCHIHW1_LOG_H

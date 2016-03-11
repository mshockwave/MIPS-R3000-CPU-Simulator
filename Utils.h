
#ifndef ARCHIHW1_UTILS_H
#define ARCHIHW1_UTILS_H

#include "Types.h"

#include <iostream>

class Log {
private:
    static std::ostream* sOstream;

public:
    static void setStream(std::ostream& ostr){
        sOstream = &ostr;
    }

    static void E(std::string tag, std::string msg);
    static void W(std::string tag, std::string msg);
    static void V(std::string tag, std::string msg);
    static void D(std::string tag, std::string msg);
};
std::ostream* Log::sOstream = &(std::cout);

void Log::E(std::string tag, std::string msg){
    *(sOstream) << "[Error] " << tag << ": " << msg << std::endl;
}
void Log::W(std::string tag, std::string msg){
    *(sOstream) << "[Warning] " << tag << ": " << msg << std::endl;
}
void Log::V(std::string tag, std::string msg){
    *(sOstream) << "[Verbose] " << tag << ": " << msg << std::endl;
}
void Log::D(std::string tag, std::string msg) {
    *(sOstream) << "[Debug] " << tag << ": " << msg << std::endl;
}

inline uint32_t createBitMask(uint8_t width){
    return (1 << width) - U32_1;
}
/**
 * Extract [msb, lsb] range of bit(s)
 * And put in a 32-bits uint align along 0 bits
 * **/
inline uint32_t extractInstrBits(uint32_t value, uint8_t msb, uint8_t lsb){
    if(msb < lsb) return 0;

    uint8_t bitsCount = msb - lsb + U8_1;
    uint32_t result = (value >> lsb);
    return (result & createBitMask(bitsCount));
}

#endif //ARCHIHW1_UTILS_H

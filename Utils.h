
#ifndef ARCHIHW1_UTILS_H
#define ARCHIHW1_UTILS_H

#include "Types.h"

#include <iostream>

const std::string EMPTY_STRING("");

class Log {
private:
    static std::ostream* sOstream;

public:
    static void setStream(std::ostream& ostr){
        sOstream = &ostr;
    }

    static std::ostream& E(const std::string &tag) {
        return (*sOstream) << "[Error] " << tag << ": ";
    }

    static std::ostream& W(const std::string &tag) {
        return (*sOstream) << "[Warning] " << tag << ": ";
    }

    static std::ostream& V(const std::string &tag) {
        return (*sOstream) << "[Verbose] " << tag << ": ";
    }

    static std::ostream& D(const std::string &tag) {
        return (*sOstream) << "[Debug] " << tag << ": ";
    }
};

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

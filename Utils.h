
#ifndef ARCHIHW1_UTILS_H
#define ARCHIHW1_UTILS_H

#include "Types.h"

#include <iostream>
#include <vector>

const std::string EMPTY_STRING("");

#if defined(__clang__) || defined(__GUNC__)
    #define UNLIKELY(condition) (__builtin_expect(!!(condition), 0))
    #define LIKELY(condition) (__builtin_expect(!!(condition), 1))
#else
    /*Fallbacks*/
    #define UNLIKELY(condition) (condition)
    #define LIKELY(condition) (condition)
#endif

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

template<
        std::size_t start_index = 0,
        std::size_t W = 4,
        bool bigEndian = true
>
inline void load2Register(std::vector<byte_t> &data, reg_t &outputReg){
    if(UNLIKELY(data.size() < W || (data.begin() + start_index >= data.end()))) return;
    for(std::size_t i = start_index; i < (start_index + W); i++){
        reg_t b = static_cast<reg_t>((bigEndian)? data[i] : data[(W - 1) - i]);
        outputReg |= (b << ((W - 1 - i) << 3));
    }
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

inline bool isSigned(uint32_t value){
    return ((1 << 31) & value) != 0 ;
}

inline bool isSumOverflow(uint32_t a, uint32_t b, uint32_t result){
    return (isSigned(a) && isSigned(b) && !isSigned(result)) ||
            (!isSigned(a) && !isSigned(b) && isSigned(result));
}

inline uint32_t signExtend16(uint16_t v){
    int32_t ex = static_cast<int32_t>(v);
    ex <<= 16;
    return static_cast<uint32_t>(ex >> 16);
}
inline uint32_t signExtend8(uint8_t v){
    int32_t ex = static_cast<int32_t>(v);
    ex <<= 24;
    return static_cast<uint32_t>(ex >> 24);
}

#endif //ARCHIHW1_UTILS_H


#ifndef ARCHIHW1_UTILS_H
#define ARCHIHW1_UTILS_H

#include "Types.h"
#include "RawBinary.h"

#include "utils/RawBufferHandle.h"

#include <iostream>
#include <vector>

extern "C"{
#include <sys/time.h>
};

const std::string EMPTY_STRING("");

#if defined(__clang__) || defined(__GUNC__)
    #define UNLIKELY(condition) (__builtin_expect(!!(condition), 0))
    #define LIKELY(condition) (__builtin_expect(!!(condition), 1))
#else
    /*Fallbacks*/
    #define UNLIKELY(condition) (condition)
    #define LIKELY(condition) (condition)
#endif

#if defined(NDEBUG)
    #define DEBUG_BLOCK \
        if(0)
#else
    #define DEBUG_BLOCK \
        if(1)
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
inline void load2Register(RawBinary::raw_container_t &data, reg_t &outputReg){
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
inline uint32_t reverse32ByteOrder(uint32_t val){
    uint32_t base = U32_0;
    base |= (extractInstrBits(val, 7, 0) << 24);
    base |= (extractInstrBits(val, 15, 8) << 16);
    base |= (extractInstrBits(val, 23, 16) << 8);
    base |= extractInstrBits(val, 31, 24);
    return base;
}
inline uint16_t reverse16ByteOrder(uint16_t val){
    uint16_t base = static_cast<uint16_t>(0);
    base |= (extractInstrBits(val, 7, 0) << 8);
    base |= extractInstrBits(val, 15, 8);
    return base;
}

inline bool isSigned(uint32_t value){
    return ((1 << 31) & value) != 0 ;
}
inline uint32_t twoComplement(uint32_t val){
    return (~val) + U32_1;
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

//Do not insert new line
#define OSTREAM_HEX_OUTPUT_FMT(width) std::setfill('0') << std::setw((width)) << std::hex << std::uppercase

inline long getCurrentTimeMs(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double time_start = tv.tv_sec * 1000.0;
    time_start += (tv.tv_usec / 1000.0);

    long time_start_ms = static_cast<long>(time_start);

    return time_start_ms % (3600L * 1000L);
}

#endif //ARCHIHW1_UTILS_H

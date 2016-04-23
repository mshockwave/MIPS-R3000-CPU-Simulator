
#ifndef ARCHIHW1_TYPES_H
#define ARCHIHW1_TYPES_H

#include <cstdint>
#include <functional>
#include <string>
#include <ostream>

typedef uint8_t byte_t;

const uint8_t REGISTER_COUNT = 32;
typedef uint32_t reg_t;
typedef uint32_t word_t;
typedef uint16_t half_w_t;

typedef uint32_t addr_t;

const uint32_t WORD_WIDTH = 4;
const uint32_t MEMORY_LENGTH = (1 << 10);

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#define FILE_PATH_SEP "\\"
#else
#define FILE_PATH_SEP "/"
#endif

class Context;
class Instruction;

class Error {
private:
    uint8_t mErrorId; //Bit masks
    std::string mDescription;

    //0 for the default level, would continue
    //1 for the severe level, halt
    uint8_t mErrorLevel;

protected:
    Error(uint8_t id, uint8_t level, std::string description) :
            mErrorId(id),
            mDescription(description),
            mErrorLevel(level) {}
    Error(uint8_t id, std::string description) :
            Error(id, LEVEL_CONTINUE, description){}

public:
    static const uint8_t LEVEL_CONTINUE;
    static const uint8_t LEVEL_HALT;

    static Error NONE; //Place holder
    static Error PIPLINE_STALL;
    static Error WRITE_REG_ZERO;
    static Error NUMBER_OVERFLOW;
    static Error MEMORY_ADDR_OVERFLOW;
    static Error DATA_MISALIGNED;

    inline bool contains(const Error& e){
        return (mErrorId & e.mErrorId) != 0;
    }

    bool operator==(const Error& rhs) const{
        return this->mErrorId == rhs.mErrorId;
    }
    bool operator==(Error& rhs){
        return this->mErrorId == rhs.mErrorId;
    }
    bool operator!=(const Error& rhs) const{ return !(*this == rhs); }
    bool operator!=(Error& rhs){ return !(*this == rhs); }

    //Error mix
    Error operator+(Error& rhs){
        return Error(rhs.mErrorId | this->mErrorId,
                     rhs.mErrorLevel | this->mErrorLevel,
                     "Mix Error");
    }
    const Error operator+(const Error& rhs){
        return Error(rhs.mErrorId | this->mErrorId,
                     rhs.mErrorLevel | this->mErrorLevel,
                     "Mix Error");
    }

    friend std::ostream& operator<<(std::ostream& os, const Error& error);
};

const unsigned int THREAD_COUNT = 5;

typedef uint32_t task_id_t;
class TaskHandle;
typedef TaskHandle task_t;

const uint8_t U8_1 = (uint8_t)1;
const uint32_t U32_1 = (uint32_t)1;
const uint32_t U32_0 = (uint32_t)0;

#endif //ARCHIHW1_TYPES_H

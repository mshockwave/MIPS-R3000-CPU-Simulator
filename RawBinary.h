#ifndef ARCHIHW1_RAWBINARY_H
#define ARCHIHW1_RAWBINARY_H

#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

extern "C"{
#include <sys/stat.h>
};

#include "Types.h"

#include "utils/RawBufferHandle.h"

class RawBinary {

public:
    typedef RawBufferHandle<byte_t> raw_container_t;

private:
    raw_container_t* mRawInstructions;
    raw_container_t* mRawData;

    inline off_t get_file_size(int fd){
        struct stat fileStat;
        fstat(fd, &fileStat);

        return fileStat.st_size;
    }

public:

    RawBinary(std::string instFilePath, std::string dataFilePath);
    RawBinary(const char* instFilePath, const char* dataFilePath) :
            RawBinary(std::string(instFilePath), std::string(dataFilePath)){}

    raw_container_t& getInstructions() { return *mRawInstructions; }
    raw_container_t& getDataImg() { return *mRawData; }

    ~RawBinary(){
        if(mRawData != nullptr) delete mRawData;
        if(mRawInstructions != nullptr) delete mRawInstructions;
    }
};


#endif //ARCHIHW1_RAWBINARY_H

#ifndef ARCHIHW1_RAWBINARY_H
#define ARCHIHW1_RAWBINARY_H

#include <vector>
#include <string>
#include <cstdint>

#include "Types.h"

class RawBinary {
private:
    std::vector<byte_t> mRawInstructions;
    std::vector<byte_t> mRawData;

public:

    RawBinary(std::string instFilePath, std::string dataFilePath);
    RawBinary(const char* instFilePath, const char* dataFilePath) :
            RawBinary(std::string(instFilePath), std::string(dataFilePath)){}

    std::vector<byte_t>& getInstructions() { return mRawInstructions; }
    std::vector<byte_t>& getDataImg() { return mRawData; }
};


#endif //ARCHIHW1_RAWBINARY_H

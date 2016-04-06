#include "RawBinary.h"
#include "Utils.h"
#include <fstream>

RawBinary::RawBinary(std::string instFilePath, std::string dataFilePath) {
    DEBUG_BLOCK {
        Log::D("Instructions Read") << "Start Time(ms): " << getCurrentTimeMs() << std::endl;
    };

    std::ifstream instFileStream(instFilePath, std::ios::binary);
    std::ifstream dataFileStream(dataFilePath, std::ios::binary);
    if(instFileStream.fail() || dataFileStream.fail()){
        throw "Error reading instruction or data file";
    }

    instFileStream.unsetf(std::ios::skipws);
    dataFileStream.unsetf(std::ios::skipws);

    //Get file size in order to improve vector performance
    std::streampos instFileSize, dataFileSize;

    instFileStream.seekg(0, std::ios::end);
    instFileSize = instFileStream.tellg();
    instFileStream.seekg(0, std::ios::beg);

    dataFileStream.seekg(0, std::ios::end);
    dataFileSize = dataFileStream.tellg();
    dataFileStream.seekg(0, std::ios::beg);

    mRawInstructions.resize(static_cast<std::size_t>(instFileSize));
    mRawData.resize(static_cast<std::size_t>(dataFileSize));

    instFileStream.read(reinterpret_cast<char*>(&mRawInstructions.front()),
                        static_cast<std::size_t>(instFileSize));
    dataFileStream.read(reinterpret_cast<char*>(&mRawData.front()),
                        static_cast<std::size_t>(dataFileSize));

    instFileStream.close();
    dataFileStream.close();
}

#include "RawBinary.h"
#include "Utils.h"
#include <fstream>

extern "C" {
#include <sys/mman.h>
};

RawBinary::RawBinary(std::string instFilePath, std::string dataFilePath) {
    DEBUG_BLOCK {
        Log::D("Instructions Read") << "Start Time(ms): " << getCurrentTimeMs() << std::endl;
    };

    FILE* instFile = fopen(instFilePath.c_str(), "rb");
    FILE* dataFile = fopen(dataFilePath.c_str(), "rb");
    if(instFile == NULL || dataFile == NULL){
        throw "Error reading instruction or data file";
    }

    int instFd = fileno(instFile),
        dataFd = fileno(dataFile);

    //Get file size in order to improve vector performance
    long instFileSize = get_file_size(instFd),
            dataFileSize = get_file_size(dataFd);

    mRawInstructions.resize(static_cast<std::size_t>(instFileSize));
    mRawData.resize(static_cast<std::size_t>(dataFileSize));

    //mRawInstructions.data() = (byte_t*)mmap(NULL, static_cast<size_t>(instFileSize), PROT_READ, MAP_PRIVATE, instFd, 0);
}

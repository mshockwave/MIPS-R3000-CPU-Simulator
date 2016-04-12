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
    long inst_file_size = get_file_size(instFd),
            data_file_size = get_file_size(dataFd);

    auto map_instr = (byte_t*)mmap(NULL, static_cast<size_t>(inst_file_size), PROT_READ, MAP_PRIVATE, instFd, 0);
    mRawInstructions = raw_container_t::Wrap(map_instr, static_cast<size_t>(inst_file_size));
    raw_container_t::release_callback_t inst_cb = [](raw_container_t& self)->void {
        int ret = munmap(self.content(), self.size());
        DEBUG_BLOCK{
            if(ret){
                Log::D("RawBinary") << "Error unmap instruction part" << std::endl;
            }else{
                Log::D("RawBinary") << "Release instruction mmap!" << std::endl;
            }
        };
    };
    mRawInstructions->SetReleaseCallback(inst_cb);

    auto map_data = (byte_t*)mmap(NULL, static_cast<size_t>(data_file_size), PROT_READ, MAP_PRIVATE, dataFd, 0);
    mRawData = raw_container_t::Wrap(map_data, static_cast<size_t>(data_file_size));
    raw_container_t::release_callback_t data_cb = [](raw_container_t& self)->void {
        int ret = munmap(self.content(), self.size());
        DEBUG_BLOCK {
            if(ret){
                Log::D("RawBinary") << "Error unmap data part" << std::endl;
            }else{
                Log::D("RawBinary") << "Release data mmap!" << std::endl;
            }
        };
    };
    mRawData->SetReleaseCallback(data_cb);
}

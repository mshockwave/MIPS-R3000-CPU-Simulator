#include <cstring>
#include <cstdlib>

#include <string>
#include <sstream>
#include <fstream>
#include <initializer_list>

#include "Types.h"
#include "Utils.h"
#include "Context.h"
#include "ExecutionEngine.h"

using namespace std;

#define SNAPSHOT_FILE   "snapshot.rpt"
#define ERROR_DUMP_FILE "error_dump.rpt"
#define CMP_DUMP_FILE   "report.rpt"

/*
 {block size, page size, mem size, cache size, set associate}
 */
static const std::initializer_list<size_t> default_instr_cmp_config{4, 8, 64, 16, 4};
static const std::initializer_list<size_t> default_data_cmp_config{4, 16, 32, 16, 1};

int main(int argc, char **argv) {

    try{
        std::stringstream ssInstrImage;
        ssInstrImage << "iimage.bin";

        std::stringstream ssDataImg;
        ssDataImg << "dimage.bin";
        
        RawBinary inputBinary(ssInstrImage.str(), ssDataImg.str());
        
        initializer_list<size_t> instr_cmp_config(default_instr_cmp_config);
        initializer_list<size_t> data_cmp_config(default_data_cmp_config);
        if(argc - 1 >= 10){
            // Read CMP configurations
            size_t i_mem_size = ::atol(argv[1]);
            size_t d_mem_size = ::atol(argv[2]);
            
            size_t i_page_size = ::atol(argv[3]);
            size_t d_page_size = ::atol(argv[4]);
            
            size_t i_cache_size = ::atol(argv[5]);
            size_t d_cache_size = ::atol(argv[8]);
            
            size_t i_set_assoc = ::atol(argv[7]);
            size_t d_set_assoc = ::atol(argv[10]);
            
            size_t i_block_size = ::atol(argv[6]);
            size_t d_block_size = ::atol(argv[9]);
            
            instr_cmp_config = {i_block_size, i_page_size, i_mem_size, i_cache_size, i_set_assoc};
            data_cmp_config = {d_block_size, d_page_size, d_mem_size, d_cache_size, d_set_assoc};
            
        }else if(argc - 1 != 0){
            std::cout << "Unrecognized command line options" << std::endl;
            return 1;
        }

        Instructions instructions(inputBinary, instr_cmp_config);

        std::ofstream snapShotOut(SNAPSHOT_FILE);
        std::ofstream errorOut(ERROR_DUMP_FILE);

        Context context(inputBinary,
                        data_cmp_config,
                        snapShotOut, errorOut);

        ExecutionEngine engine(context, instructions);

        engine.Start();

        snapShotOut.close();
        errorOut.close();
        
        /*Write CMP report*/
        {
            std::ofstream os(CMP_DUMP_FILE);
            
            // Instruction Cache
            os << "ICache :" <<std::endl;
            auto instr_cache_profile = instructions.GetCacheProfileData();
            os << "# hits: " << std::get<0>(instr_cache_profile) << std::endl;
            os << "# misses: " << std::get<1>(instr_cache_profile) << std::endl;
            os << std::endl;
            
            // Data Cache
            os << "DCache :" <<std::endl;
            auto data_cache_profile = context.GetCacheProfileData();
            os << "# hits: " << std::get<0>(data_cache_profile) << std::endl;
            os << "# misses: " << std::get<1>(data_cache_profile) << std::endl;
            os << std::endl;
            
            // Instruction TLB
            os << "ITLB :" <<std::endl;
            auto instr_tlb_profile = instructions.GetTLBProfileData();
            os << "# hits: " << std::get<0>(instr_tlb_profile) << std::endl;
            os << "# misses: " << std::get<1>(instr_tlb_profile) << std::endl;
            os << std::endl;
            
            // Data TLB
            os << "DTLB :" <<std::endl;
            auto data_tlb_profile = context.GetTLBProfileData();
            os << "# hits: " << std::get<0>(data_tlb_profile) << std::endl;
            os << "# misses: " << std::get<1>(data_tlb_profile) << std::endl;
            os << std::endl;
            
            // Instruction PageTable
            os << "IPageTable :" <<std::endl;
            auto instr_pt_profile = instructions.GetPageProfileData();
            os << "# hits: " << std::get<0>(instr_pt_profile) << std::endl;
            os << "# misses: " << std::get<1>(instr_pt_profile) << std::endl;
            os << std::endl;
            
            // Data PageTable
            os << "DPageTable :" <<std::endl;
            auto data_pt_profile = context.GetPageProfileData();
            os << "# hits: " << std::get<0>(data_pt_profile) << std::endl;
            os << "# misses: " << std::get<1>(data_pt_profile) << std::endl;
            os << std::endl;
            
            os.close();
        }

        return 0;
    }catch(const char *e){
        Log::E("Main") << e << std::endl;
        return 1;
    }catch(Error& e){
        Log::E("Main") << e << std::endl;
        return 1;
    }catch(...) {
        Log::E("Main") << "Encounter Unexpected Error" << std::endl;
        return 1;
    }
}
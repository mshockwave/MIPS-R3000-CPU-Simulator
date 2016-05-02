#include <cstring>
#include <string>
#include <sstream>
#include <fstream>

#include "Types.h"
#include "Utils.h"
#include "Context.h"
#include "ExecutionEngine.h"
#include "IFEngine.h"
#include "tasks/TaskHandle.h"

#include <boost/thread/thread.hpp>

using namespace std;

#define SNAPSHOT_FILE   "snapshot.rpt"
#define ERROR_DUMP_FILE "error_dump.rpt"

int main(int argc, char **argv) {

    char testCasePath[100] = { '\0' };

    if(argc < 2){
        //Log::E("") << "Usage: " << argv[0] << " <test case folder name>" << std::endl;
        testCasePath[0] = '.';
    }else{
        strncpy(testCasePath, argv[1], sizeof(char) * 100);
    }
    
    std::ofstream snapShotOut(SNAPSHOT_FILE);
    std::ofstream errorOut(ERROR_DUMP_FILE);
    
    try{
        std::stringstream ssInstrImage;
        ssInstrImage << testCasePath << FILE_PATH_SEP << "iimage.bin";
        
        std::stringstream ssDataImg;
        ssDataImg << testCasePath << FILE_PATH_SEP << "dimage.bin";
        
        RawBinary raw_binary(ssInstrImage.str(), ssDataImg.str());
        
        Instructions instructions(raw_binary);
        
        boost::thread_group group;
        
        Context ctx(raw_binary, &group,
                    snapShotOut, errorOut);
        task::InitInstructionMap();
        task::InitTasks();
        
        ctx.setInstructionCount((uint32_t)instructions.length());
        
        TaskHandle::ClockHandle::Barrier rising_barrier(THREAD_COUNT);
        TaskHandle::ClockHandle::Barrier falling_barrier(THREAD_COUNT);
        TaskHandle::ClockHandle clock_handle(rising_barrier, falling_barrier);
        
        /*IF*/
        group.create_thread([&]()->void{
            IFEngine engine(ctx, instructions, clock_handle);
            engine.Start();
        });
        /*ID*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::IDEngineRunnable);
            engine.Start();
        });
        /*EX*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::EXEngineRunnable);
            engine.Start();
        });
        /*DM*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::DMEngineRunnable);
            engine.Start();
        });
        /*WB*/
        group.create_thread([&]()->void{
            ExecutionEngine engine(ctx, clock_handle,
                                   engines::WBEngineRunnable);
            engine.Start();
        });
        
        ctx.StartPrinterLoop(&group);
        
        snapShotOut.close();
        errorOut.close();
        
        return 0;

    }catch(const char *e){
        Log::E("Main") << e << std::endl;
        
        snapShotOut.close();
        errorOut.close();
        
        return 1;
        
    }catch(const Error& e){
        Log::E("Main") << e << std::endl;
        
        snapShotOut.close();
        errorOut.close();
        
        return 1;
    }catch(...){
        Log::E("Main") << "Encounter Unexpected Error" << std::endl;
        
        snapShotOut.close();
        errorOut.close();
        
        return 1;
    }

}
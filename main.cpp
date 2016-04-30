#include <cstring>
#include <string>
#include <sstream>
#include <fstream>

#include "Types.h"
#include "Utils.h"
#include "Context.h"
#include "ExecutionEngine.h"

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

    /*
    try{
        std::stringstream ssInstrImage;
        ssInstrImage << testCasePath << FILE_PATH_SEP << "iimage.bin";

        std::stringstream ssDataImg;
        ssDataImg << testCasePath << FILE_PATH_SEP << "dimage.bin";
        //Log::D("Main") << "Instruction Image Path: " << instrImagePath << std::endl;
        //Log::D("Main") << "Data Image Path: " << dataImagePath << std::endl;
        RawBinary inputBinary(ssInstrImage.str(), ssDataImg.str());

        Instructions instructions(inputBinary);

        std::ofstream snapShotOut(SNAPSHOT_FILE);
        std::ofstream errorOut(ERROR_DUMP_FILE);

        Context context(inputBinary, snapShotOut, errorOut);

        ExecutionEngine engine(context, instructions);

        engine.Start();

        snapShotOut.close();
        errorOut.close();

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
     */
    return 0;
}
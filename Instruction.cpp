
#include "Instruction.h"

#include <cstring>

extern "C"{
#include <sys/time.h>
}

inline void Instruction::convert() {
    //To Little endian
    for(int i = 0; i < INSTRUCTION_BYTE_WIDTH; i++){
        this->mInstruction[i] = this->mRawInstruction[INSTRUCTION_BYTE_WIDTH - 1 - i];
    }

    //Convert to 32-bits uint for bits operations
    for(int i = 0; i < INSTRUCTION_BYTE_WIDTH; i++){
        uint32_t b = static_cast<uint32_t>(this->mRawInstruction[i]);
        this->mBitsInstruction |= (b << ((INSTRUCTION_BYTE_WIDTH - 1 - i) << 3));
    }
}

Instruction::Instruction(const byte_t *rawInstruction) :
        mBitsInstruction(0){
    memcpy(this->mRawInstruction, rawInstruction, sizeof(byte_t) * INSTRUCTION_BYTE_WIDTH);
    convert();
}

/*struct Instructions*/
Instructions::Instructions(RawBinary &binary,
                           cmp::CMP::cmp_config_t instr_cmp_config) : instr_length(U32_0) {

    //Hint: Do not use auto here
    // For the sake of preventing unexpected memory release
    RawBinary::raw_container_t& instr_bytes = binary.getInstructions();

    //TODO: assert binary.size() % 4 == 0
    //Load instruction length
    load2Register<4>(instr_bytes, instr_length);
    
    addr_t start_addr = U32_0;
    load2Register(instr_bytes, start_addr);

    auto* bytesArray = instr_bytes.content();
    
    /*Init CMP module*/
    cmp_module = std::make_shared<cmp::CMP>(instr_cmp_config,
                                            start_addr,
                                            bytesArray + 8, // Skip first eight bytes of metadata
                                            1024/*Fix size, 1K*/);

    /*The first eight bytes are PC address and instruction size, skip*/
    /*
    uint32_t i, j;
    for(i = 0, j = 8; i < instructionLength; i++, j += INSTRUCTION_BYTE_WIDTH){
        Instruction instruction(bytesArray + j);
        mInstructions.push_back(instruction);
    }
     */

    DEBUG_BLOCK {
        Log::D("Instructions Read") << "End Time(ms): " << getCurrentTimeMs() << std::endl;
    };
}

/*Instructions iterator*/
Instruction Instructions::_instructions_iterator_::operator*(){
    // TODO: Verify whether offset valid
    
    addr_t vir_addr = cmp_module->GetStartAddr() + vir_addr_offset;
    auto* data_ptr = cmp_module->Access(vir_addr);
    
    return Instruction(data_ptr);
}

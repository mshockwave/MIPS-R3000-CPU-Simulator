
#ifndef ARCHIHW1_TYPES_H
#define ARCHIHW1_TYPES_H

#include <cstdint>
#include <functional>

typedef uint8_t byte_t;

const uint8_t REGISTER_COUNT = 32;
typedef uint32_t reg_t;

const uint32_t MEMORY_WIDTH = (1 << 10);

class Context {
public:
    //Registers
    reg_t Registers[REGISTER_COUNT];
    //Special registers
    reg_t PC;
    reg_t &ZERO, &AT, &SP, &FP, &RA;

    //Memory
    byte_t Memory[MEMORY_WIDTH];

    Context(reg_t pc) :
            PC(pc),
            ZERO(Registers[0]),
            AT(Registers[1]),
            SP(Registers[29]),
            FP(Registers[30]),
            RA(Registers[31]){
        //Zero registers
        for(int i = 0; i < REGISTER_COUNT; i++){
            Registers[i] = (byte_t)0;
        }

        //Zero memory
        for(int i = 0; i < MEMORY_WIDTH; i++){
            Memory[i] = (byte_t)0;
        }
    }
};

typedef uint32_t task_id_t;

const uint8_t U8_1 = (uint8_t)1;
const uint32_t U32_1 = (uint32_t)1;

#endif //ARCHIHW1_TYPES_H

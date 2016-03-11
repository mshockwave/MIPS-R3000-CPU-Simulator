
#ifndef ARCHIHW1_TYPES_H
#define ARCHIHW1_TYPES_H

#include <cstdint>
#include <functional>

typedef uint8_t byte_t;

const uint8_t REGISTER_COUNT = 32;
typedef uint32_t register_t;

const uint32_t MEMORY_WIDTH = (1 << 10);

class Context {
public:
    //Registers
    register_t Registers[REGISTER_COUNT];
    //Special registers
    register_t *PC, *SP;

    //Memory
    byte_t Memory[MEMORY_WIDTH];

    Context() :
            PC(&Registers[0]), SP(&Registers[1]) /*TODO: Verify*/{
        //Zero registers
        for(int i = 0; i < REGISTER_COUNT; i++){
            Registers[i] = 0;
        }

        //Zero register
        for(int i = 0; i < MEMORY_WIDTH; i++){
            Memory[i] = 0;
        }
    }
};

typedef uint32_t task_id_t;

const uint8_t U8_1 = (uint8_t)1;
const uint32_t U32_1 = (uint32_t)1;

#endif //ARCHIHW1_TYPES_H

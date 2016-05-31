
#ifndef ARCHIHW1_INSTRUCTION_H
#define ARCHIHW1_INSTRUCTION_H

#define INSTRUCTION_BYTE_WIDTH 4

#include "Types.h"
#include "RawBinary.h"
#include "Utils.h"
#include "CMP.hpp"

#include <vector>
#include <memory>

class Instruction {
private:
    byte_t mRawInstruction[INSTRUCTION_BYTE_WIDTH];
    byte_t mInstruction[INSTRUCTION_BYTE_WIDTH]; //little endian
    uint32_t mBitsInstruction;

    inline void convert();

public:

    Instruction(const byte_t* rawInstruction);

    const byte_t* getInstruction(){ return const_cast<const byte_t*>(mInstruction); }
    const uint32_t GetBitsInstruction(){ return mBitsInstruction; }

};

struct Instructions{
    
    friend class _instructions_iterator_;
    
private:
    std::vector<Instruction> mInstructions;
    typedef std::shared_ptr<cmp::CMP> cmp_ptr_t;
    cmp_ptr_t cmp_module;

public:
    Instructions(RawBinary& binary,
                 cmp::CMP::cmp_config_t instr_cmp_config);
    
    class _instructions_iterator_ {
        
    public:
        
        _instructions_iterator_(cmp_ptr_t cmp_mod) :
            cmp_module(cmp_mod),
            vir_addr_offset(0){}
        
        _instructions_iterator_() :
            vir_addr_offset(0){/*Default constructor*/}
        
        _instructions_iterator_(const _instructions_iterator_& that){
            /*Copy constructor*/
            cmp_module = that.cmp_module;
            vir_addr_offset = that.vir_addr_offset;
        }
        
        _instructions_iterator_ operator+(size_t n){
            _instructions_iterator_ it_new(*this);
            it_new.vir_addr_offset += (n * INSTRUCTION_BYTE_WIDTH);
            return it_new;
        }
        _instructions_iterator_& operator+=(size_t n){
            this->vir_addr_offset += (n * INSTRUCTION_BYTE_WIDTH);
            return *this;
        }
        
        _instructions_iterator_ operator-(size_t n){
            _instructions_iterator_ it_new(*this);
            it_new.vir_addr_offset -= (n * INSTRUCTION_BYTE_WIDTH);
            return it_new;
        }
        _instructions_iterator_& operator-=(size_t n){
            this->vir_addr_offset -= (n * INSTRUCTION_BYTE_WIDTH);
            return *this;
        }
        
        _instructions_iterator_& operator++(){
            //++x
            this->operator+=(1);
            return *this;
        }
        _instructions_iterator_ operator++(int){
            //x++
            _instructions_iterator_ tmp(*this);
            this->operator+=(1);
            return tmp;
        }
        
        _instructions_iterator_& operator--(){
            //--x
            this->operator-=(1);
            return *this;
        }
        _instructions_iterator_ operator--(int){
            //x--
            _instructions_iterator_ tmp(*this);
            this->operator-=(1);
            return tmp;
        }
        
        Instruction operator*();
        
        bool operator==(const _instructions_iterator_ &rhs){ return (vir_addr_offset == rhs.vir_addr_offset); }
        bool operator!=(const _instructions_iterator_ &rhs){ return !(*this == rhs); }
        bool operator>=(const _instructions_iterator_ &rhs){ return (vir_addr_offset >= rhs.vir_addr_offset); }
        bool operator<=(const _instructions_iterator_ &rhs){ return (vir_addr_offset <= rhs.vir_addr_offset); }
        bool operator<(const _instructions_iterator_ &rhs){ return !(*this >= rhs); }
        bool operator>(const _instructions_iterator_ &rhs){ return !(*this <= rhs); }
        
    private:
        cmp_ptr_t cmp_module;
        addr_t vir_addr_offset;
        
        //inline bool is_cmp_valid(){ return cmp_module.get() != nullptr; }
    };

    //typedef std::vector<Instruction>::iterator iterator;
    typedef _instructions_iterator_ iterator;

    unsigned long length(){ return instr_length; }

    iterator begin(){ return _instructions_iterator_(cmp_module); }
    iterator end(){
        _instructions_iterator_ it_new(cmp_module);
        it_new += instr_length; // Pass the tail
        return it_new;
    }
    
private:
    uint32_t instr_length;
    
};

#endif //ARCHIHW1_INSTRUCTION_H

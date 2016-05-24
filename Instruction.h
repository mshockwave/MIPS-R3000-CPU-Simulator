
#ifndef ARCHIHW1_INSTRUCTION_H
#define ARCHIHW1_INSTRUCTION_H

#define INSTRUCTION_BYTE_WIDTH 4

#include "Types.h"
#include "RawBinary.h"
#include "Utils.h"
#include "MemCache.hpp"

#include <vector>
#include <unordered_map>
#include <set>

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
    
    friend class instruction_iterator;
    
public:
    Instructions(RawBinary& binary);
    
    class instruction_iterator{
        
        friend struct Instructions;
        
        uint32_t it_index;
        Instructions* instrs_ptr;
        uint32_t addr_boundary;
        
        bool isAccessible(){
            return !(it_index < addr_boundary &&
                     instrs_ptr != nullptr);
        }
        
    public:
        instruction_iterator() :
            it_index(0), addr_boundary(1),
            instrs_ptr(nullptr){}
        
        //Copy constructors
        instruction_iterator(const instruction_iterator &that){
            it_index = that.it_index;
            addr_boundary = that.addr_boundary;
            instrs_ptr = that.instrs_ptr;
        }
        instruction_iterator(instruction_iterator &that){
            it_index = that.it_index;
            addr_boundary = that.addr_boundary;
            instrs_ptr = that.instrs_ptr;
        }
        
        instruction_iterator operator+(size_t n){
            instruction_iterator it_new(*this);
            it_new.it_index = it_index + n;
            return it_new;
        }
        
        instruction_iterator& operator+=(int step){
            it_index += step;
            return *this;
        }
        
        instruction_iterator& operator++(){
            //suffix
            it_index++;
            return *this;
        }
        instruction_iterator operator++(int){
            //prefix
            instruction_iterator tmp(*this);
            it_index++;
            return tmp;
        }
        
        instruction_iterator& operator--(){
            //suffix
            it_index--;
            return *this;
        }
        instruction_iterator operator--(int){
            //prefix
            instruction_iterator tmp(*this);
            it_index--;
            return tmp;
        }
        
        Instruction operator*();
        
        bool operator==(const instruction_iterator &rhs){ return (it_index == rhs.it_index); }
        bool operator!=(const instruction_iterator &rhs){ return !(*this == rhs); }
        bool operator>=(const instruction_iterator &rhs){ return (it_index >= rhs.it_index); }
        bool operator<=(const instruction_iterator &rhs){ return (it_index <= rhs.it_index); }
        bool operator<(const instruction_iterator &rhs){ return !(*this >= rhs); }
        bool operator>(const instruction_iterator &rhs){ return !(*this <= rhs); }
        
    private:
        
    };
    
    //typedef std::vector<Instruction>::iterator iterator;
    typedef instruction_iterator iterator;

    unsigned long length(){ return instruction_length; }

    iterator begin(){
        iterator it;
        it.addr_boundary =  instruction_length;
        return it;
    }
    iterator end(){
        iterator it;
        it.it_index = instruction_length;
        it.addr_boundary =  instruction_length;
        return it;
    }
    
private:
    //std::vector<Instruction> mInstructions;
    addr_t instruction_length;
    addr_t instruction_start_addr;
    size_t instr_index_width,
            instr_tag_width,
            instr_offset_width;
    RawBinary& raw_binary;
    
    /*Caceh and page table of Instruction part*/
    std::vector<memcache::TLBEntry> TLB;
    std::unordered_map<addr_t, memcache::PageTableEntry> VirPageTable;
    std::vector<memcache::PhyPageEntry> PhyPages;
    typedef memcache::CacheEntry<> cache_entry_type;
    std::unordered_map<addr_t, std::vector< cache_entry_type > > Cache;
    
    uint32_t tlb_length(){
        return static_cast<uint32_t>(instruction_length >> 2); // 1/4
    }
    
    void tlb_insert(const memcache::TLBEntry& entry){
        // Find empty slot
        // And reset according to LRU if needed
        int32_t aval_index = -1;
        uint32_t i;
        bool all_used = true;
        for(i = 0; i < TLB.size(); i++){
            const auto& element = TLB[i];
            if(aval_index < 0 && !element.Use){
                aval_index = i;
            }else if(!element.Use){
                all_used = false;
            }
        }
        
        if(all_used){
            for(i = 0; i < TLB.size(); i++){
                TLB[i].Use = false;
            }
        }
        
        if(aval_index >= 0){
            TLB[aval_index] = entry;
            TLB[aval_index].Use = true;
        }
    }
    
    addr_t phy_page_insert(const memcache::PhyPageEntry& entry){
        // Find empty slot
        // And reset according to LRU if needed
        int32_t aval_index = -1;
        uint32_t i;
        bool all_used = true;
        for(i = 0; i < PhyPages.size(); i++){
            const auto& element = PhyPages[i];
            if(aval_index < 0 && !element.Use){
                aval_index = i;
            }else if(!element.Use){
                all_used = false;
            }
        }
        
        if(all_used){
            for(i = 0; i < PhyPages.size(); i++){
                PhyPages[i].Use = false;
            }
        }
        
        if(aval_index >= 0){
            // TODO: Invalid cache and page table
            
            PhyPages[aval_index] = entry;
            PhyPages[aval_index].Use = true;
            
            return static_cast<addr_t>(aval_index) * memcache::instr::PageSize;
        }
        
        return 0;
    }
};

#endif //ARCHIHW1_INSTRUCTION_H

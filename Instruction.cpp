
#include "Instruction.h"

#include <cstring>
#include <cmath>
#include <memory>

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
Instructions::Instructions(RawBinary &binary) :
    raw_binary(binary),
    instruction_length(U32_0){

        //Hint: Do not use auto here
        // For the sake of preventing unexpected memory release
        RawBinary::raw_container_t& instr_bytes = binary.getInstructions();
        
        //TODO: assert binary.size() % 4 == 0
        //Load instruction length
        load2Register<4>(instr_bytes, instruction_length);
        // Load start address
        load2Register(instr_bytes, instruction_start_addr);
        
        // Init TLB
        TLB.resize(tlb_length(), memcache::TLBEntry());
        // Init physical memory
        PhyPages.resize(memcache::instr::MemSize / memcache::instr::PageSize,
                        memcache::PhyPageEntry());
        // Init virtual address properties
        instr_offset_width = static_cast<size_t>(::log2l(memcache::instr::BlockSize));
        size_t block_amount = static_cast<size_t>(memcache::instr::CacheSize / memcache::instr::BlockSize);
        size_t cache_depth = static_cast<size_t>(block_amount / memcache::instr::SetAssoc);
        instr_index_width = static_cast<size_t>(::log2l(cache_depth));
        instr_tag_width = (INSTRUCTION_BYTE_WIDTH << 3) - (instr_offset_width + instr_index_width);

        const byte_t* bytesArray = instr_bytes.content();

        /*The first eight bytes are PC address and instruction size, skip*/
        /*
        uint32_t i, j;
        for(i = 0, j = 8; i < instruction_length; i++, j += INSTRUCTION_BYTE_WIDTH){
            Instruction instruction(bytesArray + j);
            mInstructions.push_back(instruction);
        }
         */

        DEBUG_BLOCK {
            Log::D("Instructions Read") << "End Time(ms): " << getCurrentTimeMs() << std::endl;
        };
    }


/*Instructions Iterator*/
Instruction Instructions::instruction_iterator::operator*(){
    
    // Query TLB
    auto& TLB = instrs_ptr->TLB;
    addr_t vir_addr = (instrs_ptr->instruction_start_addr +
                       it_index * INSTRUCTION_BYTE_WIDTH);
    addr_t phy_addr;
    auto it_tlb = TLB.begin();
    for(; it_tlb != TLB.end(); ++it_tlb){
        if(it_tlb->Tag == vir_addr &&
           it_tlb->Valid) break;
    }
    
    if(it_tlb == TLB.end()){
        // TLB Miss
        memcache::instr::IncrTLBMissCount();
        
        // Get phy addr from page table
        auto& VirPageTable = instrs_ptr->VirPageTable;
        auto it_page = VirPageTable.find(vir_addr);
        if(it_page == VirPageTable.end()){
            // Page fault
            memcache::instr::IncrPageMissCount();
            
            memcache::PhyPageEntry phy_entry;
            // Raw instructions offset
            phy_entry.PhyAddr = (it_index * INSTRUCTION_BYTE_WIDTH);
            
            phy_addr = instrs_ptr->phy_page_insert(phy_entry);
            memcache::PageTableEntry pte;
            pte.PhyAddr = phy_addr;
            VirPageTable[vir_addr] = pte;
        }else{
            // Page Hit
            memcache::instr::IncrPageHitCount();
            phy_addr = it_page->second.PhyAddr;
        }
        
        // Move to TLB
        memcache::TLBEntry tlb_entry;
        tlb_entry.Tag = vir_addr;
        tlb_entry.PhyAddr = phy_addr;
        tlb_entry.Valid = true;
        instrs_ptr->tlb_insert(tlb_entry);
        
    }else{
        // TLB Hit
        memcache::instr::IncrTLBHitCount();
        it_tlb->Use = true;
        phy_addr = it_tlb->PhyAddr;
    }
    
    // Cache Part
    auto& Cache = instrs_ptr->Cache;
    auto instr_index_width = instrs_ptr->instr_index_width;
    auto instr_offset_width = instrs_ptr->instr_offset_width;
    //auto instr_tag_width = instrs_ptr->instr_tag_width;
    
    auto index_value = extractInstrBits(vir_addr,
                                        instr_offset_width + instr_index_width - 1,
                                        instr_offset_width);
    auto tag_value = extractInstrBits(vir_addr,
                                      31,
                                      instr_offset_width + instr_index_width);
    auto it_cache = Cache.find(index_value);
    cache_entry_type::DataType* data = nullptr;
    if(it_cache == Cache.end()){
        // Cache Miss
        memcache::instr::IncrCacheMissCount();
        
        // New index entry
        auto& cache_sets = Cache[index_value];
        cache_sets.resize(memcache::instr::SetAssoc);
        
        // Move data from memory to cache
        const byte_t* raw_instr = instrs_ptr->raw_binary.getInstructions().content();
        
        cache_entry_type entry;
        entry.Tag = tag_value;
        entry.Data.resize(memcache::instr::BlockSize);
        auto* raw_data = entry.Data.data();
        ::memcpy(raw_data, raw_instr + phy_addr, memcache::instr::BlockSize);
        
        cache_sets[0] = entry;
        data = &cache_sets[0].Data;
    }else{
        auto& entry_list = it_cache->second;
        for(auto& entry : entry_list){
            if(entry.Tag == tag_value){
                // Cache Hit
                memcache::instr::IncrCacheHitCount();
                data = &entry.Data;
            }
        }
        
        if(data == nullptr){
            // Cache Miss
            int empty_index = 0;
            if(memcache::instr::SetAssoc != 1){
                // Not Direct Map
                empty_index = -1;
                bool full = true;
                for(int i = 0; i < entry_list.size(); i++){
                    if(!entry_list[i].Use && empty_index < 0){
                        empty_index = i;
                    }else if(!entry_list[i].Use){
                        full = false;
                    }
                }
                
                if(empty_index < 0){
                    // Something wrong...
                    empty_index = 0;
                }
                
                if(full){
                    for(auto& entry : entry_list){
                        entry.Use = false;
                    }
                }
            }
            
            cache_entry_type entry;
            entry.Use = true;
            entry.Tag = tag_value;
            entry.Data.resize(memcache::instr::BlockSize);
            auto* raw_data = entry.Data.data();
            const byte_t* raw_instr = instrs_ptr->raw_binary.getInstructions().content();
            ::memcpy(raw_data, raw_instr + phy_addr, memcache::instr::BlockSize);
            
            entry_list[empty_index] = entry;
            data = &entry_list[empty_index].Data;
        }
    }
    
    return Instruction(data->data());
}


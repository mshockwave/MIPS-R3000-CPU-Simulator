#ifndef _CMP_HPP_
#define _CMP_HPP_

#include <vector>
#include <map>
#include <initializer_list>
#include <tuple>

#include "Types.h"

namespace cmp {
    
    struct PhyPage {
        
        size_t DataOffset;
        bool Use;
        
        bool Ref;
        addr_t ReferVirAddr;
        
        PhyPage() :
        DataOffset(0),
        Ref(false), ReferVirAddr(0),
        Use(false){}
    };
    
    struct PageEntry{
        
        addr_t Tag;
        addr_t PhyAddr;
        bool Valid;
        bool Use;
        
        PageEntry() :
        Valid(false), Use(false),
        Tag(0), PhyAddr(0){}
    };
    
    struct CacheEntry{
        
        addr_t Tag;
        addr_t Index;
        
        bool Valid;
        bool Use;
        
        size_t DataOffset;
        
        CacheEntry() :
        Tag(0), Index(0),
        Valid(false), Use(false),
        DataOffset(0){}
    };
    
    class CMP{
        
    public:
        
        /*
         {block size, page size, mem size, cache size, set associate}
         */
        typedef std::initializer_list<size_t> cmp_config_t;
        
        CMP(cmp_config_t config,
            addr_t start_address,
            byte_t* data, size_t data_size);
        
        byte_t* Access(addr_t vir_addr){
            
            size_t vir_addr_offset = vir_addr % PageSize;
            
            auto tlb_result = tlb_access(vir_addr);
            if(std::get<1>(tlb_result)){
                // TLB Hit
                addr_t phy_addr = std::get<0>(tlb_result);
                size_t data_offset = phy_access(phy_addr);
                data_offset = data_offset * PageSize + vir_addr_offset;
                return disk_data + data_offset;
            }
            
            // TLB Miss
            if(tlb_miss(vir_addr)){
                // Do not count as TLB hit
                addr_t phy_addr = std::get<0>(tlb_result);
                size_t data_offset = phy_access(phy_addr);
                data_offset = data_offset * PageSize + vir_addr_offset;
                return disk_data + data_offset;
            }
            
            // Page fault
            page_fault(vir_addr);
            tlb_miss(vir_addr);
            {
                // Do not count as TLB miss
                addr_t phy_addr = std::get<0>(tlb_result);
                size_t data_offset = phy_access(phy_addr);
                data_offset = data_offset * PageSize + vir_addr_offset;
                return disk_data + data_offset;
            }
            
        }
        
    private:
        size_t BlockSize;
        size_t PageSize;
        size_t MemSize;
        size_t CacheSize;
        size_t SetAssoc;
        
        byte_t* disk_data;
        addr_t disk_data_start_addr;
        size_t disk_data_size;
        
        std::vector<PageEntry> TLB;
        std::map<addr_t, PageEntry> PageTable;
        std::vector<PhyPage> PhyPages;
        std::map<addr_t, std::vector<CacheEntry> > Cache;
        
        inline size_t pt_length(){ return disk_data_size / PageSize; }
        inline size_t tlb_length(){ return (pt_length() >> 2); }
        inline size_t phy_pages_length(){ return MemSize / PageSize; }
        inline size_t cache_length(){
            auto block_amount = CacheSize / BlockSize;
            return block_amount / SetAssoc;
        }
        
        inline addr_t pt_tag(addr_t vir_addr){
            return vir_addr / PageSize;
        }
        
        inline addr_t cache_block_offset(addr_t phy_addr){
            return phy_addr % BlockSize;
        }
        inline addr_t cache_index(addr_t phy_addr){
            auto tag_and_index = phy_addr / BlockSize;
            return tag_and_index % cache_length();
        }
        inline addr_t cache_tag(addr_t phy_addr){
            auto tag_and_index = phy_addr / BlockSize;
            return tag_and_index / cache_length();
        }
        
        std::tuple<addr_t,bool> tlb_access(addr_t vir_addr);
        bool tlb_miss(addr_t vir_addr);
        
        void page_fault(addr_t vir_addr);
        
        std::tuple<size_t,bool> cache_access(addr_t phy_addr);
        void cache_miss(addr_t phy_addr);
        
        size_t phy_access(addr_t phy_addr);
    };
    
} //namespace cmp

#endif
#include "CMP.hpp"

namespace cmp {
    
    CMP::CMP(cmp_config_t config,
             addr_t start_address,
             byte_t* data, size_t data_size) :
    disk_data_start_addr(start_address),
    disk_data(data),
    disk_data_size(data_size),
    /*Default values*/
    BlockSize(4),
    PageSize(4),
    MemSize(16),
    CacheSize(16),
    SetAssoc(1),
    LruSampleCounter(0),
    /*Profiling*/
    tlb_hit_count(0), tlb_miss_count(0),
    cache_hit_count(0), cache_miss_count(0),
    page_hit_count(0), page_fault_count(0){
        
        std::vector<size_t> config_vec(config);
        auto config_size = config_vec.size();
        if(config_size >= 1) BlockSize = config_vec[0];
        if(config_size >= 2) PageSize = config_vec[1];
        if(config_size >= 3) MemSize = config_vec[2];
        if(config_size >= 4) CacheSize = config_vec[3];
        if(config_size >= 5) SetAssoc = config_vec[4];
        
        // Init components
        TLB.resize(tlb_length(), PageEntry());
        for(addr_t offset = 0; offset < disk_data_size; offset += PageSize){
            auto addr = disk_data_start_addr + offset;
            PageTable[pt_tag(addr)] = PageEntry();
        }
        PhyPages.resize(phy_pages_length(), PhyPage());
        for(addr_t addr = 0; addr < cache_length(); addr++){
            std::vector<CacheEntry> cache_sets;
            cache_sets.resize(SetAssoc, CacheEntry());
            Cache[addr] = cache_sets;
        }
    }
    
    /*
     @return: (phy address, success with no TLB miss)
     */
    std::tuple<addr_t,bool> CMP::tlb_access(addr_t vir_addr){
        
        auto vm_tag = pt_tag(vir_addr);
        addr_t phy_addr = 0;
        
        for(auto& tlb_entry : TLB){
            if(tlb_entry.Tag == vm_tag &&
               tlb_entry.Valid){
                
                tlb_entry.LruCounter = LruSampleCounter;
                phy_addr = tlb_entry.PhyAddr;
                
                return std::make_tuple(phy_addr,true);
            }
        }
        
        return std::make_tuple(phy_addr, false);
    }
    
    /*
     @return: success with no page fault
     */
    bool CMP::tlb_miss(addr_t vir_addr){
        
        //tlb_miss_count++;
        
        auto vm_tag = pt_tag(vir_addr);
        auto& pt_entry = PageTable[vm_tag];
        if(!pt_entry.Valid) return false;
        
        //page_hit_count++;
        //pt_entry.Use = true;
        flip_phy_pages_mru(pt_entry.PhyAddr);
        
        // TLB insert
        ssize_t index = -1;
        for(size_t i = 0; i < TLB.size(); i++){
            const auto& tlb_entry = TLB[i];
            if(!tlb_entry.Valid){
                index = i;
                break;
            }
        }
        
        if(index < 0){
            // Pick according to LRU
            lru_counter_t min_lru_counter = LruSampleCounter + 1;
            for(size_t i = 0; i < TLB.size(); i++){
                const auto& tlb_entry = TLB[i];
                if(tlb_entry.LruCounter < min_lru_counter){
                    index = i;
                    min_lru_counter = tlb_entry.LruCounter;
                }
            }
        }
        
        if(index < 0) index = 0;
        TLB[index] = pt_entry;
        //TLB[index].Use = true;
        TLB[index].LruCounter = LruSampleCounter;
        TLB[index].Valid = true;
        
        return true;
    }
    
    void CMP::page_fault(addr_t vir_addr){
        
        //page_fault_count++;
        
        PhyPage phy_page;
        phy_page.Ref = true;
        //phy_page.Use = true;
        phy_page.LruCounter = LruSampleCounter;
        phy_page.Valid = true;
        phy_page.ReferVirAddr = vir_addr ;
        size_t vir_addr_offset = vir_addr - disk_data_start_addr;
        phy_page.DataOffset = vir_addr_offset - (vir_addr_offset % PageSize);
        
        
        // Find next phy mem entry
        ssize_t index = -1;
        for(size_t i = 0; i < PhyPages.size(); i++){
            const auto& phy_entry = PhyPages[i];
            if(!phy_entry.Valid){
                index = i;
                break;
            }
        }
        
        if(index < 0){
            // Use LRU to pick next page
            auto min_lru_counter = LruSampleCounter + 1;
            for(size_t i = 0; i < PhyPages.size(); i++){
                const auto& phy_entry = PhyPages[i];
                if(phy_entry.LruCounter < min_lru_counter){
                    index = i;
                    min_lru_counter = phy_entry.LruCounter;
                }
            }
            
        }
        
        if(index < 0) index = 0;
        
        const auto& phy_page_origin = PhyPages[index];
        if(phy_page_origin.Ref){
            // Purge original TLB
            auto vm_tag = pt_tag(phy_page_origin.ReferVirAddr);
            for(auto& tlb_entry : TLB){
                if(tlb_entry.Tag == vm_tag &&
                   tlb_entry.Valid){
                    // Purge
                    tlb_entry.Valid = false;
                    //tlb_entry.Use = false;
                }
            }
            
            // Purge original page entry
            for(auto& pt_entry : PageTable){
                if(pt_entry.first == vm_tag &&
                   pt_entry.second.Valid){
                    // Purge
                    pt_entry.second.Valid = false;
                    //pt_entry.second.Use = false;
                    break;
                }
            }
            
            // Purge original cache entry
            addr_t phy_addr_page_start = index * PageSize;
            addr_t phy_addr_page_end = phy_addr_page_start + PageSize;
            // Purge range: [start,end)
            for(addr_t addr = phy_addr_page_start;
                addr < phy_addr_page_end;
                addr++){
                
                auto cache_index_ = cache_index(addr);
                auto cache_tag_ = cache_tag(addr);
                auto& cache_sets = Cache[cache_index_];
                for(auto& cache_set : cache_sets){
                    if(cache_set.Tag == cache_tag_ &&
                       cache_set.Valid){
                        cache_set.Valid = false;
                        cache_set.Use = false;
                    }
                }
            }
        }
        
        PhyPages[index] = phy_page;
        //PhyPages[index].Use = true;
        PhyPages[index].LruCounter = LruSampleCounter;
        
        // Put result back to page table
        PageEntry pt_entry;
        pt_entry.PhyAddr = (index * PageSize);
        pt_entry.Tag = pt_tag(vir_addr);
        pt_entry.Valid = true;
        PageTable[pt_entry.Tag] = pt_entry;
    }
    
    /*
     @return: (disk data offset, success without cache miss)
     */
    std::tuple<size_t,bool> CMP::cache_access(addr_t phy_addr){
        
        auto cache_index_ = cache_index(phy_addr);
        auto cache_tag_ = cache_tag(phy_addr);
        
        auto& cache_sets = Cache[cache_index_];
        ssize_t index = -1;
        bool full = true;
        for(size_t i = 0; i < cache_sets.size(); i++){
            auto& cache_set = cache_sets[i];
            if(cache_set.Tag == cache_tag_ &&
               cache_set.Valid){
                
                index = i;
                
            }else if(!cache_set.Use){
                full = false;
            }
        }
        if(index < 0) return std::make_tuple(0, false);
        
        if(full){
            for(auto& set : cache_sets){
                set.Use = false;
            }
        }
        
        auto& hit_set = cache_sets[index];
        hit_set.Use = true;
        return std::make_tuple(hit_set.DataOffset, true);
    }
    
    void CMP::cache_miss(addr_t phy_addr){
        
        //cache_miss_count++;
        
        auto cache_index_ = cache_index(phy_addr);
        auto cache_tag_ = cache_tag(phy_addr);
        
        auto& cache_sets = Cache[cache_index_];
        // Find set to insert
        ssize_t index = -1;
        for(size_t i = 0; i < cache_sets.size(); i++){
            const auto& set = cache_sets[i];
            if(!set.Valid){
                index = i;
                break;
            }
        }
        
        bool full = true;
        if(index < 0){
            for(size_t i = 0; i < cache_sets.size(); i++){
                const auto& set = cache_sets[i];
                if(!set.Use &&
                   index < 0){
                    index = i;
                }else if(!set.Use){
                    full = false;
                }
            }
            
            
        }else{
            for(size_t i = 0; i < cache_sets.size(); i++){
                if(i == index) continue;
                
                if(!cache_sets[i].Use){
                    full = false;
                    break;
                }
            }
        }
        
        if(full){
            for(auto& set : cache_sets){
                set.Use = false;
            }
        }
        
        if(index < 0) index = 0;
        
        //flip_phy_pages_mru(phy_addr);
        
        size_t phy_page_index = phy_addr / PageSize;
        size_t phy_page_data_offset = PhyPages[phy_page_index].DataOffset;
        size_t phy_addr_offset = phy_addr % PageSize; // Offset within a page
        size_t cache_block_index = phy_addr_offset / BlockSize;
        
        CacheEntry new_cache_entry;
        new_cache_entry.DataOffset = phy_page_data_offset + (cache_block_index * BlockSize);
        new_cache_entry.Tag = cache_tag_;
        new_cache_entry.Index = cache_index_;
        new_cache_entry.Valid = true;
        //new_cache_entry.Use = true;
        cache_sets[index] = new_cache_entry;
    }
    
    /*
     @return: data offset of phy page
     */
    size_t CMP::phy_access(addr_t phy_addr){
        
        auto result = cache_access(phy_addr);
        if(std::get<1>(result)){
            // Cache Hit
            cache_hit_count++;
            auto data_offset_block = std::get<0>(result);
            return data_offset_block + cache_block_offset(phy_addr);
        }
        
        // Cache Miss
        cache_miss_count++;
        cache_miss(phy_addr);
        result = cache_access(phy_addr);
        auto data_offset_block = std::get<0>(result);
        return data_offset_block + cache_block_offset(phy_addr);
    }
    
} //namespace cmp
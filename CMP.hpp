#ifndef _CMP_HPP_
#define _CMP_HPP_

#include <vector>
#include <map>
#include <initializer_list>
#include <tuple>
#include <memory>

#include "Types.h"

namespace cmp {
    
    typedef unsigned long lru_counter_t;
    
    
    struct LruEntity {
        
        size_t Index;
        lru_counter_t LruCounter;
        
        LruEntity* Prev;
        LruEntity* Next;
        
        LruEntity() :
        Prev(nullptr), Next(nullptr),
        Index(0), LruCounter(0){}
        
        inline void Remove(){
            if(Prev != nullptr){
                Prev->Next = Next;
            }
            
            if(Next != nullptr){
                Next->Prev = Prev;
            }
            
            Prev = Next = nullptr;
        }
        
        inline void InsertAfter(LruEntity* element){
            Prev = element;
            
            if(Prev != nullptr && Prev->Next != nullptr){
                Prev->Next->Prev = this;
            }
            
            if(Prev != nullptr){
                Next = Prev->Next;
                Prev->Next = this;
            }
        }
    };
    
    struct LruListManager {
        LruEntity* Head;
        LruEntity* Tail;
        
        LruListManager() :
        Head(nullptr), Tail(nullptr){}
        
        ~LruListManager(){
            while(Head != Tail && Tail != nullptr){
                auto* element = Tail;
                Tail = Tail->Prev;
                if(element != nullptr) delete element;
            }
            
            if(Head != nullptr) delete Head;
        }
        
        LruEntity* AddNew(size_t index){
            auto* element = new LruEntity();
            element->Index = index;
            element->Next = nullptr;
            
            if(Head == nullptr || Tail == nullptr){
                element->Prev = nullptr;
                Head = element;
                Tail = Head;
            }else{
                element->InsertAfter(Tail);
                Tail = element;
            }
            
            return element;
        }
    };
    
    template<typename list_mngr_ptr_t>
    void UpdateLru(list_mngr_ptr_t list_manager, LruEntity* entity,
                   lru_counter_t current_counter){
        entity->LruCounter = current_counter;
        
        if(list_manager->Head == entity &&
           list_manager->Head != list_manager->Tail){
            list_manager->Head = entity->Next;
        }
        if(list_manager->Tail == entity &&
           list_manager->Head != list_manager->Tail){
            list_manager->Tail = entity->Prev;
        }
        entity->Remove();
        
        auto* tail = list_manager->Tail;
        auto* ptr = tail;
        while(ptr->LruCounter == entity->LruCounter &&
              ptr->Index > entity->Index){
            ptr = ptr->Prev;
        }
        entity->InsertAfter(ptr);
        
        if(entity->Next == nullptr) list_manager->Tail = entity;
    }
    
    struct PhyPage {
        
        size_t DataOffset;
        //bool Use;
        //lru_counter_t LruCounter;
        LruEntity* Lru;
        bool Valid;
        
        bool Ref;
        addr_t ReferVirAddr;
        
        PhyPage() :
        DataOffset(0),
        Ref(false), ReferVirAddr(0),
        /*Use(false),*/Valid(false),
        Lru(nullptr)/*, LruCounter(0)*/{}
    };
    
    struct PageEntry{
        
        addr_t Tag;
        addr_t PhyAddr;
        bool Valid;
        //bool Use;
        //lru_counter_t LruCounter;
        LruEntity* Lru;
        
        PageEntry() :
        Valid(false), /*Use(false),*/
        Lru(nullptr)/*, LruCounter(0)*/,
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
        
        typedef unsigned long profile_count_t;
        /*(hit,miss)*/
        typedef std::tuple<profile_count_t, profile_count_t> profile_result_t;
        
        CMP(cmp_config_t config,
            addr_t start_address,
            byte_t* data, size_t data_size);
        
        byte_t* Access(addr_t vir_addr){
            
            LruSampleCounter++;
            
            size_t vir_addr_offset = (vir_addr - disk_data_start_addr) % PageSize;
            
            auto tlb_result = tlb_access(vir_addr);
            if(std::get<1>(tlb_result)){
                // TLB Hit
                tlb_hit_count++;
                
                addr_t phy_page_addr = std::get<0>(tlb_result);
                addr_t phy_addr = phy_page_addr + vir_addr_offset;
                size_t data_offset = phy_access(phy_addr);
                return disk_data + data_offset;
            }
            
            // TLB Miss
            tlb_miss_count++;
            if(tlb_miss(vir_addr)){
                page_hit_count++;
                
                tlb_result = tlb_access(vir_addr);
                addr_t phy_page_addr = std::get<0>(tlb_result);
                addr_t phy_addr = phy_page_addr + vir_addr_offset;
                size_t data_offset = phy_access(phy_addr);
                return disk_data + data_offset;
            }
            
            // Page fault
            page_fault_count++;
            page_fault(vir_addr);
            tlb_miss(vir_addr);
            tlb_result = tlb_access(vir_addr);
            {
                addr_t phy_page_addr = std::get<0>(tlb_result);
                addr_t phy_addr = phy_page_addr + vir_addr_offset;
                size_t data_offset = phy_access(phy_addr);
                return disk_data + data_offset;
            }
            
        }
        
        inline addr_t GetStartAddr() const { return disk_data_start_addr; }
        
        inline profile_result_t GetCacheProfileData() const{
            return std::make_tuple(cache_hit_count,cache_miss_count);
        }
        inline profile_result_t GetTLBProfileData() const{
            return std::make_tuple(tlb_hit_count, tlb_miss_count);
        }
        inline profile_result_t GetPageProfileData() const{
            return std::make_tuple(page_hit_count, page_fault_count);
        }
        
        inline void IncreaseAllProfiles(){
            cache_miss_count++;
            tlb_miss_count++;
            page_fault_count++;
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
        std::shared_ptr<LruListManager> LruTLB;
        
        std::map<addr_t, PageEntry> PageTable;
        
        std::vector<PhyPage> PhyPages;
        std::shared_ptr<LruListManager> LruPhyPages;
        
        std::map<addr_t, std::vector<CacheEntry> > Cache;
        
        lru_counter_t LruSampleCounter;
        
        // Profiling
        profile_count_t tlb_miss_count, tlb_hit_count;
        profile_count_t cache_miss_count, cache_hit_count;
        profile_count_t page_hit_count, page_fault_count;
        
        inline size_t pt_length(){ return disk_data_size / PageSize; }
        inline size_t tlb_length(){ return (pt_length() >> 2); }
        inline size_t phy_pages_length(){ return MemSize / PageSize; }
        inline size_t cache_length(){
            auto block_amount = CacheSize / BlockSize;
            return block_amount / SetAssoc;
        }
        
        inline addr_t pt_tag(addr_t vir_addr){
            return (vir_addr - disk_data_start_addr) / PageSize;
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
        
        inline void flip_phy_pages_mru(addr_t phy_addr){
            
            size_t phy_page_index = (phy_addr / PageSize);
            
            //PhyPages[phy_page_index].LruCounter = LruSampleCounter;
            UpdateLru(LruPhyPages, PhyPages[phy_page_index].Lru, LruSampleCounter);
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
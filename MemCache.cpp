#include "MemCache.hpp"

namespace memcache {
    
    namespace instr{
        
        uint32_t MemSize,
                PageSize,
                BlockSize,
                CacheSize;
        
        int SetAssoc;
        
        unsigned long tlb_miss = 0L, tlb_hit = 0L;
        void IncrTLBMissCount(){ tlb_miss++; }
        void IncrTLBHitCount(){ tlb_hit++; }
        profile_result_t GetTLBProfileResult(){
            return std::make_tuple(tlb_miss, tlb_hit);
        }
        
        unsigned long cache_miss = 0L, cache_hit = 0L;
        void IncrCacheMissCount(){ cache_miss++; }
        void IncrCacheHitCount(){ cache_hit++; }
        profile_result_t GetCacheProfileResult(){
            return std::make_tuple(cache_miss, cache_hit);
        }
        
        unsigned long pt_miss = 0L, pt_hit = 0L;
        void IncrPageMissCount(){ pt_miss++; }
        void IncrPageHitCount(){ pt_hit++; }
        profile_result_t GetPageTableProfileResult(){
            return std::make_tuple(pt_miss, pt_hit);
        }
        
    } //namespace instr
    
    namespace data{
        
        uint32_t MemSize,
                PageSize,
                BlockSize,
                CacheSize;
        
        int SetAssoc;
        
        unsigned long tlb_miss = 0L, tlb_hit = 0L;
        void IncrTLBMissCount(){ tlb_miss++; }
        void IncrTLBHitCount(){ tlb_hit++; }
        profile_result_t GetTLBProfileResult(){
            return std::make_tuple(tlb_miss, tlb_hit);
        }
        
        unsigned long cache_miss = 0L, cache_hit = 0L;
        void IncrCacheMissCount(){ cache_miss++; }
        void IncrCacheHitCount(){ cache_hit++; }
        profile_result_t GetCacheProfileResult(){
            return std::make_tuple(cache_miss, cache_hit);
        }
        
        unsigned long pt_miss = 0L, pt_hit = 0L;
        void IncrPageMissCount(){ pt_miss++; }
        void IncrPageHitCount(){ pt_hit++; }
        profile_result_t GetPageTableProfileResult(){
            return std::make_tuple(pt_miss, pt_hit);
        }
    } //namespace data
    
} //namespace memcache
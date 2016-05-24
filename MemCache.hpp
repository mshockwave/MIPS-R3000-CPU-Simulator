#ifndef _MEM_CACHE_HPP
#define _MEM_CACHE_HPP

#include <cstdint>
#include <vector>
#include <tuple>

#include "Types.h"

namespace memcache {
    
    //(miss, hit)
    typedef std::tuple<unsigned long, unsigned long> profile_result_t;
    
    namespace instr{
        
        extern uint32_t MemSize,
                        PageSize,
                        BlockSize,
                        CacheSize;
        extern int SetAssoc;
        
        void IncrTLBMissCount();
        void IncrTLBHitCount();
        profile_result_t GetTLBProfileResult();
        
        void IncrCacheMissCount();
        void IncrCacheHitCount();
        profile_result_t GetCacheProfileResult();
        
        void IncrPageMissCount();
        void IncrPageHitCount();
        profile_result_t GetPageTableProfileResult();
        
    } //namespace instr
    
    namespace data{
        extern uint32_t MemSize,
                        PageSize,
                        BlockSize,
                        CacheSize;
        extern int SetAssoc;
        
        void IncrTLBMissCount();
        void IncrTLBHitCount();
        profile_result_t GetTLBProfileResult();
        
        void IncrCacheMissCount();
        void IncrCacheHitCount();
        profile_result_t GetCacheProfileResult();
        
        void IncrPageMissCount();
        void IncrPageHitCount();
        profile_result_t GetPageTableProfileResult();
        
    } //namespace data
    
    struct TLBEntry{
        
        bool Valid;
        bool Dirty;
        bool Use;
        
        addr_t Tag;
        addr_t PhyAddr;
        
        TLBEntry() :
            Valid(false),
            Dirty(false),
            Use(false),
            Tag(0),
            PhyAddr(0){}
    };
    
    struct PhyPageEntry{
        
        bool Valid;
        bool Dirty;
        bool Use;
        
        addr_t PhyAddr;
        
        PhyPageEntry() :
            Dirty(false),
            Use(false),
            Valid(false),
            PhyAddr(0){}
    };
    
    struct PageTableEntry{
        
        addr_t PhyAddr;
        
        PageTableEntry() : PhyAddr(0){}
    };
    
    template< typename data_type = std::vector<byte_t> >
    struct CacheEntry{
        
        typedef data_type DataType;
        
        addr_t Tag;
        data_type Data;
        bool Use;
        
        CacheEntry<data_type>() :
            Tag(0),
            Data(),
            Use(false){}
    };
    
} //namespace memcache

#endif
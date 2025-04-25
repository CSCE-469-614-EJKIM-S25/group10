#ifndef MOCKINGJAY_REPL_H_
#define MOCKINGJAY_REPL_H_

#include "repl_policies.h"
#include <unordered_map>
#include <stdlib.h>
using namespace std;

// Static RRIP
class MockingjayReplPolicy : public ReplPolicy {
    protected:
        // add class member variables here
        uint32_t numLines; //number of total cache lines (including those in the same set)
        uint32_t llc_ways; //number of ways
        uint32_t lineSize;

        int INF_ETR = 127; //etr value for percieved scans
        int MAX_RD = 104; //etr threshold for non-scan values
        int GRANULARITY = 8; //how many accesses before decrementing etr's

        static constexpr int NUM_SAMPLED_SETS = 512;
        static constexpr int NUM_SAMPLED_WAYS = 5;
        static constexpr int kTimestampBits = 8;
        static constexpr int kMaxTimestamp = 1 << kTimestampBits;

        int * etr; //etr counters for all cache lines
        int * etr_clock;

        struct SampledCacheLine {
          bool valid;
          uint64_t tag;
          uint64_t last_pc_signature;
          int timestamp;
        };

        SampledCacheLine sampledCacheEntries[NUM_SAMPLED_SETS][NUM_SAMPLED_WAYS];

        int findInSampledCache(uint64_t tag, uint32_t setIndex) {
          for (int i = 0; i < NUM_SAMPLED_WAYS; ++i) {
              if (sampledCacheEntries[setIndex][i].valid && sampledCacheEntries[setIndex][i].tag == tag) {
                  return i;
              }
          }
          return -1; // not found
        }

        // Update timestamp with wrap-around behavior
        inline int updateTimestamp(int current) { return (current + 1) % kMaxTimestamp; }

        // Compute reuse interval considering wrap-around
        inline int computeElapsedTime(int recent, int previous) {
          if (recent >= previous) { return recent - previous; } 
          else { return (recent + kMaxTimestamp) - previous; }
        }

        int findLRUOrInvalid(uint32_t setIndex) {
          int lruWay = 0;
          int oldestTs = INT32_MAX;
      
          for (int i = 0; i < NUM_SAMPLED_WAYS; ++i) {
              auto& entry = sampledCacheEntries[setIndex][i];
              if (!entry.valid) return i; // use invalid immediately
      
              if (entry.timestamp < oldestTs) {
                  oldestTs = entry.timestamp;
                  lruWay = i;
              }
          }
          return lruWay;
        }
        
    public:
        //we may want to change this
        explicit MockingjayReplPolicy(uint32_t _numLines, uint32_t _numWays, uint32_t _lineSize) : numLines(_numLines), llc_ways(_numWays), lineSize(_lineSize) {
            etr = gm_calloc<int>(numLines);
            etr_clock = gm_calloc<int>(numLines/llc_ways); //counter for amount of accesses to that set
        }
        
        ~MockingjayReplPolicy() {
            gm_free(etr_clock);
            gm_free(etr);
        }
        
        // add member methods here, refer to repl_policies.h
        void update(uint32_t id, const MemReq* req) {
          //update values if accesses = GRANULARITY
          int set = (id/llc_ways); //size of etr array is sets*ways, effectively getting bits for set here
          if(etr_clock[set] == GRANULARITY){
            uint32_t start = (id/llc_ways)*llc_ways; //id of first element in set (used integer devision to round down to multiple of ways)
            //loop through set and decrement
            for(int i = start; i < (start + llc_ways); i++){
              etr[i]--;
            }
            etr_clock[set] = 0;
          }
          else{
            etr_clock[set]++;
          }
        }
        
        void replaced(uint32_t id) {
          //mark as a miss
        }
        
        //find victim from set
        template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
          int max_etr = 0;
          int victim_loc = 0;
          for(auto ci = cands.begin(); ci != cands.end(); ci++){
            if(abs(etr[*ci]) > max_etr || (abs(etr[*ci]) == max_etr && etr[*ci] < 0)){
              victim_loc = *ci;
              max_etr = abs(etr[*ci]);
            }
          }

          //may need prefetching logic here

          return victim_loc;
        }
        DECL_RANK_BINDINGS;
};
#endif // MOCKINGJAY_REPL_H_

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

        int * etr; //etr counters for all cache lines
        int * etr_clock;

        int INF_ETR = 127; //etr value for percieved scans
        int MAX_RD = 104; //etr threshold for non-scan values
        int GRANULARITY = 8; //how many accesses before decrementing etr's
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
          if(etr_clock[(req->lineAddr >> (uint32_t)(log2(lineSize) + log2(llc_ways)) )] == GRANULARITY){
            //TODO: loop through items in the same set and decrement the etr
            //problem is accessing things from same set as we don't know line IDs

            etr_clock[(req->lineAddr >> (lineSize + llc_ways))] = 0;
          }
          else{
            etr_clock[(req->lineAddr >> (lineSize + llc_ways))]++;
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

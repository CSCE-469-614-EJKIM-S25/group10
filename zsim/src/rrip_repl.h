#ifndef RRIP_REPL_H_
#define RRIP_REPL_H_

#include "repl_policies.h"

// Static RRIP - originally implemented by Ben Armstrong
class SRRIPReplPolicy : public ReplPolicy {
    protected:
        // add class member variables here
        uint32_t* array;
        uint32_t numLines;
        uint32_t maxM;
    public:
        explicit SRRIPReplPolicy(uint32_t _numLines, uint32_t _maxM) : numLines(_numLines), maxM(_maxM) {
            array = gm_calloc<uint32_t>(numLines);
        }
        
        ~SRRIPReplPolicy() {
            gm_free(array);
        }
        
        // add member methods here, refer to repl_policies.h
        void update(uint32_t id, const MemReq* req) {
          if(array[id] == maxM + 1){ //maxM + 1 would be impossible, but I'm using it to denote just replaced
            //would not be necessary in hardware as you could have a miss set this to 3 without a check like this
            array[id] = maxM - 1;
          }
          else{ //not recently replaced
            array[id] = 0;
          }
        }
        
        void replaced(uint32_t id) { //when a block is replaced, mark it as replaced so it gets updated as maxM
          array[id] = maxM + 1;
        }
        
        template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
            while(true){
              for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
                if(array[*ci] == maxM) return *ci; //return first location of maxM
              }
              for(uint32_t i = 0; i < numLines;i++) {
                if(array[i] != maxM) ++array[i]; //increment all if no maxM
              }
            }
            return 0;
        }
        DECL_RANK_BINDINGS;
};
#endif // RRIP_REPL_H_

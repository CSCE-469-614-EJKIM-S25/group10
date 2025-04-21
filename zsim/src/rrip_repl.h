#ifndef RRIP_REPL_H_
#define RRIP_REPL_H_

#include "repl_policies.h"

// Static RRIP
class SRRIPReplPolicy : public ReplPolicy {
    protected: // LRU has global relative recency hence a timestamp -- srrip is local so we can just assign each array[id] to local rrpv
    uint32_t numLines;
    uint32_t rrpvMax;
    uint64_t* array;

    public:
        explicit SRRIPReplPolicy(uint32_t _numLines, uint32_t _rrpvMax) : numLines(_numLines), rrpvMax(_rrpvMax) {
            array = gm_calloc<uint64_t>(numLines);
        }
    
        ~SRRIPReplPolicy() { gm_free(array); }

        void update(uint32_t id, const MemReq* req) override {
            // If the line was just inserted (cold), don’t override it
            if (array[id] != rrpvMax - 1) {
                array[id] = 0;
            }
        }
        
        void replaced(uint32_t id) { array[id] = rrpvMax - 1; } // make the newly inserted line have LONG re-reference interval

        template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
            while (true){
                for (auto ci = cands.begin(); ci != cands.end(); ci.inc()){
                    if (array[*ci] == rrpvMax){ return *ci; }
                }
                for (auto ci = cands.begin(); ci != cands.end(); ci.inc()){ 
                    array[*ci] = MIN(rrpvMax, array[*ci] + 1);
                }
            }
            assert(false && "SRRIP rank() should never reach here");
            return -1;
        }

        DECL_RANK_BINDINGS;
};
#endif // RRIP_REPL_H_

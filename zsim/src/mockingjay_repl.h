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
        uint32_t numCores;

        /* CONSTANTS */

        // defined by original mockingjay -- will be initialized in constructor by default Zsim values
        uint32_t LOG2_BLOCK_SIZE; // aka log2 of line size
        uint32_t LOG2_LLC_SET;
        uint32_t LOG2_LLC_SIZE;

        uint32_t SAMPLED_CACHE_TAG_BITS;
        uint32_t PC_SIGNATURE_BITS;

        static constexpr int NUM_SAMPLED_SETS = 512;
        static constexpr int NUM_SAMPLED_WAYS = 5;
        static constexpr int LOG2_SAMPLED_CACHE_SETS = 4;

        static constexpr int kTimestampBits = 8;
        static constexpr int kMaxTimestamp = 1 << kTimestampBits;

        static constexpr double TEMP_DIFFERENCE = 1.0/16.0;
        double FLEXMIN_PENALTY; // for prefetching

        int INF_ETR = 15;
        int INF_RD = 127; // reuse distance for percieved scans
        int MAX_RD = 104; // reuse distnace threshold for non-scan values
        int GRANULARITY = 8; //how many accesses before decrementing etr's

        
        /* DATA STRUCTURES */

        struct SampledCacheLine {
          bool valid;
          uint64_t tag;
          uint64_t last_pc_signature;
          int timestamp;
        };

        SampledCacheLine sampledCacheEntries[NUM_SAMPLED_SETS][NUM_SAMPLED_WAYS];

        int * etr; //etr counters for all cache lines
        int * etr_clock;

        unordered_map<uint32_t, int> rdp; // reuse distance predictor -- PC signature as index and stores predicted reuse distance of blocks mapping to signature

        int* current_timestamp; // one timestamp per set in the LLC (numLines / llc_ways)

        /* HELPER FUNCTIONS */

        bool is_sampled_set(int set){
          uint32_t mask_length = LOG2_LLC_SET - NUM_SAMPLED_SETS;
          uint32_t mask = (1 << mask_length) - 1;
          return (set & mask) == ((set >> (LOG2_LLC_SET - mask_length)) & mask);
        }

        uint64_t CRC_HASH(uint64_t pc){
          static const unsigned long long crcPolynomial = 3988292384ULL;
          unsigned long long _returnVal = pc;
          for( unsigned int i = 0; i < 3; i++) _returnVal = ((_returnVal & 1) == 1) ? (( _returnVal >> 1 ) ^ crcPolynomial) : (_returnVal >> 1);
          return _returnVal;
        }

        uint64_t get_pc_signature(uint64_t pc, bool hit, bool prefetch, uint32_t core){
          if (numCores == 1){
            pc = pc << 1;
            if(hit){ pc = pc | 1; }
            pc = pc << 1;
            if (prefetch){ pc = pc | 1; }
            pc = CRC_HASH(pc);
          } 
          else{ // multicore sim
            pc = pc << 1;
            if(prefetch) { pc = pc | 1; }
            pc = pc << 2;
            pc = pc | core;
            pc = CRC_HASH(pc);
          }
          pc = (pc << (64 - PC_SIGNATURE_BITS)) >> (64 - PC_SIGNATURE_BITS);
          return pc;
        }

        uint32_t get_sampled_cache_index(uint64_t full_addr) {
          full_addr = full_addr >> LOG2_BLOCK_SIZE;
          full_addr = (full_addr << (64 - (LOG2_SAMPLED_CACHE_SETS + LOG2_LLC_SET))) >> (64 - (LOG2_SAMPLED_CACHE_SETS + LOG2_LLC_SET));
          return full_addr;
        }
      
        uint64_t get_sampled_cache_tag(uint64_t full_addr) {
          full_addr = full_addr >> LOG2_LLC_SET + LOG2_BLOCK_SIZE + LOG2_SAMPLED_CACHE_SETS;
          full_addr = (full_addr << (64 - SAMPLED_CACHE_TAG_BITS)) >> (64 - SAMPLED_CACHE_TAG_BITS);
          return full_addr;
        }

        int findInSampledCache(uint64_t tag, uint32_t setIndex) { // tag is the BLOCK ADDR
          for (int i = 0; i < NUM_SAMPLED_WAYS; ++i) {
              if (sampledCacheEntries[setIndex][i].valid && (sampledCacheEntries[setIndex][i].tag == tag)) {
                  return i;
              }
          }
          return -1; // not found
        }

        void detrain(uint32_t set, int way){
          SampledCacheLine temp = sampledCacheEntries[set][way];
          if (!temp.valid){ return; } // we already de-trained this entry

          if (rdp.count(temp.last_pc_signature)){ rdp.at(temp.last_pc_signature) = min(rdp.at(temp.last_pc_signature) + 1, INF_RD); } 
          else{ rdp[temp.last_pc_signature] = INF_RD; } // signature is not an index in the map -- add a new entry
          sampledCacheEntries[set][way].valid = false;
        }

        int temporal_difference(int init, int sample) {
          int diff = abs(sample - init);
          diff = diff * TEMP_DIFFERENCE;
          diff = min(1, diff);
          if (sample > init){ return min(init + diff, INF_RD); }
          else if (sample < init){ return max(init - diff, 0); }
          else{ return init; }
        }

        // Update timestamp with wrap-around behavior
        inline int updateTimestamp(int current) { return (current + 1) % kMaxTimestamp; }

        // Compute reuse interval considering wrap-around
        inline int computeElapsedTime(int recent, int previous) {
          if (recent >= previous) { return recent - previous; } 
          else { return (recent + kMaxTimestamp) - previous; }
        }

    public:
        //we may want to change this
        explicit MockingjayReplPolicy(uint32_t _numLines, uint32_t _numWays, uint32_t _lineSize, uint32_t _numCores) 
          : numLines(_numLines), llc_ways(_numWays), lineSize(_lineSize), numCores(_numCores) {

            LOG2_LLC_SET = ilog2(numLines/llc_ways);
            LOG2_BLOCK_SIZE = ilog2(lineSize);
            LOG2_LLC_SIZE = LOG2_LLC_SET + ilog2(llc_ways) + LOG2_BLOCK_SIZE;

            SAMPLED_CACHE_TAG_BITS = 31 - LOG2_LLC_SIZE;
            PC_SIGNATURE_BITS = LOG2_LLC_SIZE - 10;
            FLEXMIN_PENALTY = 2.0 - ilog2(numCores)/4.0;

            etr = gm_calloc<int>(numLines);
            etr_clock = gm_calloc<int>(numLines/llc_ways); //counter for amount of accesses to that set
            current_timestamp = gm_calloc<int>(numLines/llc_ways); // stores the most recent accessed timestamp per set

            for(int i = 0; i < numLines/llc_ways; i++) {
              etr_clock[i] = GRANULARITY;
              current_timestamp[i] = 0;
            }

        }
        
        ~MockingjayReplPolicy() {
            gm_free(etr_clock);
            gm_free(etr);
            gm_free(current_timestamp);
        }
        
        // add member methods here, refer to repl_policies.h
        void update(uint32_t id, const MemReq* req) { // id is the SPECIFIC LINE we need to access (a WAY of a set)
          if (req->type == PUTS || req->type == PUTX){ etr[id] = -INF_ETR; } // writeback special case

          bool hit = (req->type == GETS || req->type == GETX);
          bool prefetch = req->is(MemReq::PREFETCH);
          uint32_t cpu = req->srcId; // the CPU that requested this
          uint32_t pc = get_pc_signature(req->pc, hit, prefetch, cpu);

          int set = (id/llc_ways); //size of etr array is sets*ways, effectively getting bits for set here

          if (is_sampled_set(set)){
            uint32_t sampled_cache_index = get_sampled_cache_index(req->lineAddr);
            uint64_t sampled_cache_tag = get_sampled_cache_tag(req->lineAddr);
            int sampled_cache_way = findInSampledCache(sampled_cache_tag, sampled_cache_index);
    
            if (sampled_cache_way > -1){ // it exists in the sampled cache and we should update RDP
              uint64_t last_signature = sampledCacheEntries[sampled_cache_index][sampled_cache_way].last_pc_signature;
              uint64_t last_timestamp = sampledCacheEntries[sampled_cache_index][sampled_cache_way].timestamp;
              int sample = computeElapsedTime(current_timestamp[set], last_timestamp);

              if (sample <= INF_RD){
                if (prefetch) { sample *= FLEXMIN_PENALTY; }
                if (rdp.count(last_signature)){
                  int init = rdp.at(last_signature);
                  rdp.at(last_signature) = temporal_difference(init, sample);
                }
                else{ rdp[last_signature] = sample; }

                sampledCacheEntries[sampled_cache_index][sampled_cache_way].valid = false; // setting up so that detrain skips this entry
              }
            }

            int lruWay = -1;
            int lru_rd = -1;
            for (int i = 0; i < NUM_SAMPLED_WAYS; ++i) {
              auto& entry = sampledCacheEntries[sampled_cache_index][i];
              if (!entry.valid){
                lruWay = i;
                lru_rd = INF_RD + 1;
                continue;  
              }

              uint64_t last_timestamp = entry.timestamp;
              int sample = computeElapsedTime(current_timestamp[set], last_timestamp);
              if (sample > INF_RD){
                lruWay = i;
                lru_rd = INF_RD + 1;
                detrain(sampled_cache_index, i); // we are seeing this as a SCAN ACCESS so don't involve it in RDP
              }
              else if (sample > lru_rd){
                lruWay = i;
                lru_rd = sample;
              }
            }
            detrain(sampled_cache_index, lruWay);

            for (int w = 0; w < NUM_SAMPLED_WAYS; w++) {
              if (sampledCacheEntries[sampled_cache_index][w].valid == false){
                sampledCacheEntries[sampled_cache_index][w].valid = true;
                sampledCacheEntries[sampled_cache_index][w].last_pc_signature = pc;
                sampledCacheEntries[sampled_cache_index][w].tag = sampled_cache_tag;
                sampledCacheEntries[sampled_cache_index][w].timestamp = current_timestamp[set];
                break; // SO ONLY UPDATE THE FIRST WAY THAT HAS AN INVALID TAG (not sure why)
              }
            }
            current_timestamp[set] = updateTimestamp(current_timestamp[set]);
          }

          //update values if accesses = GRANULARITY
          if(etr_clock[set] == GRANULARITY){
            uint32_t start = (id/llc_ways)*llc_ways; //id of first element in set (used integer devision to round down to multiple of ways)
            //loop through set and decrement
            for(uint32_t i = start; i < (start + llc_ways); i++){
              if ((uint32_t) i != id && abs(etr[i]) < INF_ETR){ etr[i]--; }
            }
            etr_clock[set] = 0;
          }
          else{
            etr_clock[set]++;
          }

          if (id < llc_ways){
            if(!rdp.count(pc)){
              if (numCores == 1) { etr[id] = 0; }
              else{ etr[id] = INF_ETR; }
            }
            else{ // there IS an entry for this PC in the RDP
                if(rdp.at(pc) > MAX_RD) { etr[id] = INF_ETR; } // this entry has CROSSED the treshold FOR NON-SCAN-iness -- we treat it as junk
                else{ etr[id] = rdp.at(pc) / GRANULARITY; }
            }
          }
        }
        
        void replaced(uint32_t id) { etr[id] = 0; }
        
        //find victim from set
        template <typename C> inline uint32_t rank(const MemReq* req, C cands) {
          /* IF A LINE IS INVALID WE CAN JUST EVICT THAT */
          for(auto ci = cands.begin(); ci != cands.end(); ci.inc()){
            if (!cc->isValid(*ci)){ return *ci; } // cc is coherence controller and it will specify if the line is valid or not
          }

          int max_etr = 0;
          int victim_loc = 0;
          for(auto ci = cands.begin(); ci != cands.end(); ci.inc()){
            if(abs(etr[*ci]) > max_etr || (abs(etr[*ci]) == max_etr && etr[*ci] < 0)){
              victim_loc = *ci;
              max_etr = abs(etr[*ci]);
            }
          }
          
          uint64_t pc_signature = get_pc_signature(req->pc, false, req->is(MemReq::PREFETCH), req->srcId);
          bool writeback = req->type == PUTS || req->type == PUTX;
          if (!writeback && rdp.count(pc_signature) && (rdp.at(pc_signature) > MAX_RD || rdp.at(pc_signature) / GRANULARITY > max_etr)){ return llc_ways; }
          
          return victim_loc;
        }
        DECL_RANK_BINDINGS;
};
#endif // MOCKINGJAY_REPL_H_
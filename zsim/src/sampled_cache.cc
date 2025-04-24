#include "sampled_cache.hh"
#include <cmath>
#include <algorithm>

SampledCache::SampledCache()
    : cache(TOTAL_SETS, std::vector<SampledCacheEntry>(ASSOCIATIVITY)),
      timestamps(NUM_SUBCACHES, 0) {}

uint16_t SampledCache::hash_block(uint64_t block_address) {
    return (block_address >> 4) & 0x3FF; // bits [13:4]
}

uint16_t SampledCache::hash_set_index(uint64_t block_address) {
    return ((block_address >> 4) & 0x1F) * SETS_PER_SUBCACHE + (block_address & 0xF); // 5 bits + 4 bits
}

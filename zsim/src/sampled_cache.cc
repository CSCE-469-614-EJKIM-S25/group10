#include "sampled_cache.hh"
#include <algorithm>

SampledCache::SampledCache()
    : cache(TOTAL_SETS, std::vector<SampledCacheEntry>(ASSOCIATIVITY)),
      timestamps(NUM_SUBCACHES, 0) {}

uint16_t SampledCache::hash_block(uint64_t block_address) {
    return (block_address >> 4) & 0x3FF; // bits [13:4]
}

uint16_t SampledCache::hash_set_index(uint64_t block_address) {
    return ((block_address >> 4) & 0x1F) * SETS_PER_SUBCACHE + (block_address & 0xF); // 5 + 4 bits
}

void SampledCache::access(uint64_t block_address, uint16_t pc_sig, bool& hit, uint8_t& reuse_distance) {
    uint16_t index = hash_set_index(block_address);
    uint16_t block_tag = hash_block(block_address);
    uint8_t& ts = timestamps[index / SETS_PER_SUBCACHE]; // get timestamp for this sub-cache

    hit = false;

    for (auto& entry : cache[index]) {
        if (entry.valid && entry.block_tag == block_tag) {
            // Found the block — reuse distance = time difference
            reuse_distance = (ts >= entry.timestamp) ? (ts - entry.timestamp) : (ts + 256 - entry.timestamp);
            hit = true;
            return;
        }
    }
}

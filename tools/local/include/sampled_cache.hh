#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>

struct SampledCacheEntry {
    bool valid = false;
    uint16_t block_tag;     // 10-bit block address hash
    uint16_t pc_signature;  // 11-bit PC signature
    uint8_t timestamp;      // 8-bit last access timestamp
};

class SampledCache {
public:
    SampledCache();
    void access(uint64_t block_address, uint16_t pc_signature, bool& hit, uint8_t& reuse_distance);
    void update(uint64_t block_address, uint16_t pc_signature);

private:
    static constexpr int NUM_SUBCACHES = 32;
    static constexpr int SETS_PER_SUBCACHE = 16;
    static constexpr int ASSOCIATIVITY = 5;
    static constexpr int TOTAL_SETS = NUM_SUBCACHES * SETS_PER_SUBCACHE;

    std::vector<std::vector<SampledCacheEntry>> cache;
    std::vector<uint8_t> timestamps;

    uint16_t hash_block(uint64_t block_address);
    uint16_t hash_set_index(uint64_t block_address);
};

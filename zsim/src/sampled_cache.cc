struct SampledCacheEntry {
    bool valid;
    uint64_t tag;
    uint64_t last_pc_signature;
    int timestamp;
};
static constexpr int NUM_SAMPLED_SETS = 512;
static constexpr int NUM_SAMPLED_WAYS = 5;
static constexpr int kTimestampBits = 8;
static constexpr int kMaxTimestamp = 1 << kTimestampBits;

SampledCacheLine sampledCacheEntries[NUM_SAMPLED_SETS][NUM_SAMPLED_WAYS];

int findInSampledCache(uint64_t tag, uint32_t setIndex) {
    for (int i = 0; i < NUM_SAMPLED_WAYS; ++i) {
        if (sampledCacheEntries[setIndex][i].valid && sampledCacheEntries[setIndex][i].tag == tag) {
            return i;
        }
    }
    return -1; // not found
}
constexpr int kTimestampBits = 8;
constexpr int kMaxTimestamp = 1 << kTimestampBits;

// Update timestamp with wrap-around behavior
inline int updateTimestamp(int current) {
    return (current + 1) % kMaxTimestamp;
}

// Compute reuse interval considering wrap-around
inline int computeElapsedTime(int recent, int previous) {
    if (recent >= previous) {
        return recent - previous;
    } else {
        return (recent + kMaxTimestamp) - previous;
    }
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


struct SampledCacheEntry {
    bool valid;
    uint64_t tag;
    uint64_t last_pc_signature;
    int timestamp;
};
unordered_map<uint32_t, SampledCacheEntry* > sampled_cache;

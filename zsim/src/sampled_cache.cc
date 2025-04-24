struct SampledCacheLine {
    bool valid;
    uint64_t tag;
    uint64_t signature;
    int timestamp;
};
unordered_map<uint32_t, SampledCacheLine* > sampled_cache;



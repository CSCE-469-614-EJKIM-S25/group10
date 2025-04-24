#include "sampled_cache.hh"
#include <iostream>

int main() {
    SampledCache cache;

    uint64_t addr = 0x123456; // block address
    uint16_t pc = 0x42;       // PC signature
    bool hit;
    uint8_t rd;

    // First access → should be a miss
    cache.access(addr, pc, hit, rd);
    std::cout << "First access: Hit = " << hit << "\n";

    // Insert the block
    cache.update(addr, pc);

    // Access again → should be a hit and some reuse distance
    cache.access(addr, pc, hit, rd);
    std::cout << "Second access: Hit = " << hit << ", Reuse Distance = " << (int)rd << "\n";

    return 0;
}

#include "RD53Event.h"

#include <iostream>
#include <bitset>

int main() {
    Rd53StreamConfig config;
    config.chip_id = true;
    config.drop_tot = false;
    config.compressed_hitmap = true;
    config.eos_marker = true;
    config.bcid = true;
    config.l1id = true;

    std::vector<std::tuple<int, int, int>> hits = {{0, 0, 1}, {15, 20, 2}, {40, 100, 12}};
    Event event(config, hits, 127, 3, 10, 35);
    event.get_quarter_cores();
    
    auto serialized_data = event.serialize_event();
    for (const auto& data : serialized_data) {
        std::bitset<64> b(data);
        std::cout << b << std::endl;
    }

    return 0;
}
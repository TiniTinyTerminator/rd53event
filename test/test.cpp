#include "RD53Event.h"

#include <iostream>
#include <bitset>
#include <ctime>
#include <cassert>
#include <map>

using namespace RD53;

int main()
{
    StreamConfig config;

    config.chip_id = true;
    config.drop_tot = false;
    config.compressed_hitmap = true;
    config.eos_marker = false;
    config.bcid = true;
    config.l1id = true;
    config.size_qcore_horizontal = 8;
    config.size_qcore_vertical = 2;

    std::map<std::pair<uint16_t, uint16_t>, uint8_t> hits_mapped;

    std::srand(std::time(nullptr));

    for (int x = 0; x < (RD53::N_QCORES_HORIZONTAL) * config.size_qcore_horizontal; x++)
    {
        for (int y = 0; y < (RD53::N_QCORES_VERTICAL) * config.size_qcore_vertical; y++)
        {
            // hits.push_back(HitCoord(x, y, std::rand() % 16));
            hits_mapped[{x, y}] = std::rand() % 16;
        }
    }


    std::vector<HitCoord> hits;

    for (auto &[k, v] : hits_mapped)
    {
        hits.push_back(HitCoord(k.first, k.second, v));
    }
    
    StreamHeader header = {13, 1, 3, 200, 500};

    Event event(config, header, hits);

    auto serialized_data = event.serialize_event();

    // for (auto qc : event.get_qcores())
    // { 
    //     std::cout << qc.as_str();
    // }

    std::cout << std::endl;

    Decoder decoder(config, serialized_data);

    decoder.process_stream();

    auto events = decoder.get_events();

    auto received_qc = decoder.get_events()[0].get_qcores();

    assert(received_qc == event.get_qcores());
    
    std::cout << "Qcores match!" << std::endl;

    auto received_hits = decoder.get_events()[0].get_hits();

    std::sort(received_hits.begin(), received_hits.end(),
              [](const auto &a, const auto &b)
              {
                  return std::get<0>(a) < std::get<0>(b) ||
                         (std::get<0>(a) == std::get<0>(b) && std::get<1>(a) < std::get<1>(b));
              });
    std::sort(hits.begin(), hits.end(),
              [](const auto &a, const auto &b)
              {
                  return std::get<0>(a) < std::get<0>(b) ||
                         (std::get<0>(a) == std::get<0>(b) && std::get<1>(a) < std::get<1>(b));
              });

    for (size_t i = 0; i < received_hits.size(); ++i)
    {
        if (received_hits[i] != hits[i])
        {
            std::cerr << "Received and expected hits differ at index " << i << std::endl;
            // std::cerr << "Received: " << received_hits[i] << std::endl;
            // std::cerr << "Expected: " << hits[i] << std::endl;
            assert(false);
        }
    }

    return 0;
}

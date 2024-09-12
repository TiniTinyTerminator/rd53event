#include "RD53Event.h"

#include <iostream>
#include <bitset>
#include <ctime>
#include <cassert>

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

    std::vector<HitCoord> hits;

    std::srand(std::time(nullptr));

    for (int i = 0; i < RD53::N_QCORES_HORIZONTAL * config.size_qcore_horizontal; i++)
    {
        for (int j = 0; j < RD53::N_QCORES_VERTICAL * config.size_qcore_vertical; j++)
        {
            hits.push_back(HitCoord(i, j, std::rand() % 16));
        }
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

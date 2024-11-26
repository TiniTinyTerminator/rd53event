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
    config.size_qcore_horizontal = 4;
    config.size_qcore_vertical = 4;

    std::map<std::pair<uint16_t, uint16_t>, uint8_t> hits_mapped;

    std::srand(std::time(nullptr));

    for (int x = 0; x < (RD53::N_QCORES_HORIZONTAL)*config.size_qcore_horizontal; x++)
    {
        for (int y = 0; y < (RD53::N_QCORES_VERTICAL)*config.size_qcore_vertical; y++)
        {
            // hits.push_back(HitCoord(x, y, std::rand() % 16));
            hits_mapped[{x, y}] = std::rand() % 16;
        }
    }

    std::cout << (RD53::N_QCORES_HORIZONTAL)*config.size_qcore_horizontal << std::endl;
    std::cout << (RD53::N_QCORES_VERTICAL)*config.size_qcore_vertical << std::endl;
    std::vector<HitCoord> hits;

    // for (uint32_t i = 0; i < 10000; i++)
    // {
    //     hits_mapped[{std::rand() % (RD53::N_QCORES_HORIZONTAL * config.size_qcore_horizontal), std::rand() % (RD53::N_QCORES_VERTICAL * config.size_qcore_vertical)}] = std::rand() % 16;
    // }

    for (auto &[k, v] : hits_mapped)
    {
        hits.push_back(HitCoord(k.first, k.second, v));
    }

    StreamHeader header = {13, 1, 3, 200, 500};

    TEPXEvent event(config, header, hits);

    auto serialized_data = event.serialize_event();

    // for (auto qc : event.get_qcores())
    // {
    //     std::cout << qc.as_str();
    // }

    std::cout << std::endl;

    for (auto serialized : serialized_data)
    {

        Decoder decoder(config, serialized);

        decoder.process_stream();

        auto events = decoder.get_event();

        auto received_hits = decoder.get_event().get_hits()[0];

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
            auto [x1, y1, tot1] = received_hits[i];
            auto [x2, y2, tot2] = hits[i];

            if (x1 != x2 || y1 != y2)
            {
                std::cerr << "Received and expected hits differ at index " << i << std::endl;

                std::cerr << "Received: " << (uint64_t)x1 << ", " << (uint64_t)y1 << ", " << (uint64_t)tot1 << std::endl;
                std::cerr << "Expected: " << (uint64_t)x2 << ", " << (uint64_t)y2 << ", " << (uint64_t)tot2 << std::endl;
                assert(false);
            }
        }
    }
    return 0;
}

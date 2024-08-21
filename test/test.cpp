#include "RD53Event.h"

#include <iostream>
#include <bitset>
#include <ctime>
#include <cassert>

int main()
{
    Rd53StreamConfig config;

    config.chip_id = true;
    config.drop_tot = false;
    config.compressed_hitmap = true;
    config.eos_marker = false;
    config.bcid = true;
    config.l1id = true;

    std::vector<HitCoord> hits;

    std::srand(std::time(nullptr));

    // Generate random points
    for (int i = 0; i < 500; i++)
    {
        int x = std::rand() % (N_QCORES_HORIZONTAL * config.size_qcore_horizontal);
        int y = std::rand() % (N_QCORES_VERTICAL * config.size_qcore_vertical);
        int tot = (std::rand() + 1) % 16;
        auto it = std::find_if(hits.begin(), hits.end(), [x, y](const auto& hit) {
            return hit.x == x && hit.y == y;
        });
        
        if (it == hits.end())
        {
            hits.push_back(HitCoord(x, y, tot));
        }
    }

    RD53Header header = {13, 1, 3, 200, 500};

    RD53Event event(config, header, hits);

    auto serialized_data = event.serialize_event();

    // for (auto qc : event.get_qcores())
    // {
    //     std::cout << qc.as_str();
    // }

    std::cout << std::endl;

    RD53Decoder decoder(config, serialized_data);

    decoder.process_stream();

    auto events = decoder.get_events();

    auto received_qc = decoder.get_events()[0].get_qcores();

    assert(received_qc == event.get_qcores());

    auto received_hits = decoder.get_events()[0].get_hits();

    std::sort(received_hits.begin(), received_hits.end(),
             [](const auto& a, const auto& b) {
                 return std::tie(a.x, a.y) <
                        std::tie(b.x, b.y);
             });
    std::sort(hits.begin(), hits.end(),
             [](const auto& a, const auto& b) {
                 return std::tie(a.x, a.y) <
                        std::tie(b.x, b.y);
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

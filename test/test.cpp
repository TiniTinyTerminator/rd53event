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

    std::vector<std::tuple<int, int, int>> hits;

    std::srand(std::time(nullptr));

    // Generate random points
    for (int i = 0; i < 100000; i++)
    {
        int x = std::rand() % (N_QCORES_HORIZONTAL * config.size_qcore_horizontal);
        int y = std::rand() % (N_QCORES_VERTICAL * config.size_qcore_vertical);
        int tot = (std::rand() + 1) % 16;
        hits.push_back(std::make_tuple(x, y, tot));
    }

    StreamHeader header = {13, 1, 3, 200, 500};

    RD53Event event(config, header, hits);

    auto serialized_data = event.serialize_event();

    for (auto qc : event.get_qcores())
    {
        std::cout << qc.as_str();
    }

    std::cout << std::endl;

    // for (const auto &data : serialized_data)
    // {
    //     std::bitset<64> b(data);
    //     std::cout << b << std::endl;
    // }
    std::cout << std::endl;

    RD53Decoder decoder(config, serialized_data);

    decoder.process_stream();

    auto events = decoder.get_events();

    std::vector<std::pair<QuarterCore, QuarterCore>> pairs;

    auto qc_sent = event.get_qcores();
    auto qc_received = events.at(0).get_qcores();
    for (size_t i = 0; i < qc_sent.size(); i++)
    {
        std::cout << "Testing qcore at index " << i << std::endl;

        assert(static_cast<int>(qc_sent[i].get_col()) == static_cast<int>(qc_received[i].get_col()));
        assert(static_cast<int>(qc_sent[i].get_row()) == static_cast<int>(qc_received[i].get_row()));

        std::pair<uint16_t, uint64_t> sent_hits = qc_sent[i].get_hit_raw();
        std::pair<uint16_t, uint64_t> received_hits = qc_received[i].get_hit_raw();

        std::bitset<16> sent_hits_bits(sent_hits.first);
        std::bitset<64> sent_tots_bits(sent_hits.second);

        std::bitset<16> received_hits_bits(received_hits.first);
        std::bitset<64> received_tots_bits(received_hits.second);

        std::cout << "Sent hits: " << sent_hits_bits << std::endl;
        std::cout << "Sent tots: " << sent_tots_bits << std::endl;

        std::cout << "Received hits: " << received_hits_bits << std::endl;
        std::cout << "Received tots: " << received_tots_bits << std::endl;

        if (sent_hits != received_hits)
        {
            std::cout << qc_sent[i].as_str();
            assert(false);

        }
        

        assert(static_cast<int>(qc_sent[i].get_is_last()) == static_cast<int>(qc_received[i].get_is_last()));
        assert(static_cast<int>(qc_sent[i].get_is_neighbour()) == static_cast<int>(qc_received[i].get_is_neighbour()));

        std::cout << "Success at index " << i << std::endl;
    }
    //    std::cout << event.as_str() << std::endl;
    //    std::cout << events.at(0).as_str() << std::endl;

    return 0;
}

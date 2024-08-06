#include "RD53Decoder.h"
#include "RD53Event.h"



class RD53Decoder {
    std::vector<uint64_t> stream;
    const Rd53StreamConfig *config;

    std::vector<QuarterCore> qcores;
    std::vector<Event> events;

    void _get_col();
    void _get_row();
    void _is_last_in_col();
    void _is_neighbor();
    void _get_hits();
    void _get_tots();

public:
    RD53Decoder(const Rd53StreamConfig &config, const std::vector<uint64_t> &stream) : config(&config), stream(stream) {}

    void decoder();

};
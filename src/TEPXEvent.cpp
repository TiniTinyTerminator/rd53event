#include "RD53Event.h"

using namespace RD53;

TEPXEvent::TEPXEvent(const StreamConfig &config_, const StreamHeader &header_, const std::vector<HitCoord> &hits_)
    : TEPXEvent(config_, header_, std::vector<std::vector<HitCoord>>{hits_})
{
}


TEPXEvent::TEPXEvent(const StreamConfig &config_, const StreamHeader &header_, const std::vector<std::vector<HitCoord>> &frames_)
    : config(config_), header(header_), frames(frames_)
{
    const uint64_t chip_height = config.size_qcore_vertical * N_QCORES_VERTICAL;
    const uint64_t chip_width = config.size_qcore_horizontal * N_QCORES_HORIZONTAL;

    std::array<std::vector<std::vector<HitCoord>>, 4> subframes;

    auto shift_hit = [chip_height, chip_width](HitCoord hit)
    {
        auto [x, y, tot] = hit;
        return HitCoord(x % chip_height, y % chip_width, tot);
    };

    for (uint8_t i = 0; i < 4; i++)
    {

        auto is_in_quadrant = [i, chip_height, chip_width](HitCoord hit)
        {
            auto [x, y, tot] = hit;

            switch (i)
            {
            case 0:
                return x < chip_height && y < chip_width;
            case 1:
                return x < chip_height && y > chip_width;
            case 2:
                return x > chip_height && y < chip_width;
            case 3:
                return x > chip_height && y > chip_width;
            }

            return false;
        };

        for (auto frame : frames)
        {
            subframes[i].push_back(std::vector<HitCoord>());

            auto subframe = subframes[i].end() - 1;

            std::copy_if(frame.begin(), frame.end(), std::back_inserter(*subframe), is_in_quadrant);
            std::transform(subframe->begin(), subframe->end(), std::back_inserter(*subframe), shift_hit);
        }
    }

    for (uint8_t i = 0; i < 4; i++)
    {
        chips[i] = Event(config, header, subframes[i]);
    }



}

std::array<std::vector<word_t>, 4> TEPXEvent::serialize_event()
{
    std::array<std::vector<word_t>, 4> result;

    for (uint8_t i = 0; i < 4; i++)
    {
        result[i] = chips[i].serialize_event();
    }

    return result;
}

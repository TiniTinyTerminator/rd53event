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
        return HitCoord(x % chip_width, y % chip_height, tot);
    };

    for (uint8_t i = 0; i < 4; i++)
    {

        auto is_in_quadrant = [i, chip_height, chip_width](HitCoord hit)
        {
            auto [x, y, tot] = hit;

            switch (i)
            {
            case 0:
                return x < chip_width && y < chip_height;
            case 1:
                return x < chip_width && y >= chip_height;
            case 2:
                return x >= chip_width && y < chip_height;
            case 3:
                return x >= chip_width && y >= chip_height;
            }

            return false;
        };

        for (auto frame : frames)
        {
            subframes[i].push_back(std::vector<HitCoord>());

            auto & subframe = subframes[i][subframes[i].size() - 1];


            std::copy_if(frame.begin(), frame.end(), std::back_inserter(subframe), is_in_quadrant);

            std::vector<HitCoord> tmp;

            std::transform(subframe.begin(), subframe.end(), std::back_inserter(tmp), shift_hit);

            subframe = tmp;
        }
    
        StreamHeader h = header;

        h.chip_id = i;

        chips[i] = Event(config, h, subframes[i]);
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

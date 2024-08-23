#include "RD53BEvent.h"

#include <bitset>

using namespace RD53B;

Event::Event(const StreamConfig &config, const StreamHeader &header, const std::vector<HitCoord> &hits)
    : config(config), header(header), hits(hits)
{
}

Event::Event(const StreamConfig &config, const StreamHeader &header, std::vector<QuarterCore> &qcores)
    : config(config), header(header), qcores(qcores)
{
    // update address of config in qcores
    for (auto &qcore : qcores)
    {
        qcore.set_config(config);
    }
}

Event::Event(const Event &other) : config(other.config), header(other.header), hits(other.hits), qcores(other.qcores)
{
    for (auto &qcore : qcores)
    {
        qcore.set_config(config);
    }
}

Event & Event::operator=(const Event &other)
{
    if (&other != this) *this = other;

    for (auto &qcore : qcores)
    {
        qcore.set_config(config);
    }

    return *this;
}


void Event::_get_qcores_from_pixelframe()
{
    if (!qcores.empty())
        throw std::runtime_error("QuarterCores already set in event");
    if (hits.empty())
        throw std::runtime_error("No hits in event");

    std::map<std::pair<int, int>, QuarterCore> qcore_dict;

    for (const auto &[x, y, tot] : hits)
    {
        int col_in_qcore = x % config.size_qcore_horizontal;
        int qcol = (x - col_in_qcore) / config.size_qcore_horizontal;
        int row_in_qcore = y % config.size_qcore_vertical;
        int qrow = (y - row_in_qcore) / config.size_qcore_vertical;

        if (qcore_dict.find({qcol, qrow}) == qcore_dict.end())
        {
            qcore_dict[{qcol, qrow}] = QuarterCore(config, qcol, qrow);
        }

        qcore_dict[{qcol, qrow}].set_hit(col_in_qcore, row_in_qcore, tot);
    }

    for (auto it = qcore_dict.begin(); it != qcore_dict.end(); ++it)
    {
        size_t key_id = std::distance(qcore_dict.begin(), it);
        auto &[key, qcore] = *it;

        if (it == std::prev(qcore_dict.end()))
        {
            qcore.set_is_last(true);
            qcore.set_is_last_in_event(true);
        }
        else
        {
            qcore.set_is_last_in_event(false);

            auto &next_key = std::next(it)->first;
            qcore.set_is_last(key.first != next_key.first);
        }

        if (it == qcore_dict.begin())
        {
            qcore.set_is_neighbour(false);
        }
        else
        {
            auto &prev_key = std::prev(it)->first;
            qcore.set_is_neighbour(key.first == prev_key.first && key.second == prev_key.second + 1);
        }

        qcores.push_back(qcore);
    }
}

void Event::_get_pixelframe_from_qcores()
{
    if (!hits.empty())
        throw std::runtime_error("Hits already set in event");
    if (qcores.empty())
        throw std::runtime_error("No qcores in event");

    hits = {};

    for (auto &qcore : qcores)
    {
        auto qcore_coords = qcore.get_hit_vectors();
        auto col = qcore.get_col(), row = qcore.get_row();

        for (auto &&[x, y, tot] : qcore_coords)
        {
            hits.push_back(HitCoord(x + col * config.size_qcore_horizontal, y + row * config.size_qcore_vertical, tot));
        }
    }
}

std::vector<word_t> Event::serialize_event()
{
    if (qcores.empty())
        _get_qcores_from_pixelframe();

    auto packets = _retrieve_qcore_data();

    const uint8_t WORD_SIZE = config.chip_id ? 61 : 63;
    std::vector<word_t> result;
    word_t current_word = 0;
    uint8_t current_size = 0;

    packets.insert(packets.begin(), std::make_tuple(8, (header.trigger_tag << 2) | (header.trigger_pos & 0b11), DataTags::TRIGGER_TAG));

    switch (config.l1id << 1 | config.bcid)
    {
    case 0b01:
        packets.insert(packets.begin() + 1, std::make_tuple(16, header.bcid, DataTags::EXTRA_IDS));
        break;
    case 0b10:
        packets.insert(packets.begin() + 1, std::make_tuple(16, header.l1id, DataTags::EXTRA_IDS));
        break;
    case 0b11:
        packets.insert(packets.begin() + 1, std::make_tuple(16, ((header.bcid & 0xFF) << 8) | (header.l1id & 0xFF), DataTags::EXTRA_IDS));
        break;
    default:
        break;
    }

    if (DEBUG)
    {
        size_t s = 0;
        for (const auto &[width, word, name] : packets)
        {
            std::string bits = get_lsb_binary(word, width);

            s += width;

            if (s >= WORD_SIZE)
            {
                std::cout << set_color(data_tag_colors[name]) << bits.substr(0, width - (s - WORD_SIZE)) << fgc[Color::RESET] << bgc[Color::RESET] << std::endl
                          << set_color(data_tag_colors[name]) << bits.substr(width - (s - WORD_SIZE));

                s -= WORD_SIZE;
            }
            else
            {
                std::cout << set_color(data_tag_colors[name]) << bits;
            }
        }
        std::cout << fgc[Color::RESET] << bgc[Color::RESET] << std::endl;
    }

    for (const auto &[width, word, name] : packets)
    {

        uint8_t space_left = WORD_SIZE - current_size;

        if (width <= space_left)
        {
            current_word <<= width;
            current_word |= word & ((1 << width) - 1);
            current_size += width;

            if (current_size == WORD_SIZE)
            {
                result.push_back(current_word);
                current_word = 0;
                current_size = 0;
            }
        }
        else
        {
            int bits_for_current_word = word >> (width - space_left);
            current_word <<= space_left;
            current_word |= bits_for_current_word & ((1 << space_left) - 1);

            result.push_back(current_word);

            current_word = word & ((1 << (width - space_left)) - 1);
            current_size = (width - space_left);
        }
    }

    if (current_size > 0)
    {
        int shift_amount = WORD_SIZE - current_size;
        current_word <<= shift_amount;
        result.push_back(current_word);
    }

    if (config.chip_id)
    {
        for (auto &word : result)
        {
            word |= ((uint64_t)header.chip_id & 0b11) << 61;
        }
    }

    result[result.size() - 1] = result.back() | (1ull << 63);
    return result;
}

std::vector<std::tuple<uint8_t, unsigned long long, DataTags>> Event::_retrieve_qcore_data()
{
    bool prev_last_in_col = true;
    std::vector<std::tuple<uint8_t, unsigned long long, DataTags>> qcore_packages;

    for (const auto &qcore : qcores)
    {
        auto qcore_data = qcore.serialize_qcore(prev_last_in_col);
        qcore_packages.insert(qcore_packages.end(), qcore_data.begin(), qcore_data.end());
        prev_last_in_col = qcore.get_is_last();
    }

    return qcore_packages;
}

std::string Event::as_str() const
{
    std::stringstream ss;

    ss << "config: " << config.as_str() << std::endl;

    ss << "header: " << header.as_str() << std::endl;

    // auto qcores_data = get_qcores();
    for (const auto &qcore : qcores)
    {
        ss << qcore.as_str() << std::endl;
    }

    // auto hits_data = get_hits();
    for (const auto &hit : hits)
    {
        ss << "(" << hit.x << ", " << hit.y << ", " << hit.val << ")";
    }

    return ss.str();
}

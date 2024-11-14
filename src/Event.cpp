#include "RD53Event.h"

#include <bitset>
#include <fstream>

using namespace RD53;

Event::Event(const StreamConfig &config_, const StreamHeader &header_, const std::vector<HitCoord> &hits_)
    : config(config_), header(header_), hits(hits_)
{
}

Event::Event(const StreamConfig &config_, const StreamHeader &header_, std::vector<QuarterCore> &qcores_)
    : config(config_), header(header_), qcores(qcores_)
{
    // update address of config in qcores
    for (auto &qcore : qcores_)
    {
        qcore.set_config(&config);
    }
}

Event::Event(const StreamConfig &config_, const StreamHeader &header_, const std::vector<std::vector<HitCoord>> &frames_) : config(config_), header(header_), hits(frames_[0])
{

    bool first = true;

    for (uint64_t i = 1; i < frames_.size(); i++)
    {
        StreamHeader sub_header = header;

        sub_header.l1id++;
        sub_header.bcid++;
        sub_header.trigger_pos = sub_header.trigger_pos >= 3 ? 0 : sub_header.trigger_pos + 1;

        if (sub_header.trigger_pos == 3)
            first = false;

        if (sub_header.trigger_pos == 0 && !first)
            sub_header.trigger_tag = sub_header.trigger_tag >= 31 ? 0 : sub_header.trigger_tag + 1;

        events.push_back(Event(config_, sub_header, frames_[i]));
    }
}

Event::Event(const StreamConfig &config_, const StreamHeader &header_, std::vector<std::vector<QuarterCore>> &frames_) : config(config_), header(header_)
{
    bool first = true;

    for (uint64_t i = 1; i < frames_.size(); i++)
    {
        StreamHeader sub_header = header;

        sub_header.l1id++;
        sub_header.bcid++;
        sub_header.trigger_pos = sub_header.trigger_pos >= 3 ? 0 : sub_header.trigger_pos + 1;

        if (sub_header.trigger_pos == 3)
            first = false;

        if (sub_header.trigger_pos == 0 && !first)
            sub_header.trigger_tag = sub_header.trigger_tag >= 31 ? 0 : sub_header.trigger_tag + 1;

        events.push_back(Event(config_, sub_header, frames_[i]));
    }
}

Event::Event(const Event &other) : config(other.config), header(other.header), hits(other.hits), qcores(other.qcores), events(other.events)
{
    for (auto &qcore : qcores)
    {
        qcore.set_config(&config);
    }
}

Event Event::operator=(const Event &other)
{
    config = other.config;
    header = other.header;
    hits = other.hits;
    qcores = other.qcores;
    events = other.events;

    return *this;
}

void Event::_get_qcores_from_pixelframe()
{
    if (!qcores.empty())
        throw std::runtime_error("QuarterCores already set in event");
    if (hits.empty())
        throw std::runtime_error("No hits in event");

    std::map<std::pair<int, int>, QuarterCore> qcore_dict;

    for (const auto [x, y, tot] : hits)
    {

        uint16_t col_in_qcore = x % config.size_qcore_horizontal;
        uint16_t qcol = (x - col_in_qcore) / config.size_qcore_horizontal;
        uint16_t row_in_qcore = y % config.size_qcore_vertical;
        uint16_t qrow = (y - row_in_qcore) / config.size_qcore_vertical;

        if (qcore_dict.find({qcol, qrow}) == qcore_dict.end())
        {
            qcore_dict[{qcol, qrow}] = QuarterCore(config, qcol, qrow);
        }

        qcore_dict[{qcol, qrow}].set_hit(col_in_qcore, row_in_qcore, tot);
    }

    // TODO reverse column order (columns decrements from 55 to 1)

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
    if (qcores.empty() && !hits.empty())
        _get_qcores_from_pixelframe();

    const uint8_t WORD_SIZE = config.chip_id ? 61 : 63;
    std::vector<word_t> result;
    word_t current_word = 0;
    uint8_t current_size = 0;

    std::vector<std::tuple<uint8_t, RD53::word_t, DataTags>> packets = _retrieve_qcore_data();

    packets.insert(packets.begin(), std::make_tuple(8, (header.trigger_tag << 2) | (header.trigger_pos & 0b11), DataTags::TRIGGER_TAG));

    for (auto event : events)
    {
        auto subeventpackets = event._retrieve_qcore_data();
        subeventpackets.insert(subeventpackets.begin(), std::make_tuple(11, 0b111 << 8 | ((event.header.trigger_tag & 0x3F) << 2) | (event.header.trigger_pos & 0b11), DataTags::TRIGGER_TAG));

        packets.insert(packets.end(), subeventpackets.begin(), subeventpackets.end());
    }

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

    if constexpr (DEBUG)
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
    for (const auto [x, y, val] : hits)
    {
        ss << "(" << x << ", " << y << ", " << val << ")";
    }

    return ss.str();
}

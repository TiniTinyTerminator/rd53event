#include "RD53Event.h"

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <limits>
#include <bitset>
#include <string>

constexpr auto BITS_PER_WORD = std::numeric_limits<word_t>::digits;

/**
 * @brief Decodes a compressed bit pair from the input bits.
 *
 * @param bits The input bits.
 * @return A pair containing the decoded value and the number of bits read.
 */
inline std::pair<uint8_t, uint8_t> decode_bitpair(uint8_t bits)
{
    if ((bits & 1) == 0)
        return {1, 1};

    return {bits, 2};
}

/**
 * @brief Counts the number of set bits (1s) in an integer.
 *
 * @param n The input integer.
 * @return The number of set bits.
 */
unsigned count_set_bits(int n)
{
    unsigned count = 0;
    while (n)
    {
        count += n & 1; // Check the least significant bit
        n >>= 1;        // Right shift the number
    }
    return count;
}

DataTags state;

RD53Decoder::RD53Decoder(const Rd53StreamConfig &config, std::vector<word_t> &words) : stream(words), config(config), bit_index(0)
{
}

inline void RD53Decoder::_new_event()
{
    events.push_back({StreamHeader(), std::vector<QuarterCore>(0, QuarterCore(config))});

    current_event = events.begin() + events.size() - 1;

    current_header = &current_event->first;
    current_qcores = &current_event->second;

    qc = QuarterCore(config);
}

void RD53Decoder::process_stream()
{
    _new_event();

    bit_index = 0;

    word_size = config.chip_id ? 61 : 63;
    word_meta_size = config.chip_id ? 3 : 1;

    _validate_chip_id();
    _get_trigger_tag();
}

word_t RD53Decoder::_shift_stream(size_t bit_index)
{

    size_t word_index = bit_index / word_size;
    size_t bit_offset = bit_index % word_size;

    bool on_last_word = word_index == stream.size() - 1;

    word_t first_word = stream[word_index] << word_meta_size >> word_meta_size;
    word_t second_word = on_last_word ? 0 : stream[word_index + 1] << word_meta_size;

    first_word = bit_offset == 0 ? first_word : first_word << bit_offset;
    second_word = bit_offset == 0 ? 0 : second_word >> (BITS_PER_WORD - bit_offset);

    word_t full_word = first_word | second_word;

    if (DEBUG)
    {
        std::string word_str = std::bitset<64>(full_word).to_string().erase(0, word_meta_size);
        std::string first_word_str(word_str, 0, word_size - bit_offset);
        std::string second_word_str(word_str, word_size - bit_offset, word_size);

        std::stringstream ss;

        ss << fgc[Color::CYAN] << first_word_str << fgc[Color::GREEN] << second_word_str << fgc[Color::RESET] << "  " << jump_size << " " << bit_index << " " << state;

        auto str = ss.str();

        if (jump_size > 0)
        {
            const auto first_bracket = std::string(bgc[Color::BRIGHT_GREEN]);
            const auto second_bracket = std::string(bgc[Color::RESET]);

            str.insert(fgc[Color::CYAN].size(), first_bracket);
            str.insert(jump_size + fgc[Color::CYAN].size() + first_bracket.size(), second_bracket);
        }

        std::cout << str << std::endl;
    }

    return full_word;
}

word_t RD53Decoder::_get_nbits(uint8_t n_bits, bool increment)
{
    if (increment)
        jump_size = n_bits;
    else
        jump_size = 0;

    word_t a = _shift_stream(bit_index);

    if (increment)
        bit_index += n_bits;

    return (a >> (word_size - n_bits)) & ((1 << n_bits) - 1);
}

void RD53Decoder::_validate_chip_id()
{

    uint8_t chip_id = stream[0] >> 61 & 0b11;

    for (auto &word : stream)
    {
        if ((word >> 61 & 0b11) != chip_id)
            throw std::logic_error("Chip ID in stream has a mismatch");
    }

    current_header->chip_id = chip_id;
}

void RD53Decoder::_get_trigger_tag()
{
    state = DataTags::TRIGGER_TAG;

    uint8_t tag = _get_nbits(data_widths::TRIGGER_TAG_WIDTH);

    current_header->trigger_tag = tag >> 2;
    current_header->trigger_pos = tag & 0b11;

    if (DEBUG)
        std::cout << "Trigger tag: " << static_cast<int>(current_header->trigger_tag) << ", pos: " << static_cast<int>(current_header->trigger_pos) << std::endl;

    if ((config.l1id || config.bcid) && current_event == events.begin())
        _get_trigger_ids();

    _get_col();
}

void RD53Decoder::_get_trigger_ids()
{
    state = DataTags::EXTRA_IDS;

    uint16_t ids = _get_nbits(16);

    switch (config.l1id << 1 | config.bcid)
    {
    case 0b01:
        current_header->bcid = ids;
        break;
    case 0b10:
        current_header->l1id = ids;
        break;
    case 0b11:
        current_header->bcid = ids & 0xFF;
        current_header->l1id = (ids >> 8) & 0xFF;
        break;
    default:
        break;
    }

    if (DEBUG)
        std::cout << "ids: " << current_header->bcid << " " << current_header->l1id << std::endl;
}

void RD53Decoder::_get_col()
{
    state = DataTags::COLUMN;

    uint8_t col = _get_nbits(data_widths::COL_WIDTH);

    if (DEBUG)
        std::cout << "col: " << static_cast<int>(col) << std::endl;

    if (col == 0)
    {
        current_qcores->back().set_is_last(true);
        current_qcores->back().set_is_last_in_event(true);

        return;
    }
    else if (col >= 56)
    {
        current_qcores->back().set_is_last(true);
        current_qcores->back().set_is_last_in_event(true);

        _new_event();
        _get_nbits(3);

        _get_trigger_tag();
    }
    else
    {
        qc.set_col(col - 1);

        _get_neighbour_and_last();
    }
}

void RD53Decoder::_get_neighbour_and_last()
{
    state = DataTags::IS_LAST;

    qc.set_is_last(_get_nbits(1));

    state = DataTags::IS_NEIGHBOUR;

    qc.set_is_neighbour(_get_nbits(1));

    if (DEBUG)
        std::cout << "is_neighbour: " << static_cast<int>(qc.get_is_neighbour()) << " is_last: " << static_cast<int>(qc.get_is_last()) << std::endl;

    if (qc.get_is_neighbour())
    {
        qc.set_row(qc.get_row() + 1);

        if (DEBUG)
            std::cout << "row: " << qc.get_row() << std::endl;

        _get_hitmap();
    }
    else
        _get_row();
}

void RD53Decoder::_get_row()
{
    state = DataTags::ROW;

    uint8_t row = _get_nbits(data_widths::ROW_WIDTH);

    if (DEBUG)
        std::cout << "row: " << (uint32_t)row << std::endl;

    qc.set_row(row);

    _get_hitmap();
}

void RD53Decoder::_get_hitmap()
{
    state = DataTags::HITMAP;

    uint16_t hit_raw = 0;
    uint64_t tots_raw = 0;

    if (config.compressed_hitmap)
    {

        auto [row_mask, read_bits] = decode_bitpair(_get_nbits(2, false));

        bit_index += read_bits;
        jump_size = read_bits;

        for (size_t i = 0; i < 2; i++)
        {

            if (row_mask & (2 >> i))
            {
                auto [quad_mask, read_bits] = decode_bitpair(_get_nbits(2, false));
                bit_index += read_bits;
                jump_size = read_bits;

                std::vector<uint8_t> masks;

                for (size_t j = 0; j < count_set_bits(quad_mask); j++)
                {
                    auto [pair_mask, read_bits] = decode_bitpair(_get_nbits(2, false));
                    bit_index += read_bits;
                    jump_size = read_bits;

                    masks.push_back(pair_mask);
                }

                int current_quad = 0;
                for (int j = 0; j < 2; j++)
                {
                    if (quad_mask & (2 >> j))
                    {
                        for (int k = 0; k < 2; k++)
                        {
                            if (masks[current_quad] & (2 >> k))
                            {
                                auto [hitpair, read_bits] = decode_bitpair(_get_nbits(2, false));
                                bit_index += read_bits;
                                jump_size = read_bits;

                                hit_raw |= hitpair << (j * 4 + k * 2);
                            }
                        }

                        current_quad++;
                    }
                }
            }
        }
    }
    else
        hit_raw = _get_nbits(data_widths::HITMAP_WIDTH);

    if (!config.drop_tot)
        tots_raw = _get_tots(hit_raw);

    qc.set_hit_raw(hit_raw, tots_raw);

    if (DEBUG)
        std::cout << "HITS_RAW: " << std::bitset<16>(hit_raw) << " TOTS_RAW: " << std::bitset<64>(tots_raw) << std::endl;

    current_qcores->push_back(qc);

    // reset hits
    qc.set_hit_raw(0, 0);

    if (qc.get_is_last())
        _get_col();
    else
        _get_neighbour_and_last();
}

uint64_t RD53Decoder::_get_tots(uint16_t hit_raw)
{
    state = DataTags::TOT;

    uint8_t hit_index = 15;
    uint8_t hit_counter = 0;

    uint8_t n_hits = count_set_bits(hit_raw);

    uint64_t tots = 0;

    while (hit_counter < n_hits)
    {
        uint8_t hit = (hit_raw & (1 << hit_index)) >> hit_index;

        if (hit)
        {
            hit_counter++;

            uint64_t tot_value = _get_nbits(data_widths::TOT_WIDTH);

            tots |= tot_value << (hit_index * 4);
        }
        hit_index--;
    }

    return tots;
}

std::vector<RD53Event> RD53Decoder::get_events() const
{
    std::vector<RD53Event> processed_events;

    for (auto [header, qcores] : events)
    {
        processed_events.push_back(RD53Event(config, header, qcores));
    }

    return processed_events;
}
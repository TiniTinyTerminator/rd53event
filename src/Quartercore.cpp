#include "RD53Event.h"

#include <bitset>

QuarterCore::QuarterCore(const Rd53StreamConfig &config, uint8_t col, uint8_t row)
    : config(&config), col_(col), row_(row), is_last_(false), is_neighbour_(false), is_last_in_event_(false), hits_(0), tots_(0)
{
}

std::pair<bool, uint8_t> QuarterCore::get_hit(uint8_t index) const
{
    if (index >= 16)
        throw std::runtime_error("ERROR: index out of bounds");

    return {hits_ >> index & 0x1, tots_ >> (index * 4) & 0xF};
}

std::pair<bool, uint8_t> QuarterCore::get_hit(uint8_t x, uint8_t y) const
{
    int index = hit_index(x, y);

    return get_hit(index);
}

void QuarterCore::set_hit(uint8_t index, uint8_t tot)
{
    if (index >= 16)
        throw std::runtime_error("ERROR: col row out of range");

    hits_ = (hits_ & ~(1 << index)) | 1 << index;
    tots_ = (tots_ & ~((uint64_t)0xF << (index * 4))) | ((uint64_t)tot << (index * 4));
}

void QuarterCore::set_hit(uint8_t x, uint8_t y, uint8_t tot)
{
    int index = hit_index(x, y);

    set_hit(index, tot);
}

void QuarterCore::set_hit_raw(uint16_t hit_raw, uint64_t tots_raw)
{
    hits_ = hit_raw;
    tots_ = tots_raw;
}

std::pair<uint16_t, uint64_t> QuarterCore::get_hit_raw() const
{
    return {hits_, tots_};
}

std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> QuarterCore::get_hit_vectors() const
{
    std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> result;
    for (int x = 0; x < config->size_qcore_horizontal; ++x)
    {
        for (int y = 0; y < config->size_qcore_vertical; ++y)
        {
            uint8_t index = hit_index(y, x);

            if (hits_ >> index & 0x1)
            {
                result.push_back({x, y, (tots_ >> (index * 4)) & 0xF});
            }
        }
    }
    return result;
}

std::vector<std::vector<std::pair<bool, uint8_t>>> QuarterCore::get_hit_map() const
{
    std::vector<std::vector<std::pair<bool, uint8_t>>> hit_map(config->size_qcore_horizontal, std::vector<std::pair<bool, uint8_t>>(config->size_qcore_vertical));

    for (int x = 0; x < config->size_qcore_horizontal; ++x)
    {
        for (int y = 0; y < config->size_qcore_vertical; ++y)
        {
            uint8_t index = hit_index(y, x);
            hit_map[x][y] = {hits_ >> index & 0x1, tots_ >> (index * 4) & 0xF};
        }
    }

    return hit_map;
}

std::pair<int, int> QuarterCore::get_binary_tree() const
{
    int binary_tree = 0;
    int binary_tree_length = 0;
    std::array<int, 8> pair_hitor = {0};
    std::array<int, 4> quarter_hitor = {0};
    std::array<int, 2> half_hitor = {0};
    int pair_id = 0;

    while (pair_id < 8)
    {
        int quarter_id = (pair_id - (pair_id % 2)) / 2;
        int half_id = (quarter_id - (quarter_id % 2)) / 2;

        bool hit_bottom = hits_ >> ((8-pair_id)*2-1) & 0x1;
        bool hit_top = hits_ >> ((8-pair_id)*2-2) & 0x1;

        pair_hitor[pair_id] = hit_top || hit_bottom;

        if (!hit_top && hit_bottom)
        {
            binary_tree |= 0x0 << binary_tree_length;
            binary_tree_length += 1;
        }
        else if (hit_top && !hit_bottom)
        {
            binary_tree |= 0x2 << binary_tree_length;
            binary_tree_length += 2;
        }
        else if (hit_top && hit_bottom)
        {
            binary_tree |= 0x3 << binary_tree_length;
            binary_tree_length += 2;
        }

        if (pair_id % 2 == 1)
        {
            quarter_hitor[quarter_id] = pair_hitor[pair_id] || pair_hitor[pair_id - 1];

            if (quarter_id % 2 == 1)
            {
                half_hitor[half_id] = quarter_hitor[quarter_id] || quarter_hitor[quarter_id - 1];

                if (!pair_hitor[pair_id - 2] && pair_hitor[pair_id - 3])
                {
                    binary_tree |= 0x0 << binary_tree_length;
                    binary_tree_length += 1;
                }
                else if (pair_hitor[pair_id - 2] && !pair_hitor[pair_id - 3])
                {
                    binary_tree |= 0x2 << binary_tree_length;
                    binary_tree_length += 2;
                }
                else if (pair_hitor[pair_id - 2] && pair_hitor[pair_id - 3])
                {
                    binary_tree |= 0x3 << binary_tree_length;
                    binary_tree_length += 2;
                }

                if (!pair_hitor[pair_id] && pair_hitor[pair_id - 1])
                {
                    binary_tree |= 0x0 << binary_tree_length;
                    binary_tree_length += 1;
                }
                else if (pair_hitor[pair_id] && !pair_hitor[pair_id - 1])
                {
                    binary_tree |= 0x2 << binary_tree_length;
                    binary_tree_length += 2;
                }
                else if (pair_hitor[pair_id] && pair_hitor[pair_id - 1])
                {
                    binary_tree |= 0x3 << binary_tree_length;
                    binary_tree_length += 2;
                }

                if (!quarter_hitor[quarter_id] && quarter_hitor[quarter_id - 1])
                {
                    binary_tree |= 0x0 << binary_tree_length;
                    binary_tree_length += 1;
                }
                else if (quarter_hitor[quarter_id] && !quarter_hitor[quarter_id - 1])
                {
                    binary_tree |= 0x2 << binary_tree_length;
                    binary_tree_length += 2;
                }
                else if (quarter_hitor[quarter_id] && quarter_hitor[quarter_id - 1])
                {
                    binary_tree |= 0x3 << binary_tree_length;
                    binary_tree_length += 2;
                }
            }
        }
        pair_id += 1;
    }

    if (!half_hitor[1] && half_hitor[0])
    {
        binary_tree |= 0x0 << binary_tree_length;
        binary_tree_length += 1;
    }
    else if (half_hitor[1] && !half_hitor[0])
    {
        binary_tree |= 0x2 << binary_tree_length;
        binary_tree_length += 2;
    }
    else if (half_hitor[1] && half_hitor[0])
    {
        binary_tree |= 0x3 << binary_tree_length;
        binary_tree_length += 2;
    }

    return {binary_tree, binary_tree_length};
}

std::vector<std::tuple<uint8_t, unsigned long long, DataTags>> QuarterCore::serialize_qcore(bool prev_last_in_col) const
{
    std::vector<std::tuple<uint8_t, unsigned long long, DataTags>> qcore_data;

    if (prev_last_in_col)
    {
        qcore_data.push_back(std::make_tuple(6, col_ + 1, DataTags::COLUMN));
    }

    qcore_data.push_back(std::make_tuple(1, is_last_, DataTags::IS_LAST));
    qcore_data.push_back(std::make_tuple(1, is_neighbour_, DataTags::IS_NEIGHBOUR));

    if (!is_neighbour_)
    {
        qcore_data.push_back(std::make_tuple(8, row_, DataTags::ROW));
    }

    auto [bintree, bintree_length] = config->compressed_hitmap ? get_binary_tree() : std::make_pair<int, int>(std::get<0>(get_hit_raw()), 16);

    qcore_data.push_back(std::make_tuple(bintree_length, bintree, DataTags::HITMAP));

    if (!config->drop_tot)
    {

        for (int8_t i = 15; i >= 0; i--)
        {
            auto hit = get_hit(i);

            if (hit.first)
            {
                
                qcore_data.push_back(std::make_tuple(4, hit.second, DataTags::TOT));
            }
        }
    }

    return qcore_data;
}

uint8_t QuarterCore::hit_index(uint8_t row, uint8_t col) const
{
    if (col >= config->size_qcore_horizontal || row >= config->size_qcore_vertical)
    {
        throw std::runtime_error("ERROR: coordinates out of bounds");
    }

    if (config->size_qcore_vertical == 2 && config->size_qcore_horizontal == 8)
    {
        return 2 * col + row;
    }
    else if (config->size_qcore_vertical == 4 && config->size_qcore_horizontal == 4)
    {
        return row > 1 ? 8 + col * 2 + row - 2 : col * 2 + row;
    }
    else
    {
        throw std::runtime_error("ERROR: Wrong qcore size");
    }
}

// Implementation of the << operator
std::string QuarterCore::as_str() const
{
    std::stringstream str;

    str << "QuarterCore:\n";
    str << "  Column Index: " << static_cast<int>(col_) << "\n";
    str << "  Row Index: " << static_cast<int>(row_) << "\n";
    str << "  Is Last in RD53Event: " << std::boolalpha << is_last_in_event_ << "\n";
    str << "  Is Neighbour: " << std::boolalpha << is_neighbour_ << "\n";
    str << "  Is Last in Row: " << std::boolalpha << is_last_ << "\n";
    str << "  Hits (raw): " << std::bitset<16>(hits_) << "\n";
    str << "  Total Values (raw): " << std::bitset<64>(tots_) << "\n";

    auto hit_map = get_hit_map();
    str << "  Hit Map:\n";
    for (size_t x = 0; x < hit_map.size(); ++x)
    {
        for (size_t y = 0; y < hit_map[x].size(); ++y)
        {
            str << "    (" << std::boolalpha << hit_map[x][y].first << ", " << static_cast<int>(hit_map[x][y].second) << ") ";
        }
        str << "\n";
    }

    return str.str();
}
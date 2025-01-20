#include "RD53Event.h"

#include <bitset>
#include <iomanip>
#include <fstream>

using namespace RD53;

QuarterCore::QuarterCore(const StreamConfig &config, uint8_t col_, uint8_t row_)
    : config_(&config), col_(col_), row_(row_), is_last_(false), is_neighbour_(false), is_last_in_event_(false), hits_(0), tots_(0)
{
}

QuarterCore::QuarterCore(uint8_t col_, uint8_t row_)
    : config_(nullptr), col_(col_), row_(row_), is_last_(false), is_neighbour_(false), is_last_in_event_(false), hits_(0), tots_(0)
{
}

std::pair<bool, uint8_t> QuarterCore::get_hit(uint8_t index) const
{
    if (index >= 16)
        throw std::invalid_argument("index out of bounds");

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
        throw std::invalid_argument("index out of range");

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

std::vector<HitCoord> QuarterCore::get_hit_vectors() const
{
    if (config_ == nullptr)
        throw std::runtime_error("QuarterCore has no config");

    std::vector<HitCoord> result;
    for (uint8_t x = 0; x < config_->size_qcore_horizontal; x++)
    {
        for (uint8_t y = 0; y < config_->size_qcore_vertical; y++)
        {
            auto [h, tot] = get_hit(x, y);

            if (h)
            {
                result.push_back({x, y, tot});
            }
        }
    }
    return result;
}

std::vector<std::vector<std::pair<bool, uint8_t>>> QuarterCore::get_hit_map() const
{
    if (config_ == nullptr)
        throw std::runtime_error("QuarterCore has no config");

    std::vector<std::vector<std::pair<bool, uint8_t>>> hit_map(config_->size_qcore_horizontal, std::vector<std::pair<bool, uint8_t>>(config_->size_qcore_vertical));

    for (uint8_t x = 0; x < config_->size_qcore_horizontal; x++)
    {
        for (uint8_t y = 0; y < config_->size_qcore_vertical; y++)
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

        bool hit_bottom = hits_ >> ((8 - pair_id) * 2 - 1) & 0x1;
        bool hit_top = hits_ >> ((8 - pair_id) * 2 - 2) & 0x1;

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

    auto [bintree, bintree_length] = config_->compressed_hitmap ? get_binary_tree() : std::make_pair<int, int>(std::get<0>(get_hit_raw()), 16);

    qcore_data.push_back(std::make_tuple(bintree_length, bintree, DataTags::HITMAP));

    if (!config_->drop_tot)
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

uint8_t QuarterCore::hit_index(uint8_t col, uint8_t row) const
{
    if (config_ == nullptr)
        throw std::runtime_error("QuarterCore has no config");

    if (col >= config_->size_qcore_horizontal || row >= config_->size_qcore_vertical)
        throw std::invalid_argument("coordinates (" + std::to_string(col) + ", " + std::to_string(row) + ") out of bounds (" + std::to_string(config_->size_qcore_horizontal) + ", " + std::to_string(config_->size_qcore_vertical) + ")");

    if (config_->size_qcore_vertical == 2 && config_->size_qcore_horizontal == 8)
    {
        // the hits are mapped in qcore like:
        // 0  1   2  3   4  5   6  7
        // 8  9  10 11  12 13  14 15
        return col + 8 * row;
    }
    else if (config_->size_qcore_vertical == 4 && config_->size_qcore_horizontal == 4)
    {

        // the hits are mappend in qcore like:
        // 0  2  4  6
        // 1  3  5  7
        // 8 10 12 14
        // 9 11 13 15
        return row > 1 ? 8 + col * 2 + row - 2 : col * 2 + row;
    }
    else
    {
        throw std::runtime_error("ERROR: Wrong qcore size: " + std::to_string(config_->size_qcore_horizontal) + " x " + std::to_string(config_->size_qcore_vertical));
    }
}

// Implementation of the << operator
std::string QuarterCore::as_str() const
{
    std::stringstream str;

    str << std::left << "QuarterCore:\n";
    str << std::left << "  Column Index: " << std::right << std::setw(3) << static_cast<int>(col_) << "\n";
    str << std::left << "  Row Index: " << std::right << std::setw(6) << static_cast<int>(row_) << "\n";
    str << std::left << "  Last in event: " << std::right << std::setw(5) << (is_last_in_event_ ? "true " : "false") << "\n";
    str << std::left << "  Is Neighbour: " << std::right << std::setw(7) << (is_neighbour_ ? "true " : "false") << "\n";
    str << std::left << "  Is Last Row: " << std::right << std::setw(8) << (is_last_ ? "true " : "false") << "\n";
    str << std::left << "  Hits (raw): " << std::right << std::setw(10) << std::bitset<16>(hits_) << "\n";
    str << std::left << "  Tot Values: " << std::right << std::setw(10) << std::hex << std::setw(16) << std::setfill('0') << tots_ << "\n";

    if (config_ != nullptr)
    {
        auto hit_map = get_hit_map();
        str << "  Hit Map:\n";
        for (size_t x = 0; x < hit_map.size(); ++x)
        {
            for (size_t y = 0; y < hit_map[x].size(); ++y)
            {
                str << "\t(" << (hit_map[x][y].first ? "true " : "false") << ", " << std::setw(2) << static_cast<int>(hit_map[x][y].second) << ") ";
            }
            str << "\n";
        }
    }

    return str.str();
}

bool QuarterCore::operator==(const QuarterCore &other) const
{
    return col_ == other.col_ &&
           row_ == other.row_ &&
           hits_ == other.hits_ &&
           tots_ == other.tots_ &&
           is_last_ == other.is_last_ &&
           is_neighbour_ == other.is_neighbour_ &&
           is_last_in_event_ == other.is_last_in_event_;
}
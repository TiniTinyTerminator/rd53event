#include "RD53Event.h"

QuarterCore::QuarterCore(const Rd53StreamConfig &config, int col, int row)
    : config(&config), col(col), row(row), is_last(false), is_neighbour(false), is_last_in_event(false)
{
    hits.fill(false);
    tots.fill(0);
}

std::pair<bool, uint8_t> QuarterCore::get_hit(uint8_t index) const
{

    if (index < 0 || index >= 16)
        throw std::runtime_error("ERROR: index out of bounds");

    return {hits[index], tots[index]};
}

std::pair<bool, uint8_t> QuarterCore::get_hit(uint8_t x, uint8_t y) const
{
    int index = hit_index(x, y);

    if (index < 0 || index >= 16)
        throw std::runtime_error("ERROR: index out of bounds");

    return {hits[index], tots[index]};
}

void QuarterCore::set_hit(uint8_t index, uint8_t tot)
{
    if (index >= 16)
        throw std::runtime_error("ERROR: col row out of range");

    hits[index] = true;
    tots[index] = tot;
    
}

void QuarterCore::set_hit(uint8_t x, uint8_t y, uint8_t tot)
{
    int index = hit_index(x, y);

    if (index >= 16)
        throw std::runtime_error("ERROR: col row out of range");

    hits[index] = true;
    tots[index] = tot;
    
}


void QuarterCore::set_hit_raw(uint16_t hit_raw, uint64_t tots_raw)
{
    for (int i = 0; i < 16; ++i)
    {
        hits[i] = (hit_raw >> i) & 0x1;
        tots[i] = (tots_raw >> (i * 4)) & 0xF;
    }
}

std::pair<uint16_t, uint64_t> QuarterCore::get_hit_raw() const
{
    uint16_t raw_hits = 0;
    uint64_t raw_tots = 0;
    for (int i = 0; i < 16; ++i)
    {
        raw_hits |= (hits[i] & 0x1) << i;
        raw_tots |= (tots[i] & 0xF) << (i * 4);

    }
    return {raw_hits, raw_tots};
}

std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> QuarterCore::get_hit_vectors() const
{
    std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> result;
    for (int x = 0; x < config->size_qcore_horizontal; ++x)
    {
        for (int y = 0; y < config->size_qcore_vertical; ++y)
        {
            uint8_t index = hit_index(y, x);

            if (hits[index])
            {
                result.push_back({x, y, tots[index]});
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
            hit_map[x][y] = {hits[index], tots[index]};
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

        bool hit_bottom = hits[(8 - pair_id) * 2 - 1];
        bool hit_top = hits[(8 - pair_id) * 2 - 2];

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

std::vector<std::tuple<int, int, std::string>> QuarterCore::serialize_qcore(bool prev_last_in_col) const
{
    std::vector<std::tuple<int, int, std::string>> qcore_data;

    if (prev_last_in_col)
    {
        qcore_data.push_back(std::make_tuple(6, col, std::string("col")));
    }

    qcore_data.push_back(std::make_tuple(1, is_last, std::string("is last")));
    qcore_data.push_back(std::make_tuple(1, is_neighbour, std::string("is neighbour")));

    if (!is_neighbour)
    {
        qcore_data.push_back(std::make_tuple(8, row, std::string("row")));
    }

    auto [bintree, bintree_length] = config->compressed_hitmap ? get_binary_tree() : std::make_pair<int, int>(std::get<0>(get_hit_raw()), 16);

    qcore_data.push_back(std::make_tuple(bintree_length, bintree, std::string("binary tree")));

    if (!config->drop_tot)
    {

        for(uint8_t i = 0; i < 16; i++)
        {
            if(hits[16 - i])
            {
                qcore_data.push_back(std::make_tuple(4, tots[16 - i], std::string("tot from index " + std::to_string(i))));
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
        return col * 2 + row;
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

Event::Event(const Rd53StreamConfig &config, const std::vector<std::tuple<int, int, int>> &hits,
             int event_tag, int chip_id, int lcid, int bcid)
    : config(&config), event_tag(event_tag), chip_id(chip_id), lcid(lcid), bcid(bcid)
{
    for (const auto &hit : hits)
    {
        this->hits.push_back(hit);
    }
}

void Event::get_quarter_cores()
{
    std::map<std::pair<int, int>, QuarterCore> qcore_dict;

    for (const auto &[x, y, tot] : hits)
    {
        int col_in_qcore = x % config->size_qcore_horizontal;
        int qcol = (x - col_in_qcore) / config->size_qcore_horizontal;
        int row_in_qcore = y % config->size_qcore_vertical;
        int qrow = (y - row_in_qcore) / config->size_qcore_vertical;

        if (qcore_dict.find({qcol, qrow}) == qcore_dict.end())
        {
            qcore_dict[{qcol, qrow}] = QuarterCore(*config, qcol, qrow);
        }

        qcore_dict[{qcol, qrow}].set_hit(col_in_qcore, row_in_qcore, tot);
    }

    for (auto it = qcore_dict.begin(); it != qcore_dict.end(); ++it)
    {
        size_t key_id = std::distance(qcore_dict.begin(), it);
        auto &[key, qcore] = *it;

        if (it == std::prev(qcore_dict.end()))
        {
            qcore.is_last = true;
            qcore.is_last_in_event = true;
        }
        else
        {
            qcore.is_last_in_event = false;

            auto &next_key = std::next(it)->first;
            qcore.is_last = key.first != next_key.first;
        }

        if (it == qcore_dict.begin())
        {
            qcore.is_neighbour = false;
        }
        else
        {
            auto &prev_key = std::prev(it)->first;
            qcore.is_neighbour = (key.first == prev_key.first &&
                                          key.second == prev_key.second + 1);
        }

        qcores.push_back(qcore);
    }
}

std::vector<uint64_t> Event::serialize_event()
{
    const int WORD_SIZE = config->chip_id ? 61 : 63;
    std::vector<uint64_t> result;
    int current_word = 0;
    int current_size = 0;

    auto packets = _retrieve_qcore_data();
    packets.insert(packets.begin(), std::make_tuple(8, event_tag, "event tag"));

    for (const auto &[width, word, name] : packets)
    {
        int width_left = width;
        int word_copy = word;

        while (width_left > 0)
        {
            int space_left = WORD_SIZE - current_size;

            if (width_left <= space_left)
            {
                current_word <<= width_left;
                current_word |= word_copy & ((1 << width_left) - 1);
                current_size += width_left;
                width_left = 0;
            }
            else
            {
                int bits_for_current_word = word_copy >> (width_left - space_left);
                current_word <<= space_left;
                current_word |= bits_for_current_word & ((1 << space_left) - 1);
                word_copy -= bits_for_current_word << (width_left - space_left);
                width_left -= space_left;

                result.push_back(current_size);
                current_word = 0;
                current_size = 0;
            }

            if (current_size == WORD_SIZE)
            {
                result.push_back(current_size);
                current_word = 0;
                current_size = 0;
            }
        }
    }

    if (current_size > 0)
    {
        int shift_amount = WORD_SIZE - current_size;
        current_word <<= shift_amount;
        result.push_back(current_size);
    }

    if (config->chip_id)
    {
        for (auto &word : result)
        {
            word |= ((uint64_t)chip_id & 0b11) << 61;
        }
    }

    result[result.size() - 1] = result.back() | (1ull << 63);
    return result;
}

std::vector<std::tuple<int, int, std::string>> Event::_retrieve_qcore_data()
{
    bool prev_last_in_col = true;
    std::vector<std::tuple<int, int, std::string>> qcore_packages;

    for (const auto &qcore : qcores)
    {
        auto qcore_data = qcore.serialize_qcore(prev_last_in_col);
        qcore_packages.insert(qcore_packages.end(), qcore_data.begin(), qcore_data.end());
        prev_last_in_col = qcore.is_last;
    }

    return qcore_packages;
}

void say_hello_world(std::string r) {
    std::cout << r << std::endl;
}
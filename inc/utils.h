#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <bitset>
#include <string>
#include <map>

// Function to return the binary representation of the least significant bits as a string
template <typename T>
std::string get_lsb_binary(T value, int num_bits)
{
    // Create a bitset of sufficient size to hold the number of bits
    std::bitset<sizeof(T) * 8> bits(value);

    // Convert the bitset to a string and extract the desired number of bits
    std::string bitString = bits.to_string();
    std::string truncated_bit_string = bitString.substr(bitString.size() - num_bits);

    // Return the binary representation as a string
    return truncated_bit_string;
}


enum class DataTags
{
    TRIGGER_TAG,
    EXTRA_IDS,
    COLUMN,
    IS_NEIGHBOUR,
    IS_LAST,
    ROW,
    HITMAP,
    TOT
};

std::ostream &operator<<(std::ostream &os, DataTags tag);

enum Color
{
    // Standard colors
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,

    // Bright colors
    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE,

    // Reset
    RESET
};


// Map to connect DataTags enum to their ANSI escape codes for foreground & background
static std::map<DataTags, std::pair<Color, Color>> data_tag_colors = {
    {DataTags::TRIGGER_TAG, {Color::YELLOW, Color::RESET}},
    {DataTags::EXTRA_IDS, {Color::CYAN, Color::RESET}},
    {DataTags::COLUMN, {Color::GREEN, Color::RESET}},
    {DataTags::IS_NEIGHBOUR, {Color::RED, Color::RESET}},
    {DataTags::IS_LAST, {Color::MAGENTA, Color::RESET}},
    {DataTags::ROW, {Color::BLUE, Color::RESET}},
    {DataTags::HITMAP, {Color::BRIGHT_YELLOW, Color::BLACK}},
    {DataTags::TOT, {Color::WHITE, Color::RESET}}
};

using ansi_map_t = std::map<Color, std::string>;

// Map to connect Color enum to their ANSI escape codes for foreground
static ansi_map_t fgc = {
    {Color::BLACK, "\033[30m"},
    {Color::RED, "\033[31m"},
    {Color::GREEN, "\033[32m"},
    {Color::YELLOW, "\033[33m"},
    {Color::BLUE, "\033[34m"},
    {Color::MAGENTA, "\033[35m"},
    {Color::CYAN, "\033[36m"},
    {Color::WHITE, "\033[37m"},

    {Color::BRIGHT_BLACK, "\033[90m"},
    {Color::BRIGHT_RED, "\033[91m"},
    {Color::BRIGHT_GREEN, "\033[92m"},
    {Color::BRIGHT_YELLOW, "\033[93m"},
    {Color::BRIGHT_BLUE, "\033[94m"},
    {Color::BRIGHT_MAGENTA, "\033[95m"},
    {Color::BRIGHT_CYAN, "\033[96m"},
    {Color::BRIGHT_WHITE, "\033[97m"},

    {Color::RESET, "\033[39m"}
};

// Map to connect Color enum to their ANSI escape codes for background
static ansi_map_t bgc = {
    {Color::BLACK, "\033[40m"},
    {Color::RED, "\033[41m"},
    {Color::GREEN, "\033[42m"},
    {Color::YELLOW, "\033[43m"},
    {Color::BLUE, "\033[44m"},
    {Color::MAGENTA, "\033[45m"},
    {Color::CYAN, "\033[46m"},
    {Color::WHITE, "\033[47m"},

    {Color::BRIGHT_BLACK, "\033[100m"},
    {Color::BRIGHT_RED, "\033[101m"},
    {Color::BRIGHT_GREEN, "\033[102m"},
    {Color::BRIGHT_YELLOW, "\033[103m"},
    {Color::BRIGHT_BLUE, "\033[104m"},
    {Color::BRIGHT_MAGENTA, "\033[105m"},
    {Color::BRIGHT_CYAN, "\033[106m"},
    {Color::BRIGHT_WHITE, "\033[107m"},

    {Color::RESET, "\033[49m"}
};


const std::string set_color(std::pair<Color, Color> color_pair);

#endif

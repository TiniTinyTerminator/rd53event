
#include <ostream>

#include "utils.h"

std::ostream &operator<<(std::ostream &os, DataTags tag)
{
    switch (tag)
    {
    case DataTags::TRIGGER_TAG:
        return os << "TRIGGER_TAG";
    case DataTags::EXTRA_IDS:
        return os << "EXTRA_IDS";
    case DataTags::COLUMN:
        return os << "COLUMN";
    case DataTags::IS_NEIGHBOUR:
        return os << "IS_NEIGHBOUR";
    case DataTags::IS_LAST:
        return os << "IS_LAST";
    case DataTags::ROW:
        return os << "ROW";
    case DataTags::HITMAP:
        return os << "HITMAP";
    case DataTags::TOT:
        return os << "TOT";
    default:
        return os << "UNKNOWN";
    }
}

const std::string set_color(std::pair<Color, Color> color_pair)
{
    return fgc[color_pair.first] + bgc[color_pair.second];
}
#pragma once
#ifndef FG_INC_QUALITY_TYPES
#define FG_INC_QUALITY_TYPES

#include <magic_enum.hpp>

enum class Quality : char
{
    UNIVERSAL = (char)-1,
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    VERY_HIGH = 3,
    ULTRA = 4,
    REALITY = 5
};

inline Quality getQualityFromText(std::string_view text)
{
    auto quality = magic_enum::enum_cast<Quality>(text);
    return (quality.has_value() ? quality.value() : Quality::UNIVERSAL);
}

inline std::string_view getQualityName(Quality quality)
{
    return magic_enum::enum_name(quality);
}
#endif //> FG_INC_QUALITY_TYPES
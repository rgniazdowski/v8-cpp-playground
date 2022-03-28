#include <catch2/catch.hpp>
#include <util/BitField.hpp>
#include <iostream>
#include <string>
#include <bitset>
#include <cstddef>
#include <cstdint>

uint32_t bit(uint32_t idx) { return (1ULL << (idx == 0 ? 0 : idx - 1)); }

struct TestStruct
{
    enum
    {
        MAX_F1 = (1ULL << 26) - 1ULL,
        MAX_F2 = (1ULL << 6) - 1ULL,
        MAX_F3 = (1ULL << 32) - 1ULL
    };
    union
    {
        uint8_t arr[8]; // 8 bytes - 64 bit
        uint64_t value;
        util::BitFieldMember<0, 26> f1;  // [0, 26)
        util::BitFieldMember<26, 6> f2;  // [26, 32)
        util::BitFieldMember<32, 32> f3; // [32, 64)
    };

    std::string getValueBits(void) const { return std::bitset<64>(value).to_string(); }
    std::string getF1Bits(void) const { return std::bitset<26>(f1).to_string(); }
    std::string getF2Bits(void) const { return std::bitset<6>(f2).to_string(); }
    std::string getF3Bits(void) const { return std::bitset<32>(f3).to_string(); }
};
//>---------------------------------------------------------------------------------------

TEST_CASE("Bitfields", "[bitfields]")
{

}
//!--------------------------------------------------------------------------------------
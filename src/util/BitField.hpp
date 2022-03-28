#pragma once
#ifndef FG_INC_UTIL_BITFIELD
#define FG_INC_UTIL_BITFIELD

#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace util
{
    template <unsigned firstBit, unsigned bitSize>
    struct BitFieldMember
    {
        using self_type = BitFieldMember<firstBit, bitSize>;

        enum
        {
            LAST_BIT = firstBit + bitSize - 1,
            MASK = (1ULL << bitSize) - 1ULL,
            FIRST_BIT_IDX = 8 - (firstBit & 7),
            EXCESS_BITS_LAST_BYTE = 7 - (LAST_BIT & 7)
        };

        BitFieldMember() { assign(0); }
        BitFieldMember(uint32_t value) { assign(value); }

        uint8_t *selfArray() { return reinterpret_cast<uint8_t *>(this); }
        const uint8_t *selfArray() const { return reinterpret_cast<const uint8_t *>(this); }

        /* used to read data from the field */
        /* will also work with all the operators that work with integral types */
        inline constexpr operator uint32_t() const { return this->value(); }

        inline constexpr uint32_t value(void) const
        {
            const uint8_t *arr = selfArray();
            const uint8_t *p = arr + firstBit / 8;
            uint32_t idx = FIRST_BIT_IDX;
            uint32_t ret = 0;
            ret |= *p;
            while (idx < bitSize)
            {
                ret <<= 8;
                ret |= *(++p);
                idx += 8;
            }
            return ((ret >> EXCESS_BITS_LAST_BYTE) & MASK);
        }

        /* used to assign a value into the field */
        inline self_type &operator=(uint32_t value)
        {
            this->assign(value);
            return *this;
        }

        inline void assign(uint32_t value)
        {
            uint8_t *arr = selfArray();
            uint8_t *p = arr + LAST_BIT / 8;
            value &= MASK;
            uint32_t wmask = ~(MASK << EXCESS_BITS_LAST_BYTE);
            value <<= EXCESS_BITS_LAST_BYTE;
            int i = (LAST_BIT & 7) + 1;
            (*p &= wmask) |= value;
            while (i < bitSize)
            {
                value >>= 8;
                wmask >>= 8;
                (*(--p) &= wmask) |= value;
                i += 8;
            }
        }

        inline self_type &operator+=(unsigned m)
        {
            *this = *this + m;
            return *this;
        }

        inline self_type &operator-=(unsigned m)
        {
            *this = *this - m;
            return *this;
        }

        inline self_type &operator*=(unsigned m)
        {
            *this = *this * m;
            return *this;
        }

        inline self_type &operator/=(unsigned m)
        {
            *this = *this / m;
            return *this;
        }

        inline self_type &operator%=(unsigned m)
        {
            *this = *this % m;
            return *this;
        }

        inline self_type &operator<<=(unsigned m)
        {
            *this = *this << m;
            return *this;
        }

        inline self_type &operator>>=(unsigned m)
        {
            *this = *this >> m;
            return *this;
        }

        inline self_type &operator|=(unsigned m)
        {
            *this = *this | m;
            return *this;
        }

        inline self_type &operator&=(unsigned m)
        {
            *this = *this & m;
            return *this;
        }

        inline self_type &operator^=(unsigned m)
        {
            *this = *this ^ m;
            return *this;
        }
    }; //> struct BitFieldMember
} //> namespace util

#endif //> FG_INC_UTIL_BITFIELD

#pragma once
#ifndef FG_INC_ENUM_FLAGS
#define FG_INC_ENUM_FLAGS

#define ENUM_FLAGS(Type)                                                                                             \
    inline Type operator!(Type x) { return static_cast<Type>(!static_cast<int>(x)); }                                \
    inline bool operator&&(Type x, Type y) { return static_cast<bool>(static_cast<int>(x) && static_cast<int>(y)); } \
    inline bool operator||(Type x, Type y) { return static_cast<bool>(static_cast<int>(x) || static_cast<int>(y)); } \
    inline bool operator||(bool x, Type y) { return static_cast<bool>(static_cast<int>(x) || static_cast<int>(y)); } \
    inline bool operator||(Type x, bool y) { return static_cast<bool>(static_cast<int>(x) || static_cast<int>(y)); } \
    inline Type operator&(Type x, Type y) { return static_cast<Type>(static_cast<int>(x) & static_cast<int>(y)); }   \
    inline Type operator|(Type x, Type y) { return static_cast<Type>(static_cast<int>(x) | static_cast<int>(y)); }   \
    inline Type operator^(Type x, Type y) { return static_cast<Type>(static_cast<int>(x) ^ static_cast<int>(y)); }   \
    inline Type operator~(Type x) { return static_cast<Type>(~static_cast<int>(x)); }                                \
    inline Type &operator&=(Type &x, Type y)                                                                         \
    {                                                                                                                \
        x = x & y;                                                                                                   \
        return x;                                                                                                    \
    }                                                                                                                \
    inline Type &operator|=(Type &x, Type y)                                                                         \
    {                                                                                                                \
        x = x | y;                                                                                                   \
        return x;                                                                                                    \
    }                                                                                                                \
    inline Type &operator^=(Type &x, Type y)                                                                         \
    {                                                                                                                \
        x = x ^ y;                                                                                                   \
        return x;                                                                                                    \
    }

#endif //> FG_INC_ENUM_FLAGS
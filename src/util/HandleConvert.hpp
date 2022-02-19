#pragma once
#ifndef FG_INC_UTIL_HANDLE_CONVERT
#define FG_INC_UTIL_HANDLE_CONVERT

namespace util
{
    template <typename T>
    struct convert
    {
        static T *convertToPointer(void *pointer, uint64_t identifier) { return static_cast<T *>(pointer); }
    };
} //> namespace util

#endif //> FG_INC_UTIL_HANDLE_CONVERT
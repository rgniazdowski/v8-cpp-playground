#pragma once
#ifndef FG_INC_UTIL_TAG
#define FG_INC_UTIL_TAG

#include <typeinfo>
#include <string>

namespace util
{
    struct UniversalIdBase
    {
    protected:
        inline static uint8_t s_gid = 0;
    };

    template <typename UserClass>
    struct UniversalId : UniversalIdBase
    {
        using user_type = std::remove_pointer_t<UserClass>;
        using self_type = UniversalId<UserClass>;

    private:
        inline static bool s_idset = false;
        inline static uint8_t s_lid = 0;

    public:
        static uint8_t id()
        {
            if (!s_idset)
                s_lid = (s_idset ? s_gid : ++s_gid);
            s_idset = true;
            return s_lid;
        }
        static const char *name(void)
        {
            auto n = typeid(UserClass).name();
            auto sv = std::string_view(n);
            int s = 0;
            for (int i = 0; i < sv.length(); i++)
            {
                char c = n[i];
                if (c == ' ')
                {
                    s = i + 1;
                    break;
                }
            }
            n += s;
            return n;
        }
        UniversalId() = delete;
        UniversalId(const self_type &in) = delete;
        self_type &operator=(const self_type &in) = delete;
    };

    struct TagBase
    {
    protected:
        inline static uint8_t s_gid = 0;
    };

    template <typename UserClass>
    struct Tag : TagBase
    {
        using user_type = std::remove_pointer_t<UserClass>;
        using self_type = Tag<UserClass>;

    private:
        inline static bool s_idset = false;
        inline static uint8_t s_lid = 0;

    public:
        static uint8_t id()
        {
            if (!s_idset)
                s_lid = (s_idset ? s_gid : ++s_gid);
            s_idset = true;
            return s_lid;
        }
        static const char *name(void)
        {
            // static const char **needles;
            auto n = typeid(UserClass).name();
            auto sv = std::string_view(n);
            int s = 0;
            for (int i = 0; i < sv.length(); i++)
            {
                char c = n[i];
                if (c == ' ')
                {
                    s = i + 1;
                    break;
                }
            }
            n += s;
            return n;
        }
        Tag() = delete;
        Tag(const self_type &in) = delete;
        self_type &operator=(const self_type &in) = delete;
    };
} //> namespace util

#endif //> FG_INC_UTIL_TAG